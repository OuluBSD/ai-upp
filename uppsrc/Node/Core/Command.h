#ifndef _Node_Core_Command_h_
#define _Node_Core_Command_h_

#include "Editor.h"

namespace Upp {

namespace Node {

struct CommandResult {
	bool   success = false;
	String error;
	
	CommandResult() {}
	CommandResult(bool s, const String& e = String()) : success(s), error(e) {}
	
	operator bool() const { return success; }
};

struct CommandContext {
	Graph&       graph;
	EditorState& editor;
	
	CommandContext(Graph& g, EditorState& e) : graph(g), editor(e) {}
};

class Command : public Moveable<Command> {
public:
	virtual ~Command() {}
	virtual String        GetName() const = 0;
	virtual void          FromArgs(const Upp::Value& args) {}
	virtual CommandResult Execute(CommandContext& ctx) = 0;
	virtual void          Undo(CommandContext& ctx) = 0;
};

class CommandDispatcher {
	typedef Function<One<Command>()> Factory;
	VectorMap<String, Factory> factories;

public:
	void Register(const String& name, Factory factory) { factories.Add(name, factory); }
	void RegisterStandardCommands();
	
	One<Command>  Create(const String& name, const Upp::Value& args);
	CommandResult Execute(const String& name, CommandContext& ctx, const Upp::Value& args);
};

struct AddNodeCmd : public Command {
	EntityId id;
	Pointf   pos;

	virtual String GetName() const override { return "AddNode"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); pos.x = a["x"]; pos.y = a["y"]; }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct MoveNodeCmd : public Command {
	EntityId id;
	Pointf   new_pos;
	Pointf   old_pos;

	virtual String GetName() const override { return "MoveNode"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); new_pos.x = a["x"]; new_pos.y = a["y"]; }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct RemoveNodeCmd : public Command {
	EntityId id;
	NodeDoc  removed_node;
	Array<EdgeDoc> removed_edges;

	virtual String GetName() const override { return "RemoveNode"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct SelectCmd : public Command {
	EntityId id;
	bool     exclusive = true;
	Index<EntityId> old_selection;

	virtual String GetName() const override { return "Select"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); exclusive = (bool)a["exclusive"]; }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct ClearSelectionCmd : public Command {
	Index<EntityId> old_selection;

	virtual String GetName() const override { return "ClearSelection"; }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct ToggleSelectionCmd : public Command {
	EntityId id;
	bool     was_selected = false;

	virtual String GetName() const override { return "ToggleSelection"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct AddEdgeCmd : public Command {
	EntityId id;
	EntityId source_node;
	EntityId source_pin;
	EntityId target_node;
	EntityId target_pin;

	virtual String GetName() const override { return "AddEdge"; }
	virtual void   FromArgs(const Upp::Value& a) override {
		id = a["id"].ToString(); source_node = a["source_node"].ToString();
		source_pin = a["source_pin"].ToString(); target_node = a["target_node"].ToString();
		target_pin = a["target_pin"].ToString();
	}
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

struct SetWidgetValueCmd : public Command {
	EntityId id; // format "node:slot"
	Value    new_val;
	Value    old_val;

	virtual String GetName() const override { return "SetWidgetValue"; }
	virtual void   FromArgs(const Upp::Value& a) override { id = a["id"].ToString(); new_val = a["value"]; }
	virtual CommandResult Execute(CommandContext& ctx) override;
	virtual void Undo(CommandContext& ctx) override;
};

class HistoryStack {
	struct Entry : public Moveable<Entry> {
		Vector<One<Command>> commands;
	};
	Vector<Entry> undo_stack;
	Vector<Entry> redo_stack;
	
	Vector<One<Command>> current_transaction;
	bool in_transaction = false;

public:
	void Begin();
	void Commit();
	void Abort(CommandContext& ctx);
	
	CommandResult Execute(CommandContext& ctx, One<Command> cmd);
	
	bool Undo(CommandContext& ctx);
	bool Redo(CommandContext& ctx);
	
	bool CanUndo() const { return undo_stack.GetCount(); }
	bool CanRedo() const { return redo_stack.GetCount(); }
	
	void Clear();
};

} // namespace Node

} // namespace Upp

#endif
