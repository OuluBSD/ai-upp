#include "Algo.h"

namespace Upp {

namespace Node {

Vector<EntityId> TopologicalSort(const Graph& graph)
{
	const GraphDoc& doc = graph.GetDoc();
	Vector<EntityId> res;
	
	Index<EntityId> node_ids;
	Vector<int> indegree;
	for(const auto& n : doc.nodes) {
		node_ids.Add(n.id);
		indegree.Add(0);
	}
	
	for(const auto& e : doc.edges) {
		int ti = node_ids.Find(e.target_node);
		if(ti >= 0) indegree[ti]++;
	}
	
	Vector<int> queue;
	for(int i = 0; i < indegree.GetCount(); i++)
		if(indegree[i] == 0)
			queue.Add(i);
	
	int head = 0;
	while(head < queue.GetCount()) {
		int ui = queue[head++];
		res.Add(node_ids[ui]);
		
		for(const auto& e : doc.edges) {
			if(e.source_node == node_ids[ui]) {
				int vi = node_ids.Find(e.target_node);
				if(vi >= 0) {
					indegree[vi]--;
					if(indegree[vi] == 0)
						queue.Add(vi);
				}
			}
		}
	}
	
	return res;
}

} // namespace Node

} // namespace Upp
