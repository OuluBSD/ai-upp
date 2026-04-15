#include "Core.h"

namespace Upp {

namespace Node {

NodeDoc& Graph::AddNode(const EntityId& id)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindNode(id));
	NodeDoc& n = doc.nodes.Add();
	n.id = id;
	Invalidate();
	return n;
}

void Graph::RemoveNode(const EntityId& id)
{
	for(int i = 0; i < doc.nodes.GetCount(); i++) {
		if(doc.nodes[i].id == id) {
			doc.nodes.Remove(i);
			Invalidate();
			break;
		}
	}
	// Also remove dangling edges
	for(int i = doc.edges.GetCount() - 1; i >= 0; i--) {
		if(doc.edges[i].source_node == id || doc.edges[i].target_node == id) {
			doc.edges.Remove(i);
			Invalidate();
		}
	}
}

NodeDoc* Graph::FindNode(const EntityId& id)
{
	for(int i = 0; i < doc.nodes.GetCount(); i++)
		if(doc.nodes[i].id == id)
			return &doc.nodes[i];
	return nullptr;
}

const NodeDoc* Graph::FindNode(const EntityId& id) const
{
	for(int i = 0; i < doc.nodes.GetCount(); i++)
		if(doc.nodes[i].id == id)
			return &doc.nodes[i];
	return nullptr;
}

EdgeDoc& Graph::AddEdge(const EntityId& id, const EntityId& source_node, const EntityId& source_pin,
                       const EntityId& target_node, const EntityId& target_pin)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindEdge(id));
	EdgeDoc& e = doc.edges.Add();
	e.id = id;
	e.source_node = source_node;
	e.source_pin = source_pin;
	e.target_node = target_node;
	e.target_pin = target_pin;
	Invalidate();
	return e;
}

void Graph::RemoveEdge(const EntityId& id)
{
	for(int i = 0; i < doc.edges.GetCount(); i++) {
		if(doc.edges[i].id == id) {
			doc.edges.Remove(i);
			Invalidate();
			break;
		}
	}
}

EdgeDoc* Graph::FindEdge(const EntityId& id)
{
	for(int i = 0; i < doc.edges.GetCount(); i++)
		if(doc.edges[i].id == id)
			return &doc.edges[i];
	return nullptr;
}

const EdgeDoc* Graph::FindEdge(const EntityId& id) const
{
	for(int i = 0; i < doc.edges.GetCount(); i++)
		if(doc.edges[i].id == id)
			return &doc.edges[i];
	return nullptr;
}

GroupDoc& Graph::AddGroup(const EntityId& id)
{
	ASSERT(IsValidEntityId(id));
	ASSERT(!FindGroup(id));
	GroupDoc& g = doc.groups.Add();
	g.id = id;
	Invalidate();
	return g;
}

void Graph::RemoveGroup(const EntityId& id)
{
	for(int i = 0; i < doc.groups.GetCount(); i++) {
		if(doc.groups[i].id == id) {
			doc.groups.Remove(i);
			Invalidate();
			break;
		}
	}
}

GroupDoc* Graph::FindGroup(const EntityId& id)
{
	for(int i = 0; i < doc.groups.GetCount(); i++)
		if(doc.groups[i].id == id)
			return &doc.groups[i];
	return nullptr;
}

const GroupDoc* Graph::FindGroup(const EntityId& id) const
{
	for(int i = 0; i < doc.groups.GetCount(); i++)
		if(doc.groups[i].id == id)
			return &doc.groups[i];
	return nullptr;
}

Vector<ValidationMessage> Graph::Validate() const
{
	Vector<ValidationMessage> res;
	
	Index<EntityId> node_ids;
	for(const auto& n : doc.nodes) {
		if(node_ids.Find(n.id) >= 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Duplicate node ID: " + n.id, n.id));
		node_ids.Add(n.id);
		
		Index<EntityId> pin_ids;
		for(const auto& p : n.pins) {
			if(pin_ids.Find(p.id) >= 0)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Duplicate pin ID: " + p.id + " in node " + n.id, n.id));
			pin_ids.Add(p.id);
		}
	}
	
	for(const auto& e : doc.edges) {
		int source_node_idx = node_ids.Find(e.source_node);
		if(source_node_idx < 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent source node " + e.source_node, e.id));
		else if(!e.source_pin.IsEmpty()) {
			const auto& n = doc.nodes[source_node_idx];
			bool found_pin = false;
			for(const auto& p : n.pins)
				if(p.id == e.source_pin) { found_pin = true; break; }
			if(!found_pin)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent source pin " + e.source_pin + " in node " + e.source_node, e.id));
		}

		int target_node_idx = node_ids.Find(e.target_node);
		if(target_node_idx < 0)
			res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent target node " + e.target_node, e.id));
		else if(!e.target_pin.IsEmpty()) {
			const auto& n = doc.nodes[target_node_idx];
			bool found_pin = false;
			for(const auto& p : n.pins)
				if(p.id == e.target_pin) { found_pin = true; break; }
			if(!found_pin)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Edge " + e.id + " refers to non-existent target pin " + e.target_pin + " in node " + e.target_node, e.id));
		}
	}
	
	for(const auto& g : doc.groups) {
		for(const auto& n_id : g.nodes) {
			if(node_ids.Find(n_id) < 0)
				res.Add(ValidationMessage(ValidationMessage::ERROR, "Group " + g.id + " refers to non-existent node " + n_id, g.id));
		}
	}
	
	return res;
}

bool Graph::LoadJson(const String& json, Vector<ValidationMessage>& errors)
{
	if(!LoadFromJson(doc, json)) {
		errors.Add(ValidationMessage(ValidationMessage::ERROR, "Failed to parse JSON"));
		return false;
	}
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

} // namespace Node

} // namespace Upp
