#include "Command.h"

namespace Upp {

namespace Node {

CommandResult AddNodeCmd::Execute(CommandContext& ctx) {
	if(ctx.graph.FindNode(id)) return CommandResult(false, "Node already exists: " + id);
	ctx.graph.AddNode(id).pos = pos;
	return true;
}

void AddNodeCmd::Undo(CommandContext& ctx) {
	ctx.graph.RemoveNode(id);
}

CommandResult MoveNodeCmd::Execute(CommandContext& ctx) {
	NodeDoc* n = ctx.graph.FindNode(id);
	if(!n) return CommandResult(false, "Node not found: " + id);
	old_pos = n->pos;
	n->pos = new_pos;
	ctx.graph.Invalidate(id);
	return true;
}

void MoveNodeCmd::Undo(CommandContext& ctx) {
	NodeDoc* n = ctx.graph.FindNode(id);
	if(n) {
		n->pos = old_pos;
		ctx.graph.Invalidate(id);
	}
}

CommandResult RemoveNodeCmd::Execute(CommandContext& ctx) {
	NodeDoc* n = ctx.graph.FindNode(id);
	if(!n) return CommandResult(false, "Node not found: " + id);

	removed_node <<= *n;

	// Record node position in array
	const GraphDoc& doc = ctx.graph.GetDoc();
	removed_node_pos = -1;
	for(int i = 0; i < doc.nodes.GetCount(); i++)
		if(doc.nodes[i].id == id) { removed_node_pos = i; break; }

	// Record edges and their positions before removal
	removed_edges.Clear();
	removed_edge_pos.Clear();
	for(int i = 0; i < doc.edges.GetCount(); i++) {
		if(doc.edges[i].source_node == id || doc.edges[i].target_node == id) {
			removed_edges.Add() <<= doc.edges[i];
			removed_edge_pos.Add(i);
		}
	}

	ctx.graph.RemoveNode(id);
	return true;
}

void RemoveNodeCmd::Undo(CommandContext& ctx) {
	// Restore node (appended — ordering is cosmetic, not semantic)
	ctx.graph.AddNode(id) <<= removed_node;

	// Restore edges by inserting at original positions (forward order preserves relative order)
	GraphDoc& doc = ctx.graph.GetDoc();
	for(int k = 0; k < removed_edges.GetCount(); k++) {
		int pos = min(removed_edge_pos[k], doc.edges.GetCount());
		EdgeDoc& e = doc.edges.Insert(pos);
		e <<= removed_edges[k];
	}
	ctx.graph.RebuildIndexPublic();
}

CommandResult AddEdgeCmd::Execute(CommandContext& ctx) {
	if(ctx.graph.FindEdge(id)) return CommandResult(false, "Edge already exists: " + id);
	ctx.graph.AddEdge(id, source_node, source_pin, target_node, target_pin);
	return true;
}

void AddEdgeCmd::Undo(CommandContext& ctx) {
	ctx.graph.RemoveEdge(id);
}

CommandResult SetWidgetValueCmd::Execute(CommandContext& ctx) {
	int sep = id.Find(':');
	if(sep < 0) return CommandResult(false, "Invalid slot ID: " + id);
	EntityId node_id = id.Left(sep);
	EntityId slot_id = id.Mid(sep + 1);
	
	NodeDoc* n = ctx.graph.FindNode(node_id);
	if(!n) return CommandResult(false, "Node not found: " + node_id);
	
	for(auto& s : n->slots) {
		if(s.id == slot_id) {
			old_val = s.properties.Get("value", Value());
			s.properties.Add("value", new_val);
			return true;
		}
	}
	return CommandResult(false, "Slot not found: " + slot_id);
}

void SetWidgetValueCmd::Undo(CommandContext& ctx) {
	int sep = id.Find(':');
	if(sep < 0) return;
	EntityId node_id = id.Left(sep);
	EntityId slot_id = id.Mid(sep + 1);
	NodeDoc* n = ctx.graph.FindNode(node_id);
	if(!n) return;
	for(auto& s : n->slots) {
		if(s.id == slot_id) {
			s.properties.Add("value", old_val);
			return;
		}
	}
}

CommandResult SelectCmd::Execute(CommandContext& ctx) {
	old_selection <<= ctx.editor.selection;
	if(exclusive)
		ctx.editor.ClearSelection();
	ctx.editor.Select(id);
	return true;
}

void SelectCmd::Undo(CommandContext& ctx) {
	ctx.editor.selection <<= old_selection;
}

CommandResult ClearSelectionCmd::Execute(CommandContext& ctx) {
	old_selection <<= ctx.editor.selection;
	ctx.editor.ClearSelection();
	return true;
}

void ClearSelectionCmd::Undo(CommandContext& ctx) {
	ctx.editor.selection <<= old_selection;
}

CommandResult ToggleSelectionCmd::Execute(CommandContext& ctx) {
	was_selected = ctx.editor.IsSelected(id);
	if(was_selected)
		ctx.editor.Deselect(id);
	else
		ctx.editor.Select(id);
	return true;
}

void ToggleSelectionCmd::Undo(CommandContext& ctx) {
	if(was_selected)
		ctx.editor.Select(id);
	else
		ctx.editor.Deselect(id);
}

void CommandDispatcher::RegisterStandardCommands()
{
	Register("AddNode", [] { return One<Command>(new AddNodeCmd); });
	Register("MoveNode", [] { return One<Command>(new MoveNodeCmd); });
	Register("RemoveNode", [] { return One<Command>(new RemoveNodeCmd); });
	Register("Select", [] { return One<Command>(new SelectCmd); });
	Register("ClearSelection", [] { return One<Command>(new ClearSelectionCmd); });
	Register("ToggleSelection", [] { return One<Command>(new ToggleSelectionCmd); });
	Register("AddEdge", [] { return One<Command>(new AddEdgeCmd); });
	Register("SetWidgetValue", [] { return One<Command>(new SetWidgetValueCmd); });
}

One<Command> CommandDispatcher::Create(const String& name, const Upp::Value& args)
{
	int i = factories.Find(name);
	if(i < 0) return nullptr;
	One<Command> cmd = factories[i]();
	cmd->FromArgs(args);
	return cmd;
}

CommandResult CommandDispatcher::Execute(const String& name, CommandContext& ctx, const Upp::Value& args)
{
	One<Command> cmd = Create(name, args);
	if(!cmd) return CommandResult(false, "Unknown command: " + name);
	return cmd->Execute(ctx);
}

void HistoryStack::Begin()
{
	Commit();
	in_transaction = true;
}

void HistoryStack::Commit()
{
	if(current_transaction.GetCount()) {
		undo_stack.Add().commands = pick(current_transaction);
		redo_stack.Clear();
	}
	in_transaction = false;
}

void HistoryStack::Abort(CommandContext&& ctx)
{
	for(int i = current_transaction.GetCount() - 1; i >= 0; i--)
		current_transaction[i]->Undo(ctx);
	current_transaction.Clear();
	in_transaction = false;
}

CommandResult HistoryStack::Execute(CommandContext&& ctx, One<Command> cmd)
{
	CommandResult res = cmd->Execute(ctx);
	if(res) {
		current_transaction.Add(pick(cmd));
		if(!in_transaction)
			Commit();
	}
	return res;
}

bool HistoryStack::Undo(CommandContext&& ctx)
{
	Commit();
	if(undo_stack.IsEmpty()) return false;

	Entry& e = undo_stack.Top();
	for(int i = e.commands.GetCount() - 1; i >= 0; i--)
		e.commands[i]->Undo(ctx);

	redo_stack.Add().commands = pick(e.commands);
	undo_stack.Drop();
	return true;
}

bool HistoryStack::Redo(CommandContext&& ctx)
{
	if(redo_stack.IsEmpty()) return false;

	Entry& e = redo_stack.Top();
	for(int i = 0; i < e.commands.GetCount(); i++)
		e.commands[i]->Execute(ctx);

	undo_stack.Add().commands = pick(e.commands);
	redo_stack.Drop();
	return true;
}

void HistoryStack::Clear()
{
	undo_stack.Clear();
	redo_stack.Clear();
	current_transaction.Clear();
}

} // namespace Node

} // namespace Upp
