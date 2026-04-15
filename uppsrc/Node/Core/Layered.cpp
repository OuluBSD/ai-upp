#include "Algo.h"

namespace Upp {

namespace Node {

void LayeredLayout(const Graph& graph, Vector<NodeState>& states)
{
	const GraphDoc& doc = graph.GetDoc();
	int n = doc.nodes.GetCount();
	states.SetCount(n);
	if(n == 0) return;

	Index<EntityId> node_ids;
	for(const auto& node : doc.nodes) node_ids.Add(node.id);

	// 1. Ranking via longest path (topological order)
	Vector<int> rank(n, 0);
	Vector<EntityId> sorted = TopologicalSort(graph);
	for(const auto& id : sorted) {
		int ui = node_ids.Find(id);
		if(ui < 0) continue;
		for(const auto& e : doc.edges) {
			if(e.source_node == id) {
				int vi = node_ids.Find(e.target_node);
				if(vi >= 0)
					rank[vi] = max(rank[vi], rank[ui] + 1);
			}
		}
	}

	int max_rank = 0;
	for(int r : rank) max_rank = max(max_rank, r);

	// 2. Build layers
	Vector<Vector<int>> layers(max_rank + 1);
	for(int i = 0; i < n; i++)
		layers[rank[i]].Add(i);

	// 3. Barycenter crossing minimization (3 passes)
	for(int pass = 0; pass < 3; pass++) {
		// Forward: order each layer by average rank of predecessors in previous layer
		for(int r = 1; r <= max_rank; r++) {
			Vector<double> bary(layers[r].GetCount(), 0.0);
			// position map for previous layer
			VectorMap<int, double> prev_pos;
			for(int k = 0; k < layers[r-1].GetCount(); k++)
				prev_pos.Add(layers[r-1][k], (double)k);

			for(int k = 0; k < layers[r].GetCount(); k++) {
				int vi = layers[r][k];
				double sum = 0; int cnt = 0;
				for(const auto& e : doc.edges) {
					if(e.target_node == node_ids[vi]) {
						int ui = node_ids.Find(e.source_node);
						int p = prev_pos.Find(ui);
						if(p >= 0) { sum += prev_pos[p]; cnt++; }
					}
				}
				bary[k] = cnt > 0 ? sum / cnt : (double)k;
			}
			// Sort layer by bary value
			Vector<int> order(layers[r].GetCount());
			for(int k = 0; k < order.GetCount(); k++) order[k] = k;
			Sort(order, [&](int a, int b){ return bary[a] < bary[b]; });
			Vector<int> new_layer(layers[r].GetCount());
			for(int k = 0; k < order.GetCount(); k++) new_layer[k] = layers[r][order[k]];
			layers[r] = pick(new_layer);
		}
		// Backward pass
		for(int r = max_rank - 1; r >= 0; r--) {
			Vector<double> bary(layers[r].GetCount(), 0.0);
			VectorMap<int, double> next_pos;
			for(int k = 0; k < layers[r+1].GetCount(); k++)
				next_pos.Add(layers[r+1][k], (double)k);

			for(int k = 0; k < layers[r].GetCount(); k++) {
				int ui = layers[r][k];
				double sum = 0; int cnt = 0;
				for(const auto& e : doc.edges) {
					if(e.source_node == node_ids[ui]) {
						int vi = node_ids.Find(e.target_node);
						int p = next_pos.Find(vi);
						if(p >= 0) { sum += next_pos[p]; cnt++; }
					}
				}
				bary[k] = cnt > 0 ? sum / cnt : (double)k;
			}
			Vector<int> order(layers[r].GetCount());
			for(int k = 0; k < order.GetCount(); k++) order[k] = k;
			Sort(order, [&](int a, int b){ return bary[a] < bary[b]; });
			Vector<int> new_layer(layers[r].GetCount());
			for(int k = 0; k < order.GetCount(); k++) new_layer[k] = layers[r][order[k]];
			layers[r] = pick(new_layer);
		}
	}

	// 4. Coordinate assignment — center each layer vertically
	const double x_gap = 200.0;
	const double y_gap = 80.0;
	for(int r = 0; r <= max_rank; r++) {
		int cnt = layers[r].GetCount();
		double y_start = -(cnt - 1) * y_gap / 2.0;
		for(int k = 0; k < cnt; k++) {
			int ui = layers[r][k];
			states[ui].layout_pos = Pointf(r * x_gap, y_start + k * y_gap);
		}
	}
}

} // namespace Node

} // namespace Upp
