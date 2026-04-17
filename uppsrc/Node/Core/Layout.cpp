#include "Layout.h"

namespace Upp {

namespace Node {

// ---------------------------------------------------------------------------
// SmartPacker implementation
// ---------------------------------------------------------------------------

Rectf SmartPacker::GetNodeBounds(const NodeDoc& n)
{
	// Estimate node size based on label and type
	// Default: 150x60, adjust based on content
	double w = 150.0;
	double h = 60.0;
	
	// Rough estimate: longer labels = wider nodes
	if(n.label.GetCount() > 20)
		w = 150.0 + (n.label.GetCount() - 20) * 4.0;
	
	// Cap at reasonable size
	w = min(w, 300.0);
	h = max(h, 60.0);
	
	return Rectf(n.pos.x, n.pos.y, n.pos.x + w, n.pos.y + h);
}

void SmartPacker::AnalyzeGraph(Graph& graph)
{
	items.Clear();
	
	const GraphDoc& doc = graph.GetDoc();
	
	// Step 1: Create items for all groups
	for(int i = 0; i < doc.groups.GetCount(); i++) {
		const GroupDoc& g = doc.groups[i];
		ConnectionInfo item;
		item.id = g.vfs_path;
		item.is_group = true;
		item.node_count = g.nodes.GetCount();
		items.Add(pick(item));
	}
	
	// Step 2: Find ungrouped nodes and create items for them
	// A node is ungrouped if its ID doesn't match any group's vfs_path prefix
	for(int i = 0; i < doc.nodes.GetCount(); i++) {
		const NodeDoc& n = doc.nodes[i];
		bool in_group = false;
		for(int j = 0; j < doc.groups.GetCount(); j++) {
			const GroupDoc& g = doc.groups[j];
			if(n.id.StartsWith(g.vfs_path.Mid(1) + "_") || n.id.StartsWith(g.vfs_path + "/")) {
				in_group = true;
				break;
			}
		}
		if(!in_group) {
			ConnectionInfo item;
			item.id = n.id;
			item.is_group = false;
			item.node_count = 1;
			items.Add(pick(item));
		}
	}
	
	// Step 3: Estimate bounds for all items
	for(int i = 0; i < items.GetCount(); i++)
		EstimateItemBounds(items[i], graph);
	
	// Step 4: Build connection graph
	for(int i = 0; i < items.GetCount(); i++) {
		for(int j = i + 1; j < items.GetCount(); j++) {
			int count = CountConnections(items[i].id, items[i].is_group, items[j].id, items[j].is_group, graph);
			if(count > 0) {
				items[i].connection_ids.Add(items[j].id);
				items[i].connection_counts.Add(count);
				items[j].connection_ids.Add(items[i].id);
				items[j].connection_counts.Add(count);
			}
		}
	}
}

int SmartPacker::GetTotalConnectionCount(const ConnectionInfo& item) const
{
	int total = 0;
	for(int i = 0; i < item.connection_counts.GetCount(); i++)
		total += item.connection_counts[i];
	return total;
}

int SmartPacker::CountConnections(const String& id1, bool is_group1, const String& id2, bool is_group2, Graph& graph)
{
	int count = 0;
	const GraphDoc& doc = graph.GetDoc();
	
	for(int i = 0; i < doc.edges.GetCount(); i++) {
		const EdgeDoc& e = doc.edges[i];
		String from_group, to_group;
		String from_node = e.source_node;
		String to_node = e.target_node;
		
		// Determine which group/item each node belongs to
		if(is_group1) {
			String prefix1 = id1.Mid(1) + "_";
			if(from_node.StartsWith(prefix1) || from_node.StartsWith(id1 + "/"))
				from_group = id1;
			if(to_node.StartsWith(prefix1) || to_node.StartsWith(id1 + "/"))
				to_group = id1;
		} else {
			if(from_node == id1) from_group = id1;
			if(to_node == id1) to_group = id1;
		}
		
		if(is_group2) {
			String prefix2 = id2.Mid(1) + "_";
			if(from_node.StartsWith(prefix2) || from_node.StartsWith(id2 + "/"))
				from_group = id2;
			if(to_node.StartsWith(prefix2) || to_node.StartsWith(id2 + "/"))
				to_group = id2;
		} else {
			if(from_node == id2) from_group = id2;
			if(to_node == id2) to_group = id2;
		}
		
		// Count if edge connects the two items
		if(((from_group == id1 && to_group == id2) || (from_group == id2 && to_group == id1)))
			count++;
	}
	
	return count;
}

void SmartPacker::EstimateItemBounds(ConnectionInfo& item, Graph& graph)
{
	if(item.is_group) {
		// Group bounds: estimate based on node count
		// Assume nodes arranged in grid inside group
		int cols = max(1, (int)sqrt(item.node_count));
		int rows = (item.node_count + cols - 1) / cols;

		// Node dimensions with spacing - conservative estimates
		// Scene.cpp computes: TITLE_H + pin_rows*PIN_ROW_H + slot_h + 8
		double node_w = 200.0;
		double node_h = 150.0;
		double col_w = node_w + node_padding * 3;  // 260px
		double row_h = node_h + node_padding * 3;  // 210px

		double w = cols * col_w + group_inner_padding * 2;
		double h = rows * row_h + group_inner_padding * 2;

		item.bounds = Rectf(0, 0, w, h);
	} else {
		// Ungrouped node: use estimated node bounds
		const NodeDoc* n = graph.FindNode(item.id);
		if(n)
			item.bounds = GetNodeBounds(*n);
		else
			item.bounds = Rectf(0, 0, 200.0, 150.0);
	}
}

void SmartPacker::PackNodesInGroupAtPosition(Graph& graph, const String& group_path, Pointf pos)
{
	// Find group by vfs_path (not by EntityId)
	const GroupDoc* grp = nullptr;
	const GraphDoc& doc = graph.GetDoc();
	for(int i = 0; i < doc.groups.GetCount(); i++) {
		if(doc.groups[i].vfs_path == group_path) {
			grp = &doc.groups[i];
			break;
		}
	}
	
	if(!grp) return;
	
	// Collect nodes in this group
	Vector<NodeDoc*> nodes;
	for(int i = 0; i < grp->nodes.GetCount(); i++) {
		NodeDoc* n = graph.FindNode(grp->nodes[i]);
		if(n) nodes.Add(n);
	}
	
	if(nodes.IsEmpty()) return;
	
	// Simple grid packing inside group
	int cols = max(1, (int)sqrt(nodes.GetCount()));
	int rows = (nodes.GetCount() + cols - 1) / cols;
	
	// Node dimensions: Scene.cpp computes height as TITLE_H + pin_rows*PIN_ROW_H + slot_h + 8
	// Use conservative estimates that account for nodes with many pins
	// Typical node: 100-200px wide, 80-150px tall depending on pins
	double node_w = 200.0;  // Typical node width
	double node_h = 150.0;  // Conservative height estimate
	
	// Spacing between nodes - must provide enough gap to prevent overlap
	// even if nodes are taller than estimated
	double col_w = node_w + node_padding * 3;  // 200 + 60 = 260px
	double row_h = node_h + node_padding * 3;  // 150 + 60 = 210px
	
	// Center the grid within the group bounds
	double start_x = pos.x + group_inner_padding;
	double start_y = pos.y + group_inner_padding;
	
	int idx = 0;
	for(int r = 0; r < rows; r++) {
		for(int c = 0; c < cols && idx < nodes.GetCount(); c++) {
			NodeDoc* n = nodes[idx++];
			n->pos = Pointf(start_x + c * col_w, start_y + r * row_h);
			graph.Invalidate(n->id);
		}
	}
}

void SmartPacker::PackGlobal()
{
	if(items.IsEmpty()) return;
	
	if(orientation == LAYOUT_TALL) {
		// Shelf packing - arrange in horizontal rows
		struct Shelf {
			double y = 0;
			double h = 0;
			double x = 0;  // Current x position in this shelf
		};

		Vector<Shelf> shelves;
		double cursor_x = group_padding;
		double cursor_y = group_padding;
		
		// Use viewport width if available, otherwise use a reasonable default
		double available_width = has_viewport ? (viewport.Width() - 2 * group_padding) : 2000.0;

		for(int i = 0; i < items.GetCount(); i++) {
			ConnectionInfo& item = items[i];
			double w = item.bounds.Width();
			double h = item.bounds.Height();

			// Try to fit in current shelf
			bool fits_in_current = !shelves.IsEmpty() && (cursor_x + w + group_padding <= available_width);

			if(fits_in_current) {
				// Place in current shelf
				Shelf& shelf = shelves[shelves.GetCount()-1];
				item.bounds = Rectf(cursor_x, shelf.y, cursor_x + w, shelf.y + h);
				shelf.x = cursor_x + w + group_padding;
				shelf.h = max(shelf.h, h);
				cursor_x = shelf.x;
			} else {
				// Start new shelf
				Shelf shelf;
				shelf.y = cursor_y;
				shelf.h = h;
				
				item.bounds = Rectf(group_padding, cursor_y, group_padding + w, cursor_y + h);
				
				shelf.x = group_padding + w + group_padding;
				cursor_y += h + group_padding * 4;  // Less vertical spacing
				cursor_x = shelf.x;

				shelves.Add(shelf);
			}
		}
	} else {
		// WIDE orientation - LAYOUT_WIDE
		struct Column {
			double x = 0;
			double w = 0;
			double y = 0;  // Current y position in this column
		};

		Vector<Column> columns;
		double cursor_x = group_padding;

		// Use viewport height if available
		double available_height = has_viewport ? (viewport.Height() - 2 * group_padding) : 2000.0;

		for(int i = 0; i < items.GetCount(); i++) {
			ConnectionInfo& item = items[i];
			double w = item.bounds.Width();
			double h = item.bounds.Height();

			// Try to fit in current column
			bool fits_in_current = !columns.IsEmpty() && 
				(columns[columns.GetCount()-1].y + h + group_padding <= available_height);

			if(fits_in_current) {
				// Place in current column
				Column& col = columns[columns.GetCount()-1];
				item.bounds = Rectf(col.x, col.y, col.x + w, col.y + h);
				col.y = col.y + h + group_padding;
				col.w = max(col.w, w);
			} else {
				// Start new column
				Column col;
				col.x = cursor_x;
				col.w = w;
				col.y = group_padding + h + group_padding;

				item.bounds = Rectf(cursor_x, group_padding, cursor_x + w, group_padding + h);
				
				cursor_x += w + group_padding * 2;  // Compact horizontal spacing for WIDE

				columns.Add(col);
			}
		}
	}
}

void SmartPacker::AdjustAspectRatio()
{
	if(!has_viewport) return;
	
	// Compute current layout bounds
	Rectf layout_bounds;
	for(int i = 0; i < items.GetCount(); i++)
		layout_bounds.Union(items[i].bounds);
	
	// Target aspect ratio from viewport
	double target_ratio = viewport.Width() / viewport.Height();
	if(target_ratio < 0.5) target_ratio = 1.0;
	
	// Current layout aspect ratio
	double layout_ratio = layout_bounds.Width() / layout_bounds.Height();
	
	// Only adjust if significantly different from target
	if(layout_ratio > target_ratio * 1.5 || layout_ratio < target_ratio / 1.5) {
		// Calculate scale to fit viewport with margin
		double scale_x = viewport.Width() * 0.9 / layout_bounds.Width();
		double scale_y = viewport.Height() * 0.9 / layout_bounds.Height();
		double scale = min(scale_x, scale_y);
		
		// Don't scale down - only scale up if needed
		if(scale > 1.0) {
			for(int i = 0; i < items.GetCount(); i++) {
				items[i].bounds.left *= scale;
				items[i].bounds.right *= scale;
				items[i].bounds.top *= scale;
				items[i].bounds.bottom *= scale;
			}
		}
	}
}

void SmartPacker::Pack(Graph& graph)
{
	// Step 1: Analyze graph structure
	AnalyzeGraph(graph);

	LOG("SmartPacker::Pack: items.GetCount()=" << items.GetCount() 
	    << " groups=" << graph.GetDoc().groups.GetCount() 
	    << " nodes=" << graph.GetDoc().nodes.GetCount());

	// Step 1.5: Sort items to place ungrouped nodes near their connected groups
	// Strategy: For each ungrouped node, find the group it connects to most,
	// then place the ungrouped node immediately after that group
	// For unconnected ungrouped nodes, place them after the first group
	Array<ConnectionInfo> sorted;
	Vector<bool> used(items.GetCount(), false);

	// Collect ungrouped nodes that have no connections
	Vector<int> ungrouped_indices;
	
	// Place groups first, with their connected ungrouped nodes
	bool placed_first_group = false;
	for(int i = 0; i < items.GetCount(); i++) {
		if(used[i]) continue;
		if(!items[i].is_group) {
			// This is an ungrouped node - collect it for later
			ungrouped_indices.Add(i);
			continue;
		}

		// Place this group
		sorted.Add(pick(items[i]));
		used[i] = true;
		placed_first_group = true;

		// If there are unconnected ungrouped nodes, place the first one after this first group
		// This keeps unconnected nodes visually near the layout start
		if(placed_first_group && ungrouped_indices.GetCount() > 0) {
			// Only place ungrouped nodes that have NO connections at all
			for(int m = ungrouped_indices.GetCount()-1; m >= 0; m--) {
				int idx = ungrouped_indices[m];
				if(items[idx].connection_ids.GetCount() == 0) {
					// This node has no connections - place it near first group
					sorted.Add(pick(items[idx]));
					used[idx] = true;
					ungrouped_indices.Remove(m);
				}
			}
		}

		// Now place any ungrouped nodes that connect to this group
		for(int j = 0; j < items.GetCount(); j++) {
			if(used[j]) continue;
			if(items[j].is_group) continue;

			// Check if this ungrouped node connects to the group we just placed
			const ConnectionInfo& last_group = sorted.Top();
			bool connects = false;
			for(int k = 0; k < last_group.connection_ids.GetCount(); k++) {
				if(last_group.connection_ids[k] == items[j].id) {
					connects = true;
					break;
				}
			}
			if(connects) {
				sorted.Add(pick(items[j]));
				used[j] = true;
				// Remove from ungrouped_indices if present
				for(int m = ungrouped_indices.GetCount()-1; m >= 0; m--) {
					if(ungrouped_indices[m] == j) {
						ungrouped_indices.Remove(m);
						break;
					}
				}
			}
		}
	}

	// Place remaining ungrouped nodes (unconnected ones) after the last group
	for(int idx : ungrouped_indices) {
		if(!used[idx]) {
			sorted.Add(pick(items[idx]));
			used[idx] = true;
		}
	}

	// Add any remaining items (shouldn't happen, but just in case)
	for(int i = 0; i < items.GetCount(); i++) {
		if(!used[i])
			sorted.Add(pick(items[i]));
	}

	items.Clear();
	for(int i = 0; i < sorted.GetCount(); i++)
		items.Add(pick(sorted[i]));
	sorted.Clear();

	// Step 2: Global packing (groups + ungrouped nodes) FIRST
	// This determines where each group will be placed
	PackGlobal();
	
	// Step 3: Adjust aspect ratio to fit viewport
	AdjustAspectRatio();
	
	// Step 4: Pack nodes inside each group, offset by group position
	for(int i = 0; i < items.GetCount(); i++) {
		const ConnectionInfo& item = items[i];
		if(item.is_group) {
			PackNodesInGroupAtPosition(graph, item.id, item.bounds.TopLeft());
		} else {
			// Ungrouped node: apply position directly
			NodeDoc* n = graph.FindNode(item.id);
			if(n)
				n->pos = Pointf(item.bounds.left, item.bounds.top);
		}
	}
	
	// Step 5: Validate layout
	ValidateLayout(graph);
}

void SmartPacker::ValidateLayout(Graph& graph)
{
	const GraphDoc& doc = graph.GetDoc();
	int node_overlaps = 0;
	int group_overlaps = 0;
	
	// Check node-node overlaps
	for(int i = 0; i < doc.nodes.GetCount(); i++) {
		const NodeDoc& a = doc.nodes[i];
		Rectf rect_a(a.pos.x, a.pos.y, a.pos.x + a.sz.cx, a.pos.y + a.sz.cy);
		
		for(int j = i + 1; j < doc.nodes.GetCount(); j++) {
			const NodeDoc& b = doc.nodes[j];
			Rectf rect_b(b.pos.x, b.pos.y, b.pos.x + b.sz.cx, b.pos.y + b.sz.cy);
			
			if(rect_a.Intersects(rect_b)) {
				node_overlaps++;
				LOG("OVERLAP: " << a.id << " " << rect_a << " vs " << b.id << " " << rect_b);
			}
		}
	}
	
	// Check group-group overlaps (based on node bounds)
	for(int i = 0; i < doc.groups.GetCount(); i++) {
		const GroupDoc& ga = doc.groups[i];
		Rectf bounds_a = ComputeGroupBounds(graph, ga);
		
		for(int j = i + 1; j < doc.groups.GetCount(); j++) {
			const GroupDoc& gb = doc.groups[j];
			Rectf bounds_b = ComputeGroupBounds(graph, gb);
			
			if(bounds_a.Intersects(bounds_b)) {
				group_overlaps++;
				LOG("GROUP OVERLAP: " << ga.vfs_path << " " << bounds_a << " vs " << gb.vfs_path << " " << bounds_b);
			}
		}
	}
	
	if(node_overlaps > 0 || group_overlaps > 0) {
		LOG("LAYOUT VALIDATION FAILED: " << node_overlaps << " node overlaps, " 
		    << group_overlaps << " group overlaps");
	} else {
		LOG("LAYOUT VALIDATION PASSED: No overlaps detected");
	}
}

Rectf SmartPacker::ComputeGroupBounds(Graph& graph, const GroupDoc& g)
{
	Rectf bounds(1e300, 1e300, -1e300, -1e300);
	bool first = true;

	for(const auto& nid : g.nodes) {
		const NodeDoc* node = graph.FindNode(nid);
		if(!node) continue;

		Rectf node_rect(node->pos.x, node->pos.y,
		                node->pos.x + node->sz.cx, node->pos.y + node->sz.cy);

		if(first) {
			bounds = node_rect;
			first = false;
		} else {
			bounds.Union(node_rect);
		}
	}

	if(first) return Rectf(0, 0, 200, 200);

	// Add padding
	const double GROUP_PAD = 40.0;
	const double TITLE_H = 30.0;
	bounds.left -= GROUP_PAD;
	bounds.top -= TITLE_H + GROUP_PAD;
	bounds.right += GROUP_PAD;
	bounds.bottom += GROUP_PAD;

	return bounds;
}

// ---------------------------------------------------------------------------
// ScriptedLayout implementation
// ---------------------------------------------------------------------------

// Compute the bounding box of a grid layout with a given column count,
// without applying positions. Returns Sizef(total_w, total_h).
static Sizef ComputeGridSize(const Vector<NodeDoc*>& nodes, const Vector<int>& order,
                              int cols, double node_padding)
{
	int N = nodes.GetCount();
	if(N == 0 || cols <= 0) return Sizef(0, 0);
	int rows = (N + cols - 1) / cols;

	// Per-column max-width, per-row max-height
	Vector<double> col_w(cols, 0.0);
	Vector<double> row_h(rows, 0.0);

	for(int oi = 0; oi < N; oi++) {
		int i   = order[oi];
		int col = oi % cols;
		int row = oi / cols;
		double w = nodes[i]->sz.cx > 0 ? nodes[i]->sz.cx : 200.0;
		double h = nodes[i]->sz.cy > 0 ? nodes[i]->sz.cy : 150.0;
		col_w[col] = max(col_w[col], w);
		row_h[row] = max(row_h[row], h);
	}

	double total_w = 0, total_h = 0;
	for(double cw : col_w) total_w += cw + node_padding;
	for(double rh : row_h) total_h += rh + node_padding;
	// Remove trailing padding
	if(total_w > node_padding) total_w -= node_padding;
	if(total_h > node_padding) total_h -= node_padding;

	return Sizef(total_w, total_h);
}

// Apply a grid layout with a given column count and return tight bounding box.
static Rectf ApplyGridLayout(Graph& graph, const Vector<NodeDoc*>& nodes,
                              const Vector<int>& order, int cols, double node_padding)
{
	int N = nodes.GetCount();
	if(N == 0 || cols <= 0) return Rectf(0, 0, 0, 0);
	int rows = (N + cols - 1) / cols;

	Vector<double> col_w(cols, 0.0);
	Vector<double> row_h(rows, 0.0);

	for(int oi = 0; oi < N; oi++) {
		int i   = order[oi];
		int col = oi % cols;
		int row = oi / cols;
		double w = nodes[i]->sz.cx > 0 ? nodes[i]->sz.cx : 200.0;
		double h = nodes[i]->sz.cy > 0 ? nodes[i]->sz.cy : 150.0;
		col_w[col] = max(col_w[col], w);
		row_h[row] = max(row_h[row], h);
	}

	// Column/row offsets
	Vector<double> col_x(cols, 0.0), row_y(rows, 0.0);
	for(int c = 1; c < cols; c++) col_x[c] = col_x[c-1] + col_w[c-1] + node_padding;
	for(int r = 1; r < rows; r++) row_y[r] = row_y[r-1] + row_h[r-1] + node_padding;

	Rectf tight(0, 0, 0, 0);
	bool first = true;
	for(int oi = 0; oi < N; oi++) {
		int i   = order[oi];
		int col = oi % cols;
		int row = oi / cols;
		double x = col_x[col];
		double y = row_y[row];
		double w = nodes[i]->sz.cx > 0 ? nodes[i]->sz.cx : 200.0;
		double h = nodes[i]->sz.cy > 0 ? nodes[i]->sz.cy : 150.0;

		nodes[i]->pos = Pointf(x, y);
		graph.Invalidate(nodes[i]->id);

		Rectf nr(x, y, x + w, y + h);
		if(first) { tight = nr; first = false; }
		else       { tight.Union(nr); }
	}
	return tight;
}

Rectf ScriptedLayout::PackGroupNodes(Graph& graph, const GroupDoc& grp)
{
	// Collect member nodes
	Vector<NodeDoc*> nodes;
	for(const EntityId& nid : grp.nodes) {
		NodeDoc* n = graph.FindNode(nid);
		if(n) nodes.Add(n);
	}
	int N = nodes.GetCount();
	if(N == 0) return Rectf(0, 0, 0, 0);

	// Build topological order so nodes read left-to-right / top-to-bottom
	// in the direction of data flow.
	const GraphDoc& doc = graph.GetDoc();
	VectorMap<String, int> idx_map;
	for(int i = 0; i < N; i++)
		idx_map.Add(nodes[i]->id, i);

	struct Arc : Moveable<Arc> {
		int from, to;
		Arc(int f, int t) : from(f), to(t) {}
	};
	Vector<int>  in_degree(N, 0);
	Vector<Arc>  arcs;
	for(const EdgeDoc& e : doc.edges) {
		int fi = idx_map.Find(e.source_node);
		int ti = idx_map.Find(e.target_node);
		if(fi >= 0 && ti >= 0 && fi != ti) {
			arcs.Add(Arc(fi, ti));
			in_degree[ti]++;
		}
	}
	Vector<int> order;
	{
		Vector<int> queue;
		for(int i = 0; i < N; i++)
			if(in_degree[i] == 0) queue.Add(i);
		Vector<int> deg = clone(in_degree);
		while(!queue.IsEmpty()) {
			int cur = queue[0]; queue.Remove(0);
			order.Add(cur);
			for(const Arc& a : arcs)
				if(a.from == cur && --deg[a.to] == 0)
					queue.Add(a.to);
		}
		Vector<bool> vis(N, false);
		for(int i : order) vis[i] = true;
		for(int i = 0; i < N; i++) if(!vis[i]) order.Add(i);
	}

	// Look up the prescribed area for this group so we can score candidates.
	// If not available, use a square-ish target aspect ratio.
	double target_ar = 1.0;  // width / height
	int ri = group_rects.Find(grp.vfs_path);
	if(ri >= 0) {
		double pw = group_rects[ri].Width()  - 2 * inner_padding_;
		double ph = group_rects[ri].Height() - 2 * inner_padding_;
		if(ph > 0) target_ar = pw / ph;
	}

	// Try every column count from 1 to N and score by how well the aspect
	// ratio matches the target.  Prefer layouts that are slightly wider than
	// tall (to match left-to-right reading of a data-flow graph).
	// Score = |log(actual_ar / target_ar)| — lower is better.
	int best_cols = 1;
	double best_score = 1e300;
	for(int cols = 1; cols <= N; cols++) {
		Sizef sz = ComputeGridSize(nodes, order, cols, node_padding_);
		if(sz.cy <= 0) continue;
		double ar = sz.cx / sz.cy;
		double score = fabs(log(ar / target_ar));
		if(score < best_score) {
			best_score = score;
			best_cols  = cols;
		}
	}

	return ApplyGridLayout(graph, nodes, order, best_cols, node_padding_);
}

void ScriptedLayout::Run(Graph& graph)
{
	const GraphDoc& doc = graph.GetDoc();

	// --- Step 1: Determine scale factor from the reference node (if provided) ---
	// The reference tells us: in source coords, node80 has rect ref_prescribed_.
	// We need to know the actual rendered size of that node (or estimate it).
	// Then scale = actual_size / prescribed_size.
	// If no reference is given, scale = 1.

	double coord_scale = 1.0;  // source-coord → world-coord scale
	if(has_scale_ref_) {
		double prescribed_w = ref_prescribed_.Width();
		double prescribed_h = ref_prescribed_.Height();
		// Find the reference node to get actual size
		const NodeDoc* ref_node = graph.FindNode(ref_node_id_);
		double actual_w = (ref_node && ref_node->sz.cx > 0) ? ref_node->sz.cx : 200.0;
		double actual_h = (ref_node && ref_node->sz.cy > 0) ? ref_node->sz.cy : 150.0;
		// Use height as the more reliable dimension for node sizing
		double scale_w = actual_w / prescribed_w;
		double scale_h = actual_h / prescribed_h;
		coord_scale = max(scale_w, scale_h);
		LOG("ScriptedLayout: ref node=" << ref_node_id_
		    << " prescribed=" << prescribed_w << "x" << prescribed_h
		    << " actual=" << actual_w << "x" << actual_h
		    << " coord_scale=" << coord_scale);
	}

	// --- Step 2: For each group, auto-pack its nodes into local coords ---
	// Collect the packing results and compute the required scale per group.

	struct GroupPack : Moveable<GroupPack> {
		String vfs_path;
		Rectf  prescribed;   // from group_rects (source-space, unscaled)
		Rectf  node_tight;   // bounding box of auto-laid nodes (local coords)
	};
	Vector<GroupPack> packs;

	for(int gi = 0; gi < doc.groups.GetCount(); gi++) {
		const GroupDoc& grp = doc.groups[gi];
		int ri = group_rects.Find(grp.vfs_path);
		if(ri < 0) {
			LOG("ScriptedLayout: no prescribed rect for group " << grp.vfs_path << ", skipping");
			continue;
		}
		GroupPack gp;
		gp.vfs_path   = grp.vfs_path;
		gp.prescribed = group_rects[ri];

		// Pack nodes into local space (starting at (inner_padding_, inner_padding_))
		// temporarily — we'll re-offset them later
		Rectf tight = PackGroupNodes(graph, grp);
		gp.node_tight = tight;
		packs.Add(pick(gp));
	}

	// --- Step 3: Find the group needing the largest *scale-up* ---
	// For each group, the available internal area (in world coords) is:
	//   avail = prescribed.Size() * coord_scale - 2*inner_padding
	// The packed nodes occupy node_tight.Size().
	// group_scale = max(nodes_w / avail_w, nodes_h / avail_h)
	// We want all groups to use the same (worst-case) group_scale so the
	// layout proportions stay consistent.

	double worst_scale = 1.0;

	for(const GroupPack& gp : packs) {
		double avail_w = gp.prescribed.Width()  * coord_scale - 2 * inner_padding_;
		double avail_h = gp.prescribed.Height() * coord_scale - 2 * inner_padding_;
		if(avail_w <= 0) avail_w = 1;
		if(avail_h <= 0) avail_h = 1;

		double nw = gp.node_tight.Width();
		double nh = gp.node_tight.Height();
		if(nw <= 0) nw = 1;
		if(nh <= 0) nh = 1;

		double gscale = max(nw / avail_w, nh / avail_h);
		LOG("ScriptedLayout: group " << gp.vfs_path
		    << " avail=" << avail_w << "x" << avail_h
		    << " nodes=" << nw << "x" << nh
		    << " gscale=" << gscale);
		if(gscale > worst_scale) worst_scale = gscale;
	}
	LOG("ScriptedLayout: worst_scale=" << worst_scale
	    << " coord_scale=" << coord_scale);

	// --- Step 4: Apply uniform scale and position everything ---
	// World-space group origin = prescribed.TopLeft() * coord_scale
	// World-space group area   = prescribed.Size()    * coord_scale
	// Node positions within group are re-packed with the uniform group_scale
	// (i.e. the packed positions are multiplied by worst_scale so that the
	//  tightest group exactly fills its area; all looser groups scale identically,
	//  leaving whitespace).

	for(const GroupPack& gp : packs) {
		// World-space group top-left
		double gx = gp.prescribed.left  * coord_scale;
		double gy = gp.prescribed.top   * coord_scale;

		// Repack nodes at (gx + inner_padding + local_pos * worst_scale, gy + inner_padding + ...)
		const GroupDoc* grp = nullptr;
		for(const GroupDoc& g : doc.groups)
			if(g.vfs_path == gp.vfs_path) { grp = &g; break; }
		if(!grp) continue;

		// Gather nodes in order (reuse the same topo order by re-running PackGroupNodes
		// with a scale applied).  Instead of calling PackGroupNodes again we scale
		// the already-set node positions (they are currently in local coords starting from 0).
		for(const EntityId& nid : grp->nodes) {
			NodeDoc* n = graph.FindNode(nid);
			if(!n) continue;
			// n->pos currently holds the local position set by PackGroupNodes
			double local_x = n->pos.x;
			double local_y = n->pos.y;
			n->pos = Pointf(
				gx + inner_padding_ + local_x * worst_scale,
				gy + inner_padding_ + local_y * worst_scale
			);
			graph.Invalidate(n->id);
		}
	}

	// --- Step 5: Position standalone (ungrouped) nodes ---
	// Build set of grouped node ids for fast lookup
	Index<String> grouped;
	for(const GroupDoc& g : doc.groups)
		for(const EntityId& nid : g.nodes)
			grouped.FindAdd(nid);

	for(NodeDoc& n : graph.GetDoc().nodes) {
		if(grouped.Find(n.id) >= 0) continue;  // already positioned above
		int ri = group_rects.Find(n.id);
		if(ri < 0) continue;
		Rectf r = group_rects[ri];
		n.pos = Pointf(r.left * coord_scale, r.top * coord_scale);
		graph.Invalidate(n.id);
	}
}

} // namespace Node

} // namespace Upp
