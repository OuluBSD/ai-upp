#include "Core.h"

namespace Upp {

namespace Node {

void Graph::RebuildIndex()
{
	node_idx.Clear();
	for(int i = 0; i < doc.nodes.GetCount(); i++)
		node_idx.Add(doc.nodes[i].id, i);
	edge_idx.Clear();
	for(int i = 0; i < doc.edges.GetCount(); i++)
		edge_idx.Add(doc.edges[i].id, i);
	group_idx.Clear();
	for(int i = 0; i < doc.groups.GetCount(); i++)
		group_idx.Add(doc.groups[i].id, i);
}

NodeDoc& Graph::AddNode(const EntityId& id)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindNode(id));
	int i = doc.nodes.GetCount();
	NodeDoc& n = doc.nodes.Add();
	n.id = id;
	node_idx.Add(id, i);
	Invalidate();
	return n;
}

void Graph::RemoveNode(const EntityId& id)
{
	int i = node_idx.Find(id);
	if(i < 0) return;
	int pos = node_idx[i];
	doc.nodes.Remove(pos);
	// Rebuild node index (positions shifted after remove)
	node_idx.Clear();
	for(int j = 0; j < doc.nodes.GetCount(); j++)
		node_idx.Add(doc.nodes[j].id, j);

	// Remove dangling edges
	for(int j = doc.edges.GetCount() - 1; j >= 0; j--) {
		if(doc.edges[j].source_node == id || doc.edges[j].target_node == id)
			doc.edges.Remove(j);
	}
	// Rebuild edge index (positions shifted)
	edge_idx.Clear();
	for(int j = 0; j < doc.edges.GetCount(); j++)
		edge_idx.Add(doc.edges[j].id, j);

	Invalidate();
}

NodeDoc* Graph::FindNode(const EntityId& id)
{
	int i = node_idx.Find(id);
	return i >= 0 ? &doc.nodes[node_idx[i]] : nullptr;
}

const NodeDoc* Graph::FindNode(const EntityId& id) const
{
	int i = node_idx.Find(id);
	return i >= 0 ? &doc.nodes[node_idx[i]] : nullptr;
}

EdgeDoc& Graph::AddEdge(const EntityId& id, const EntityId& source_node, const EntityId& source_pin,
                        const EntityId& target_node, const EntityId& target_pin)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindEdge(id));
	int i = doc.edges.GetCount();
	EdgeDoc& e = doc.edges.Add();
	e.id = id;
	e.source_node = source_node;
	e.source_pin = source_pin;
	e.target_node = target_node;
	e.target_pin = target_pin;
	edge_idx.Add(id, i);
	Invalidate();
	return e;
}

void Graph::RemoveEdge(const EntityId& id)
{
	int i = edge_idx.Find(id);
	if(i < 0) return;
	int pos = edge_idx[i];
	doc.edges.Remove(pos);
	edge_idx.Clear();
	for(int j = 0; j < doc.edges.GetCount(); j++)
		edge_idx.Add(doc.edges[j].id, j);
	Invalidate();
}

EdgeDoc* Graph::FindEdge(const EntityId& id)
{
	int i = edge_idx.Find(id);
	return i >= 0 ? &doc.edges[edge_idx[i]] : nullptr;
}

const EdgeDoc* Graph::FindEdge(const EntityId& id) const
{
	int i = edge_idx.Find(id);
	return i >= 0 ? &doc.edges[edge_idx[i]] : nullptr;
}

GroupDoc& Graph::AddGroup(const EntityId& id)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindGroup(id));
	int i = doc.groups.GetCount();
	GroupDoc& g = doc.groups.Add();
	g.id = id;
	group_idx.Add(id, i);
	Invalidate();
	return g;
}

void Graph::RemoveGroup(const EntityId& id)
{
	int i = group_idx.Find(id);
	if(i < 0) return;
	int pos = group_idx[i];
	doc.groups.Remove(pos);
	group_idx.Clear();
	for(int j = 0; j < doc.groups.GetCount(); j++)
		group_idx.Add(doc.groups[j].id, j);
	Invalidate();
}

GroupDoc* Graph::FindGroup(const EntityId& id)
{
	int i = group_idx.Find(id);
	return i >= 0 ? &doc.groups[group_idx[i]] : nullptr;
}

const GroupDoc* Graph::FindGroup(const EntityId& id) const
{
	int i = group_idx.Find(id);
	return i >= 0 ? &doc.groups[group_idx[i]] : nullptr;
}

