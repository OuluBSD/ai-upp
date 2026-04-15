#include "Layout.h"

namespace Upp {

namespace Node {

void SpringLayout::Run(Graph& graph, Vector<NodeState>& states)
{
	GraphDoc& doc = graph.GetDoc();
	if(states.GetCount() != doc.nodes.GetCount())
		states.SetCount(doc.nodes.GetCount());
	
	// Initialize positions if needed
	for(int i = 0; i < states.GetCount(); i++) {
		if(states[i].layout_pos.x == 0 && states[i].layout_pos.y == 0) {
			states[i].layout_pos = Pointf(i * 10, i * 10);
		}
		states[i].layout_force = Pointf(0, 0);
	}
	
	// Build id→index map once so edge force loop is O(E) not O(E*N)
	Index<EntityId> node_idx;
	for(const auto& n : doc.nodes)
		node_idx.Add(n.id);

	SeedRandom(seed);

	for(int iter = 0; iter < iterations; iter++) {
		// Repulsive
		for(int i = 0; i < states.GetCount(); i++) {
			for(int j = 0; j < i; j++) {
				double dx = states[j].layout_pos.x - states[i].layout_pos.x;
				double dy = states[j].layout_pos.y - states[i].layout_pos.y;
				double d2 = dx * dx + dy * dy;
				if(d2 < 0.01) {
					dx = 0.1 * Randomf() + 0.1;
					dy = 0.1 * Randomf() + 0.1;
					d2 = dx * dx + dy * dy;
				}
				double d = sqrt(d2);
				if(d < max_repulsive_dist) {
					double f = k * k / d;
					states[j].layout_force.x += f * dx / d;
					states[j].layout_force.y += f * dy / d;
					states[i].layout_force.x -= f * dx / d;
					states[i].layout_force.y -= f * dy / d;
				}
			}
		}
		
		// Attractive
		for(const auto& e : doc.edges) {
			int si = node_idx.Find(e.source_node);
			int ti = node_idx.Find(e.target_node);
			if(si >= 0 && ti >= 0) {
				double dx = states[ti].layout_pos.x - states[si].layout_pos.x;
				double dy = states[ti].layout_pos.y - states[si].layout_pos.y;
				double d2 = dx * dx + dy * dy;
				if(d2 < 0.01) {
					dx = 0.1 * Randomf() + 0.1;
					dy = 0.1 * Randomf() + 0.1;
					d2 = dx * dx + dy * dy;
				}
				double d = sqrt(d2);
				if(d > max_repulsive_dist) {
					d = max_repulsive_dist;
					d2 = d * d;
				}
				double f = (d2 - k * k) / k;
				double attr = e.attraction > 0 ? e.attraction : 1.0;
				f *= log(attr) * 0.5 + 1.0;
				
				states[ti].layout_force.x -= f * dx / d;
				states[ti].layout_force.y -= f * dy / d;
				states[si].layout_force.x += f * dx / d;
				states[si].layout_force.y += f * dy / d;
			}
		}
		
		// Update
		for(int i = 0; i < states.GetCount(); i++) {
			double mx = c * states[i].layout_force.x;
			double my = c * states[i].layout_force.y;
			mx = max(-max_vertex_move, min(max_vertex_move, mx));
			my = max(-max_vertex_move, min(max_vertex_move, my));
			states[i].layout_pos.x += mx;
			states[i].layout_pos.y += my;
			states[i].layout_force = Pointf(0, 0);
		}
	}
}

} // namespace Node

} // namespace Upp
