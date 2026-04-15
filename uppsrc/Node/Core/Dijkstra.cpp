#include "Algo.h"

namespace Upp {

namespace Node {

Vector<PathNode> Dijkstra(const Graph& graph, const EntityId& start_node)
{
	const GraphDoc& doc = graph.GetDoc();
	Vector<PathNode> res;
	
	Index<EntityId> node_ids;
	for(const auto& n : doc.nodes) {
		node_ids.Add(n.id);
		auto& pn = res.Add();
		pn.entity_id = n.id;
		if(n.id == start_node)
			pn.distance = 0;
	}
	
	Vector<bool> optimized;
	optimized.SetCount(res.GetCount(), false);
	
	for(int count = 0; count < res.GetCount(); count++) {
		int u = -1;
		double min_dist = 1e300;
		
		for(int i = 0; i < res.GetCount(); i++) {
			if(!optimized[i] && res[i].distance <= min_dist) {
				min_dist = res[i].distance;
				u = i;
			}
		}
		
		if(u == -1 || min_dist == 1e300) break;
		
		optimized[u] = true;
		
		for(const auto& e : doc.edges) {
			int v = -1;
			if(e.source_node == node_ids[u]) v = node_ids.Find(e.target_node);
			else if(!e.directed && e.target_node == node_ids[u]) v = node_ids.Find(e.source_node);
			
			if(v >= 0 && !optimized[v]) {
				double alt = res[u].distance + e.weight;
				if(alt < res[v].distance) {
					res[v].distance = alt;
					res[v].predecessor = node_ids[u];
				}
			}
		}
	}
	
	return res;
}

} // namespace Node

} // namespace Upp