Vector<ValidationMessage> Graph::Validate() const
{
	Vector<ValidationMessage> res;

	Index<EntityId> node_ids;
	for(const auto& n : doc.nodes) {
		if(node_ids.Find(n.id) >= 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Duplicate node ID: " + n.id, n.id));
		node_ids.Add(n.id);

		// Pin IDs must be unique per direction (input/output separately),
		// but the same name is allowed for an input and an output pin (pass-through style).
		Index<EntityId> in_pin_ids, out_pin_ids;
		for(const auto& p : n.pins) {
			Index<EntityId>& dir_ids = (p.kind == PinKind::Output) ? out_pin_ids : in_pin_ids;
			if(dir_ids.Find(p.id) >= 0)
				res.Add(ValidationMessage(ValidationMessage::ERROR,
				        "Duplicate " + String(p.kind == PinKind::Output ? "output" : "input") +
				        " pin ID: " + p.id + " in node " + n.id, n.id));
			dir_ids.Add(p.id);
		}
	}

	for(const auto& e : doc.edges) {
		const PinDoc* src_pin = nullptr;
		const PinDoc* tgt_pin = nullptr;

		int si = node_ids.Find(e.source_node);
		if(si < 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent source node " + e.source_node, e.id));
		else if(!e.source_pin.IsEmpty()) {
			// Search specifically for an Output pin with this id (nodes may have same-named in+out pins)
			for(const auto& p : doc.nodes[si].pins)
				if(p.id == e.source_pin && p.kind == PinKind::Output) { src_pin = &p; break; }
			if(!src_pin)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent output pin " + e.source_pin + " in node " + e.source_node, e.id));
		}

		int ti = node_ids.Find(e.target_node);
		if(ti < 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent target node " + e.target_node, e.id));
		else if(!e.target_pin.IsEmpty()) {
			// Search specifically for an Input pin with this id (nodes may have same-named in+out pins)
			for(const auto& p : doc.nodes[ti].pins)
				if(p.id == e.target_pin && p.kind == PinKind::Input) { tgt_pin = &p; break; }
			if(!tgt_pin)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent input pin " + e.target_pin + " in node " + e.target_node, e.id));
		}

		// Typed-pin validation: if both sides carry a concrete type name, they must match.
		if(src_pin && tgt_pin && !src_pin->type_name.IsEmpty() && !tgt_pin->type_name.IsEmpty()) {
			if(src_pin->type_name != tgt_pin->type_name) {
				res.Add(ValidationMessage(
					ValidationMessage::ERROR,
					"Edge " + e.id + " type mismatch: " + e.source_node + "." + e.source_pin +
					" (" + src_pin->type_name + ") -> " +
					e.target_node + "." + e.target_pin + " (" + tgt_pin->type_name + ")",
					e.id));
			}
		}
	}

	for(const auto& g : doc.groups)
		for(const auto& n_id : g.nodes)
			if(node_ids.Find(n_id) < 0)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Group " + g.id + " refers to non-existent node " + n_id, g.id));

	return res;
}

bool Graph::LoadJson(const String& json, Vector<ValidationMessage>& errors)
{
	if(!LoadFromJson(doc, json)) {
		errors.Add(ValidationMessage(ValidationMessage::ERROR, "Failed to parse JSON"));
		return false;
	}
	RebuildIndex();
	Invalidate();
	errors.Append(Validate());
	for(const auto& m : errors)
		if(m.severity == ValidationMessage::ERROR)
			return false;
	return true;
}

String Graph::SaveJson() const
{
	return StoreAsJson(doc);
}

bool Graph::LoadXml(const String& xml, Vector<ValidationMessage>& errors)
{
	if(!LoadFromXML(doc, xml)) {
		errors.Add(ValidationMessage(ValidationMessage::ERROR, "Failed to parse XML"));
		return false;
	}
	RebuildIndex();
	Invalidate();
	errors.Append(Validate());
	for(const auto& m : errors)
		if(m.severity == ValidationMessage::ERROR)
			return false;
	return true;
}

String Graph::SaveXml() const
{
	return StoreAsXML(doc);
}

void Graph::ApplyLayout(const Vector<NodeState>& states)
{
	int n = min(states.GetCount(), doc.nodes.GetCount());
	for(int i = 0; i < n; i++) {
		if(doc.nodes[i].pos != states[i].layout_pos) {
			doc.nodes[i].pos = states[i].layout_pos;
			Invalidate(doc.nodes[i].id);
		}
	}
}

} // namespace Node

} // namespace Upp
