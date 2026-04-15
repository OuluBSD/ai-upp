#include "Clipboard.h"

namespace Upp {

namespace Node {

String StoreClipboard(const Graph& graph, const Index<EntityId>& ids)
{
	ClipboardPayload p;
	const GraphDoc& src = graph.GetDoc();
	
	for(const auto& id : ids) {
		const NodeDoc* n = graph.FindNode(id);
		if(n) p.subgraph.nodes.Add() <<= *n;
	}
	
	for(const auto& e : src.edges) {
		if(ids.Find(e.source_node) >= 0 && ids.Find(e.target_node) >= 0)
			p.subgraph.edges.Add() <<= e;
	}
	
	return StoreAsJson(p);
}

bool LoadClipboard(Graph& graph, const String& data, Pointf paste_pos, Vector<ValidationMessage>& errors)
{
	ClipboardPayload p;
	if(!LoadFromJson(p, data)) {
		errors.Add(ValidationMessage(ValidationMessage::ERROR, "Invalid clipboard data"));
		return false;
	}
	
	Pointf min_p(1e300, 1e300);
	for(const auto& n : p.subgraph.nodes) {
		min_p.x = min(min_p.x, n.pos.x);
		min_p.y = min(min_p.y, n.pos.y);
	}
	if(min_p.x == 1e300) return true;
	
	Pointf offset = paste_pos - min_p;
	
	VectorMap<EntityId, EntityId> id_map;
	for(const auto& n : p.subgraph.nodes) {
		EntityId new_id = "n_" + Uuid::Create().ToString();
		id_map.Add(n.id, new_id);
		NodeDoc& nn = graph.AddNode(new_id);
		nn <<= n;
		nn.id = new_id;
		nn.pos += offset;
	}
	
	for(const auto& e : p.subgraph.edges) {
		EntityId new_id = "e_" + Uuid::Create().ToString();
		EntityId sn = id_map.Get(e.source_node, e.source_node);
		EntityId tn = id_map.Get(e.target_node, e.target_node);
		EdgeDoc& ne = graph.AddEdge(new_id, sn, e.source_pin, tn, e.target_pin);
		ne <<= e;
		ne.id = new_id;
		ne.source_node = sn;
		ne.target_node = tn;
	}
	
	return true;
}

} // namespace Node

} // namespace Upp
