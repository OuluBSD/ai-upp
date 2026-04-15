#include "Algo.h"

namespace Upp {

namespace Node {

void OrderedTreeLayout(const Graph& graph, Vector<NodeState>& states)
{
	Vector<EntityId> order = TopologicalSort(graph);
	const GraphDoc& doc = graph.GetDoc();
	states.SetCount(doc.nodes.GetCount());
	
	Index<EntityId> node_ids;
	for(const auto& n : doc.nodes) node_ids.Add(n.id);
	
	int counter = 1;
	for(const auto& id : order) {
		int i = node_ids.Find(id);
		if(i >= 0) {
			int rank = (int)(log((double)counter) / log(2.0));
			int file = counter - (int)pow(2.0, (double)rank);
			states[i].layout_pos = Pointf(rank * 150, file * 100);
			counter++;
		}
	}
}

void TournamentTreeLayout(const Graph& graph, Vector<NodeState>& states)
{
	const GraphDoc& doc = graph.GetDoc();
	states.SetCount(doc.nodes.GetCount());
	
	int node_count = doc.nodes.GetCount();
	int level_count = (int)(log((double)node_count + 1.0) / log(2.0));
	
	for(int i = 0; i < node_count; i++) {
		int counter = i + 1;
		int depth = (int)(log((double)counter) / log(2.0));
		double offset = pow(2.0, (double)(level_count - depth));
		double x = offset + (counter - pow(2.0, (double)depth)) * pow(2.0, (double)(level_count - depth) + 1.0);
		states[i].layout_pos = Pointf(x * 50, depth * 100);
	}
}

} // namespace Node

} // namespace Upp
