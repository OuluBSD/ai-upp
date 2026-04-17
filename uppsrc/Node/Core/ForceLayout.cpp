#include "ForceLayout.h"

namespace Upp {

namespace Node {

// ---------------------------------------------------------------------------
// ForceRefine implementation
// ---------------------------------------------------------------------------

void ForceRefine(Vector<ForceItem>& items,
                 int                num_groups,
                 double             cell_size,
                 int                iters,
                 double             strength)
{
	int N = items.GetCount();
	if(N < 2) return;

	// ---- Pre-compute per-item bounding half-extents (centre-based) ----------
	// We work with node centres throughout; pos is top-left on input/output.
	// Internally keep centres in cx[]/cy[].
	Vector<double> cx(N), cy(N), hw(N), hh(N);
	for(int i = 0; i < N; i++) {
		hw[i] = items[i].sz.cx * 0.5;
		hh[i] = items[i].sz.cy * 0.5;
		cx[i] = items[i].pos.x + hw[i];
		cy[i] = items[i].pos.y + hh[i];
	}

	// ---- Pre-compute minimum gap for each pair (i, j) -----------------------
	// gap = 4 * cell_size + 0.5 * cell_size * max(port_i, port_j)
	// We store just per-item half-gap contribution so pair gap = gap_i + gap_j.
	Vector<double> half_gap(N);
	for(int i = 0; i < N; i++)
		half_gap[i] = (4.0 + 0.5 * items[i].port_count) * cell_size * 0.5;

	// ---- Force weights -------------------------------------------------------
	const double W_REPEL_NODE  = 1.0;   // node-node repulsion
	const double W_REPEL_GROUP = 0.4;   // group-group repulsion (applied to nodes)
	const double W_ATTRACT_GRP = 0.15;  // group centroid attraction
	const double W_ATTRACT_SCN = 0.05;  // scene centroid attraction

	// ---- Cooling schedule: linear decay from strength→0 ---------------------
	for(int iter = 0; iter < iters; iter++) {
		double t     = 1.0 - (double)iter / iters;   // 1→0
		double scale = strength * t;

		Vector<double> fx(N, 0.0), fy(N, 0.0);

		// ---- 1. Node-node repulsion ------------------------------------------
		for(int i = 0; i < N; i++) {
			for(int j = i + 1; j < N; j++) {
				double dx = cx[i] - cx[j];
				double dy = cy[i] - cy[j];
				double dist2 = dx*dx + dy*dy;
				double target_gap = half_gap[i] + half_gap[j]
				                  + (hw[i] + hw[j] + hh[i] + hh[j]) * 0.5;
				double min_dist = target_gap;
				if(dist2 < 1e-6) { dx = 1; dy = 0; dist2 = 1; }
				double dist = sqrt(dist2);
				if(dist < min_dist * 2.5) {
					// Repulsion proportional to penetration depth
					double pen = min_dist - dist;
					double f   = W_REPEL_NODE * (pen + max(0.0, min_dist - dist)) / dist;
					fx[i] += f * dx;  fy[i] += f * dy;
					fx[j] -= f * dx;  fy[j] -= f * dy;
				}
			}
		}

		// ---- 2. Group centroid attraction + group-group repulsion -----------
		if(num_groups > 0) {
			// Compute per-group centroids
			Vector<double> gcx(num_groups, 0), gcy(num_groups, 0);
			Vector<int>    gcn(num_groups, 0);
			for(int i = 0; i < N; i++) {
				int g = items[i].group_id;
				if(g >= 0 && g < num_groups) {
					gcx[g] += cx[i]; gcy[g] += cy[i]; gcn[g]++;
				}
			}
			for(int g = 0; g < num_groups; g++)
				if(gcn[g] > 0) { gcx[g] /= gcn[g]; gcy[g] /= gcn[g]; }

			// Per-node: attract toward own group centroid
			for(int i = 0; i < N; i++) {
				int g = items[i].group_id;
				if(g < 0 || g >= num_groups || gcn[g] < 2) continue;
				double dx = gcx[g] - cx[i], dy = gcy[g] - cy[i];
				double d  = sqrt(dx*dx + dy*dy);
				if(d > 1.0) {
					double f = W_ATTRACT_GRP * d;
					fx[i] += f * dx / d;
					fy[i] += f * dy / d;
				}
			}

			// Group-group repulsion: repel group centroids, apply to member nodes
			for(int ga = 0; ga < num_groups; ga++) {
				if(gcn[ga] == 0) continue;
				for(int gb = ga + 1; gb < num_groups; gb++) {
					if(gcn[gb] == 0) continue;
					double dx = gcx[ga] - gcx[gb];
					double dy = gcy[ga] - gcy[gb];
					double dist2 = dx*dx + dy*dy;
					if(dist2 < 1e-6) { dx = 1; dist2 = 1; }
					double dist = sqrt(dist2);
					// Repulsion zone: 3× average node diagonal
					double zone = cell_size * 30.0;
					if(dist < zone) {
						double f = W_REPEL_GROUP * (zone - dist) / dist;
						// Apply equally to all nodes in each group
						for(int i = 0; i < N; i++) {
							if(items[i].group_id == ga) {
								fx[i] += f * dx; fy[i] += f * dy;
							} else if(items[i].group_id == gb) {
								fx[i] -= f * dx; fy[i] -= f * dy;
							}
						}
					}
				}
			}
		}

		// ---- 3. Scene centroid attraction ------------------------------------
		{
			double scx = 0, scy = 0;
			for(int i = 0; i < N; i++) { scx += cx[i]; scy += cy[i]; }
			scx /= N; scy /= N;
			for(int i = 0; i < N; i++) {
				double dx = scx - cx[i], dy = scy - cy[i];
				double d  = sqrt(dx*dx + dy*dy);
				if(d > 1.0) {
					double f = W_ATTRACT_SCN * d;
					fx[i] += f * dx / d;
					fy[i] += f * dy / d;
				}
			}
		}

		// ---- Integrate -------------------------------------------------------
		double max_move = cell_size * 4.0 * t + cell_size * 0.5;
		for(int i = 0; i < N; i++) {
			double mx = fx[i] * scale, my = fy[i] * scale;
			// Clamp step size to prevent explosion
			double mag = sqrt(mx*mx + my*my);
			if(mag > max_move) { mx = mx/mag*max_move; my = my/mag*max_move; }
			cx[i] += mx;
			cy[i] += my;
		}
	}

	// ---- Write centres back as top-left positions ---------------------------
	for(int i = 0; i < N; i++) {
		items[i].pos.x = cx[i] - hw[i];
		items[i].pos.y = cy[i] - hh[i];
	}
}

// ---------------------------------------------------------------------------
// ForceRefineGraph: convenience wrapper for Graph
// ---------------------------------------------------------------------------

void ForceRefineGraph(Graph& graph, int iters)
{
	const GraphDoc& doc = graph.GetDoc();

	// Map group vfs_path → integer id
	VectorMap<String, int> group_idx;
	for(int i = 0; i < doc.groups.GetCount(); i++)
		group_idx.Add(doc.groups[i].vfs_path, i);
	int num_groups = doc.groups.GetCount();

	// Build a set of which nodes belong to which group
	VectorMap<String, int> node_group;
	for(int gi = 0; gi < doc.groups.GetCount(); gi++)
		for(const EntityId& nid : doc.groups[gi].nodes)
			node_group.Add(nid, gi);

	// Compute cell_size from smallest node dimension
	double min_dim = 1e18;
	for(const NodeDoc& n : doc.nodes) {
		if(n.sz.cx > 1) min_dim = min(min_dim, (double)n.sz.cx);
		if(n.sz.cy > 1) min_dim = min(min_dim, (double)n.sz.cy);
	}
	double cell_size = (min_dim < 1e17) ? clamp(min_dim / 10.0, 1.0, 100.0) : 10.0;

	// Build ForceItem list
	Vector<ForceItem> fitems;
	fitems.SetCount(doc.nodes.GetCount());
	for(int i = 0; i < doc.nodes.GetCount(); i++) {
		const NodeDoc& n = doc.nodes[i];
		fitems[i].pos   = n.pos;
		fitems[i].sz    = n.sz.cx > 0 && n.sz.cy > 0
		                    ? Sizef(n.sz.cx, n.sz.cy)
		                    : Sizef(200, 60);
		int gi = node_group.Find(n.id);
		fitems[i].group_id   = (gi >= 0) ? node_group[gi] : -1;
		// port_count = max(in, out)
		int in_c = 0, out_c = 0;
		for(const PinDoc& p : n.pins)
			(p.kind == PinKind::Output ? out_c : in_c)++;
		fitems[i].port_count = max(in_c, out_c);
	}

	ForceRefine(fitems, num_groups, cell_size, iters);

	// Write back
	for(int i = 0; i < doc.nodes.GetCount(); i++) {
		NodeDoc* n = graph.FindNode(doc.nodes[i].id);
		if(n) {
			n->pos = fitems[i].pos;
			graph.Invalidate(n->id);
		}
	}
}

} // namespace Node

} // namespace Upp
