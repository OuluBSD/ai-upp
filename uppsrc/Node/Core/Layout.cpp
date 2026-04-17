#include "Layout.h"
#include "ForceLayout.h"
#include "Spiral.h"

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

void SmartPacker::PackNodesInGroupAtPosition(Graph& graph, const String& group_path,
                                              Pointf pos, bool use_spiral, bool use_circle)
{
	const GroupDoc* grp = nullptr;
	const GraphDoc& doc = graph.GetDoc();
	for(int i = 0; i < doc.groups.GetCount(); i++) {
		if(doc.groups[i].vfs_path == group_path) { grp = &doc.groups[i]; break; }
	}
	if(!grp) return;

	Vector<NodeDoc*> nodes;
	for(int i = 0; i < grp->nodes.GetCount(); i++) {
		NodeDoc* n = graph.FindNode(grp->nodes[i]);
		if(n) nodes.Add(n);
	}
	if(nodes.IsEmpty()) return;

	int N = nodes.GetCount();
	double spacing = node_padding * 12.0; // generous initial spacing for spiral/circle

	double start_x = pos.x + group_inner_padding;
	double start_y = pos.y + group_inner_padding;

	if(use_spiral || use_circle) {
		Vector<Pointf> pts = use_circle
		    ? CirclePositions(N, spacing)
		    : SpiralPositions(N, spacing);
		// Find bounding box of pts so we can offset to start_x/y
		double minx = pts[0].x, miny = pts[0].y;
		for(const Pointf& p : pts) { minx = min(minx, p.x); miny = min(miny, p.y); }
		for(int i = 0; i < N; i++) {
			nodes[i]->pos = Pointf(start_x + pts[i].x - minx, start_y + pts[i].y - miny);
			graph.Invalidate(nodes[i]->id);
		}
	} else {
		// Simple grid (used by TALL and WIDE)
		int cols  = max(1, (int)sqrt((double)N));
		double node_w = 200.0, node_h = 150.0;
		double col_w = node_w + node_padding * 3;
		double row_h = node_h + node_padding * 3;
		for(int i = 0; i < N; i++) {
			nodes[i]->pos = Pointf(start_x + (i % cols) * col_w,
			                      start_y + (i / cols) * row_h);
			graph.Invalidate(nodes[i]->id);
		}
	}
}

// ---------------------------------------------------------------------------
// ComputeCellSize: smallest-node-dim / 10 (mirrors PCB router formula)
// ---------------------------------------------------------------------------

double SmartPacker::ComputeCellSize(const Graph& graph)
{
	const GraphDoc& doc = graph.GetDoc();
	double min_dim = 1e18;
	for(const NodeDoc& n : doc.nodes) {
		if(n.sz.cx > 1) min_dim = min(min_dim, (double)n.sz.cx);
		if(n.sz.cy > 1) min_dim = min(min_dim, (double)n.sz.cy);
	}
	return (min_dim < 1e17) ? clamp(min_dim / 10.0, 1.0, 100.0) : 10.0;
}

// ---------------------------------------------------------------------------
// PackGlobalWindow: arrange items in a 2-D grid matching viewport AR
// ---------------------------------------------------------------------------

void SmartPacker::PackGlobalWindow()
{
	if(items.IsEmpty()) return;

	int total = items.GetCount();
	double target_ar = (has_viewport && viewport.Height() > 0)
	                   ? viewport.Width() / viewport.Height() : 1.5;

	// Find best column count: minimise |log(actual_ar / target_ar)|
	int best_cols = 1;
	double best_score = 1e300;
	for(int cols = 1; cols <= total; cols++) {
		int rows = (total + cols - 1) / cols;
		// Estimate bounding box for this grid
		double max_w = 0, max_h = 0;
		for(int i = 0; i < total; i++) {
			max_w = max(max_w, items[i].bounds.Width());
			max_h = max(max_h, items[i].bounds.Height());
		}
		double grid_w = cols * (max_w + group_padding);
		double grid_h = rows * (max_h + group_padding);
		if(grid_h <= 0) continue;
		double score = fabs(log(grid_w / grid_h / target_ar));
		if(score < best_score) { best_score = score; best_cols = cols; }
	}

	// Place items in the chosen grid
	double col_w = 0, row_h = 0;
	for(int i = 0; i < total; i++) {
		col_w = max(col_w, items[i].bounds.Width());
		row_h = max(row_h, items[i].bounds.Height());
	}
	col_w += group_padding;
	row_h += group_padding;

	double ox = group_padding, oy = group_padding;
	for(int i = 0; i < total; i++) {
		int col = i % best_cols, row = i / best_cols;
		items[i].bounds = Rectf(ox + col * col_w, oy + row * row_h,
		                        ox + col * col_w + items[i].bounds.Width(),
		                        oy + row * row_h + items[i].bounds.Height());
	}
}

// ---------------------------------------------------------------------------
// PackGlobalSpiral / PackGlobalCircle
// ---------------------------------------------------------------------------

void SmartPacker::PackGlobalSpiral(bool circle)
{
	if(items.IsEmpty()) return;

	// Spacing between group centres = largest group diagonal + group_padding
	double max_diag = 0;
	for(const ConnectionInfo& ci : items)
		max_diag = max(max_diag, sqrt(ci.bounds.Width()*ci.bounds.Width()
		                            + ci.bounds.Height()*ci.bounds.Height()));
	double spacing = max_diag + group_padding * 2;

	int N = items.GetCount();
	Vector<Pointf> pts = circle ? CirclePositions(N, spacing) : SpiralPositions(N, spacing);

	// Shift so top-left is at (group_padding, group_padding)
	double minx = pts[0].x, miny = pts[0].y;
	for(const Pointf& p : pts) { minx = min(minx, p.x); miny = min(miny, p.y); }

	for(int i = 0; i < N; i++) {
		double x = group_padding + pts[i].x - minx;
		double y = group_padding + pts[i].y - miny;
		double w = items[i].bounds.Width(), h = items[i].bounds.Height();
		items[i].bounds = Rectf(x - w*0.5, y - h*0.5, x + w*0.5, y + h*0.5);
	}
}

void SmartPacker::PackGlobal()
{
	if(items.IsEmpty()) return;

	if(orientation == LAYOUT_WINDOW) { PackGlobalWindow(); return; }
	if(orientation == LAYOUT_SPIRAL) { PackGlobalSpiral(false); return; }
	if(orientation == LAYOUT_CIRCLE) { PackGlobalSpiral(true);  return; }

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

	// Step 2: Global packing
	PackGlobal();

	// Step 3: Adjust aspect ratio (not needed for WINDOW/SPIRAL/CIRCLE — they already target AR)
	if(orientation == LAYOUT_TALL || orientation == LAYOUT_WIDE)
		AdjustAspectRatio();

	// Step 4: Pack nodes inside each group
	bool use_spiral = (orientation == LAYOUT_SPIRAL);
	bool use_circle = (orientation == LAYOUT_CIRCLE);
	for(int i = 0; i < items.GetCount(); i++) {
		const ConnectionInfo& item = items[i];
		if(item.is_group) {
			PackNodesInGroupAtPosition(graph, item.id, item.bounds.TopLeft(),
			                          use_spiral, use_circle);
		} else {
			NodeDoc* n = graph.FindNode(item.id);
			if(n)
				n->pos = Pointf(item.bounds.left, item.bounds.top);
		}
	}

	// Step 5: Force-directed refinement
	if(force_refine)
		ForceRefineGraph(graph);

	// Step 6: Validate layout
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

// Scene layout constants (must stay in sync with Scene.cpp)
static const double LAY_NODE_W      = 200.0;
static const double LAY_TITLE_H     = 26.0;
static const double LAY_PIN_ROW_H   = 22.0;
static const double LAY_SLOT_ROW_H  = 24.0;
static const double LAY_DOCEDIT_H   = 6 * LAY_SLOT_ROW_H;
static const double LAY_IMAGE_H     = 140.0;

// Estimate rendered node height from its doc (mirrors Scene.cpp::AddNodeItems).
static Sizef EstimateNodeSize(const NodeDoc& n)
{
	double w = n.sz.cx > 0 ? n.sz.cx : LAY_NODE_W;

	int in_count = 0, out_count = 0;
	for(const PinDoc& p : n.pins)
		(p.kind == PinKind::Output ? out_count : in_count)++;
	int pin_rows = max(in_count, out_count);

	double slot_h = 0;
	for(const WidgetSlotDoc& s : n.slots) {
		if(s.type == "Image" || s.type == "IMAGE")
			slot_h += LAY_IMAGE_H;
		else if(s.type == "DocEdit")
			slot_h += LAY_DOCEDIT_H;
		else
			slot_h += LAY_SLOT_ROW_H;
	}

	double h = LAY_TITLE_H + pin_rows * LAY_PIN_ROW_H + slot_h + 8.0;
	return Sizef(w, h);
}


// Compute grid sizes for each node: width/height per slot in the grid.
// Returns per-column widths and per-row heights.
static void ComputeGridBands(const Vector<NodeDoc*>& nodes, const Vector<int>& order,
                              int cols,
                              Vector<double>& col_w, Vector<double>& row_h)
{
	int N = nodes.GetCount();
	int rows = (N + cols - 1) / cols;
	col_w.SetCount(cols, 0.0);
	row_h.SetCount(rows, 0.0);
	for(int oi = 0; oi < N; oi++) {
		Sizef sz = EstimateNodeSize(*nodes[order[oi]]);
		col_w[oi % cols] = max(col_w[oi % cols], sz.cx);
		row_h[oi / cols] = max(row_h[oi / cols], sz.cy);
	}
}

// Tight (minimum) bounding size for a given column count.
static Sizef ComputeGridSize(const Vector<NodeDoc*>& nodes, const Vector<int>& order,
                              int cols, double padding)
{
	Vector<double> col_w, row_h;
	ComputeGridBands(nodes, order, cols, col_w, row_h);
	double tw = 0, th = 0;
	for(double v : col_w) tw += v + padding;
	for(double v : row_h) th += v + padding;
	if(tw > padding) tw -= padding;
	if(th > padding) th -= padding;
	return Sizef(tw, th);
}

// Apply a grid layout, distributing nodes to fill avail_w × avail_h.
// If avail_w/avail_h <= 0, falls back to tight packing.
static Rectf ApplyGridLayout(Graph& graph,
                              const Vector<NodeDoc*>& nodes, const Vector<int>& order,
                              int cols, double padding,
                              double avail_w = 0, double avail_h = 0)
{
	int N = nodes.GetCount();
	if(N == 0 || cols <= 0) return Rectf(0, 0, 0, 0);
	int rows = (N + cols - 1) / cols;

	Vector<double> col_w, row_h;
	ComputeGridBands(nodes, order, cols, col_w, row_h);

	// Compute tight totals
	double tight_w = 0, tight_h = 0;
	for(double v : col_w) tight_w += v;
	for(double v : row_h) tight_h += v;

	// Extra space to distribute as equal gaps between items
	// gap = (avail - tight) / (count + 1)  — includes leading + trailing gaps
	double gap_x = padding, gap_y = padding;
	if(avail_w > tight_w && cols > 0)
		gap_x = (avail_w - tight_w) / (cols + 1);
	if(avail_h > tight_h && rows > 0)
		gap_y = (avail_h - tight_h) / (rows + 1);

	// Build column/row offsets
	Vector<double> col_x(cols), row_y(rows);
	col_x[0] = gap_x;
	for(int c = 1; c < cols; c++) col_x[c] = col_x[c-1] + col_w[c-1] + gap_x;
	row_y[0] = gap_y;
	for(int r = 1; r < rows; r++) row_y[r] = row_y[r-1] + row_h[r-1] + gap_y;

	Rectf tight(0, 0, 0, 0);
	bool first = true;
	for(int oi = 0; oi < N; oi++) {
		int i   = order[oi];
		double x = col_x[oi % cols];
		double y = row_y[oi / cols];
		Sizef sz = EstimateNodeSize(*nodes[i]);

		nodes[i]->pos = Pointf(x, y);
		graph.Invalidate(nodes[i]->id);

		Rectf nr(x, y, x + sz.cx, y + sz.cy);
		if(first) { tight = nr; first = false; }
		else       tight.Union(nr);
	}
	return tight;
}

// ---------------------------------------------------------------------------
// SA (Simulated Annealing) layout with GRASP initialization
//
// State  : position of each node (Pointf array indexed [0..N-1])
// Energy : W_ov * total_overlap_area
//        + W_bd * total_out_of_bounds_area
//        + W_cn * sum of intra-group edge lengths
//
// GRASP init: topological grid layout (same as old grid search), provides
//             a structured starting point with data-flow order preserved.
// ---------------------------------------------------------------------------

static const double SA_W_OV = 1.0;    // overlap weight
static const double SA_W_BD = 2.0;    // boundary-penalty weight
static const double SA_W_CN = 0.005;  // connection-distance weight

// Compute axis-aligned overlap area between two rectangles (0 if no overlap).
static double OverlapArea(const Rectf& a, const Rectf& b)
{
	double dx = min(a.right, b.right)  - max(a.left, b.left);
	double dy = min(a.bottom, b.bottom) - max(a.top, b.top);
	return (dx > 0 && dy > 0) ? dx * dy : 0.0;
}

// Area of the portion of rect r that lies outside the boundary [0..bw] x [0..bh].
static double OutsideArea(const Rectf& r, double bw, double bh)
{
	double ix0 = max(r.left,   0.0),  ix1 = min(r.right,  bw);
	double iy0 = max(r.top,    0.0),  iy1 = min(r.bottom, bh);
	double inner = (ix1 > ix0 && iy1 > iy0) ? (ix1 - ix0) * (iy1 - iy0) : 0.0;
	return r.Width() * r.Height() - inner;
}

struct SaState {
	Vector<Pointf>  pos;    // position of node[i]
	Vector<Sizef>   sz;     // size of node[i] (constant)
	double          energy = 0;

	SaState Copy() const {
		SaState r;
		r.pos    = clone(pos);
		r.sz     = clone(sz);
		r.energy = energy;
		return r;
	}
};

static double ComputeEnergy(const SaState& s, double bw, double bh,
                             const Vector<int>& arc_from, const Vector<int>& arc_to)
{
	int N = s.pos.GetCount();
	double ov = 0, bd = 0, cn = 0;

	for(int i = 0; i < N; i++) {
		Rectf ri(s.pos[i].x, s.pos[i].y,
		         s.pos[i].x + s.sz[i].cx, s.pos[i].y + s.sz[i].cy);
		bd += OutsideArea(ri, bw, bh);
		for(int j = i + 1; j < N; j++) {
			Rectf rj(s.pos[j].x, s.pos[j].y,
			         s.pos[j].x + s.sz[j].cx, s.pos[j].y + s.sz[j].cy);
			ov += OverlapArea(ri, rj);
		}
	}
	for(int k = 0; k < arc_from.GetCount(); k++) {
		int fi = arc_from[k], ti = arc_to[k];
		double cx_f = s.pos[fi].x + s.sz[fi].cx * 0.5;
		double cy_f = s.pos[fi].y + s.sz[fi].cy * 0.5;
		double cx_t = s.pos[ti].x + s.sz[ti].cx * 0.5;
		double cy_t = s.pos[ti].y + s.sz[ti].cy * 0.5;
		double dx = cx_f - cx_t, dy = cy_f - cy_t;
		cn += sqrt(dx*dx + dy*dy);
	}
	return SA_W_OV * ov + SA_W_BD * bd + SA_W_CN * cn;
}

Rectf ScriptedLayout::PackGroupNodes(Graph& graph, const GroupDoc& grp,
                                      double avail_w, double avail_h)
{
	// Collect member nodes
	Vector<NodeDoc*> nodes;
	for(const EntityId& nid : grp.nodes) {
		NodeDoc* n = graph.FindNode(nid);
		if(n) nodes.Add(n);
	}
	int N = nodes.GetCount();
	if(N == 0) return Rectf(0, 0, 0, 0);

	// --- Topological order (data-flow) ---
	const GraphDoc& doc = graph.GetDoc();
	VectorMap<String, int> idx_map;
	for(int i = 0; i < N; i++)
		idx_map.Add(nodes[i]->id, i);

	struct Arc : Moveable<Arc> {
		int from, to;
		Arc(int f, int t) : from(f), to(t) {}
	};
	Vector<int> in_degree(N, 0);
	Vector<Arc> arcs;
	// Build arcs for edges within this group
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

	// --- Grid search for best column count (GRASP init) ---
	double target_ar = (avail_h > 0) ? avail_w / avail_h : 1.0;
	int best_cols = 1;
	{
		double best_score = 1e300;
		for(int cols = 1; cols <= N; cols++) {
			Sizef sz = ComputeGridSize(nodes, order, cols, node_padding_);
			if(sz.cy <= 0) continue;
			double ar = sz.cx / sz.cy;
			double score = fabs(log(ar / target_ar));
			if(score < best_score) { best_score = score; best_cols = cols; }
		}
	}

	// Ensure avail fits the tight grid
	{
		Sizef tight = ComputeGridSize(nodes, order, best_cols, node_padding_);
		avail_w = max(avail_w, tight.cx);
		avail_h = max(avail_h, tight.cy);
	}

	// GRASP: use grid layout as initial SA state
	ApplyGridLayout(graph, nodes, order, best_cols, node_padding_, avail_w, avail_h);

	// --- Build SA state ---
	SaState state;
	state.pos.SetCount(N);
	state.sz.SetCount(N);
	for(int i = 0; i < N; i++) {
		state.pos[i] = nodes[i]->pos;
		state.sz[i]  = EstimateNodeSize(*nodes[i]);
	}

	// Arc arrays for energy computation
	Vector<int> arc_from, arc_to;
	for(const Arc& a : arcs) { arc_from.Add(a.from); arc_to.Add(a.to); }

	state.energy = ComputeEnergy(state, avail_w, avail_h, arc_from, arc_to);

	// If already clean (no overlap, no oob), skip SA entirely
	if(state.energy < 1e-6) {
		Rectf bounds(0, 0, 0, 0);
		bool first = true;
		for(int i = 0; i < N; i++) {
			Rectf r(state.pos[i].x, state.pos[i].y,
			        state.pos[i].x + state.sz[i].cx, state.pos[i].y + state.sz[i].cy);
			if(first) { bounds = r; first = false; } else bounds.Union(r);
		}
		return bounds;
	}

	// --- Simulated Annealing ---
	// Seeded RNG from group path hash → deterministic per-group
	uint64 rng = 0xcbf29ce484222325ULL;
	for(char c : grp.vfs_path) rng = (rng ^ (unsigned char)c) * 0x100000001b3ULL;
	auto Rnd01 = [&]() -> double {
		rng ^= rng >> 33; rng *= 0xff51afd7ed558ccdULL;
		rng ^= rng >> 33; rng *= 0xc4ceb9fe1a85ec53ULL;
		rng ^= rng >> 33;
		return (rng >> 11) * (1.0 / (1ULL << 53));
	};
	auto RndInt = [&](int n) -> int { return (int)(Rnd01() * n) % n; };

	double T_start = max(avail_w, avail_h) * 0.5;  // large initial temperature
	double T_end   = node_padding_ * 0.01;
	int    iters   = max(2000, N * 200);
	double cool    = (T_end < T_start) ? pow(T_end / T_start, 1.0 / iters) : 0.995;
	double T = T_start;

	SaState best = state.Copy();

	for(int it = 0; it < iters; it++, T *= cool) {
		SaState trial = state.Copy();

		if(N == 1 || Rnd01() < 0.7) {
			// Move: translate one random node
			int ni = RndInt(N);
			double dx = (Rnd01() * 2 - 1) * T;
			double dy = (Rnd01() * 2 - 1) * T;
			trial.pos[ni].x = max(0.0, min(avail_w - trial.sz[ni].cx, state.pos[ni].x + dx));
			trial.pos[ni].y = max(0.0, min(avail_h - trial.sz[ni].cy, state.pos[ni].y + dy));
		} else {
			// Move: swap two random nodes' positions
			int a = RndInt(N), b = RndInt(N);
			while(b == a) b = RndInt(N);
			Swap(trial.pos[a], trial.pos[b]);
		}

		trial.energy = ComputeEnergy(trial, avail_w, avail_h, arc_from, arc_to);
		double dE = trial.energy - state.energy;
		if(dE < 0 || Rnd01() < exp(-dE / (T + 1e-12))) {
			state = pick(trial);
			if(state.energy < best.energy)
				best = state.Copy();
		}
	}

	// Write best positions back to nodes
	Rectf bounds(0, 0, 0, 0);
	bool first = true;
	for(int i = 0; i < N; i++) {
		nodes[i]->pos = best.pos[i];
		graph.Invalidate(nodes[i]->id);
		Rectf r(best.pos[i].x, best.pos[i].y,
		        best.pos[i].x + best.sz[i].cx, best.pos[i].y + best.sz[i].cy);
		if(first) { bounds = r; first = false; } else bounds.Union(r);
	}
	RLOG("SA group=" << grp.vfs_path << " N=" << N
	     << " energy_start=" << best.energy
	     << " iters=" << iters);
	return bounds;
}

void ScriptedLayout::Run(Graph& graph)
{
	const GraphDoc& doc = graph.GetDoc();

	// --- Compute coord_scale ---
	// We want the prescribed source coords scaled so that each group area
	// (in world pixels) is large enough to comfortably contain its nodes.
	//
	// For each group that has a prescribed rect, compute:
	//   group_scale = max( tight_node_w / (prescribed_w - 2*inner),
	//                      tight_node_h / (prescribed_h - 2*inner) )
	// where tight_node_* is the bounding size when all nodes are packed
	// tightly at their estimated sizes.
	//
	// We want world_size = prescribed_size * coord_scale >= tight_size * margin
	// => coord_scale >= tight_size * margin / prescribed_size  (per group)
	// Take the maximum over all groups so the most space-constrained group fits.

	// coord_scale < 1: compress source coords so nodes fill the groups sensibly.
	// coord_scale > 1: expand source coords if nodes need more space than prescribed.
	// We start at 0 (unset) and take the max of all per-group requirements.
	double coord_scale = 0.0;
	const double FIT_MARGIN = 1.6;  // 60% headroom so nodes aren't packed wall-to-wall

	for(int gi = 0; gi < doc.groups.GetCount(); gi++) {
		const GroupDoc& grp = doc.groups[gi];
		int ri = group_rects.Find(grp.vfs_path);
		if(ri < 0) continue;
		const Rectf& pr = group_rects[ri];

		// Collect nodes
		Vector<NodeDoc*> nodes;
		for(const EntityId& nid : grp.nodes) {
			NodeDoc* n = graph.FindNode(nid);
			if(n) nodes.Add(n);
		}
		if(nodes.IsEmpty()) continue;

		// Build a simple topological order for grid-size estimation
		int N2 = nodes.GetCount();
		VectorMap<String, int> idx2;
		for(int i = 0; i < N2; i++) idx2.Add(nodes[i]->id, i);
		Vector<int> in2(N2, 0);
		for(const EdgeDoc& e : doc.edges) {
			int fi = idx2.Find(e.source_node), ti = idx2.Find(e.target_node);
			if(fi >= 0 && ti >= 0 && fi != ti) in2[ti]++;
		}
		Vector<int> ord2;
		{
			Vector<int> q, dg = clone(in2);
			for(int i = 0; i < N2; i++) if(!dg[i]) q.Add(i);
			while(!q.IsEmpty()) {
				int c = q[0]; q.Remove(0); ord2.Add(c);
				for(const EdgeDoc& e : doc.edges) {
					int fi = idx2.Find(e.source_node), ti = idx2.Find(e.target_node);
					if(fi == c && ti >= 0 && fi != ti && --dg[ti] == 0) q.Add(ti);
				}
			}
			Vector<bool> vis(N2, false);
			for(int i : ord2) vis[i] = true;
			for(int i = 0; i < N2; i++) if(!vis[i]) ord2.Add(i);
		}

		// Best column count for this group's aspect ratio
		double pr_ar = (pr.Height() > 0) ? pr.Width() / pr.Height() : 1.0;
		int best_c = 1; double best_s = 1e300;
		for(int c=1; c<=N2; c++) {
			Sizef sz = ComputeGridSize(nodes, ord2, c, node_padding_);
			if(sz.cy <= 0) continue;
			double s = fabs(log(sz.cx/sz.cy / pr_ar));
			if(s < best_s) { best_s = s; best_c = c; }
		}
		Sizef tight = ComputeGridSize(nodes, ord2, best_c, node_padding_);
		tight.cx += 2 * inner_padding_;
		tight.cy += 2 * inner_padding_;

		// Scale needed so that prescribed_rect * scale >= tight * margin
		double pr_w = pr.Width(),  pr_h = pr.Height();
		if(pr_w > 0 && tight.cx > 0) {
			double s = (tight.cx * FIT_MARGIN) / pr_w;
			coord_scale = max(coord_scale, s);
		}
		if(pr_h > 0 && tight.cy > 0) {
			double s = (tight.cy * FIT_MARGIN) / pr_h;
			coord_scale = max(coord_scale, s);
		}
	}

	if(coord_scale <= 0.0) coord_scale = 1.0;  // fallback if no groups have rects
	RLOG("ScriptedLayout::Run coord_scale=" << coord_scale);

	// --- Step 1: Pack each group's nodes to fill its (scaled) prescribed area ---

	for(int gi = 0; gi < doc.groups.GetCount(); gi++) {
		const GroupDoc& grp = doc.groups[gi];
		int ri = group_rects.Find(grp.vfs_path);
		if(ri < 0) {
			RLOG("ScriptedLayout: no prescribed rect for group " << grp.vfs_path << ", skipping");
			continue;
		}
		// Scale the prescribed rect
		Rectf prescribed = group_rects[ri];
		prescribed.left   *= coord_scale;
		prescribed.top    *= coord_scale;
		prescribed.right  *= coord_scale;
		prescribed.bottom *= coord_scale;

		double aw = max(0.0, prescribed.Width()  - 2 * inner_padding_);
		double ah = max(0.0, prescribed.Height() - 2 * inner_padding_);

		// Pack nodes into local (0-based) coords via SA+GRASP
		PackGroupNodes(graph, grp, aw, ah);

		// Shift from local coords to the group's world-space origin
		double gx = prescribed.left + inner_padding_;
		double gy = prescribed.top  + inner_padding_;
		for(const EntityId& nid : grp.nodes) {
			NodeDoc* n = graph.FindNode(nid);
			if(!n) continue;
			n->pos.x += gx;
			n->pos.y += gy;
			graph.Invalidate(n->id);
		}
		RLOG("ScriptedLayout: group " << grp.vfs_path
		     << " scaled_rect=" << prescribed << " origin=(" << gx << "," << gy << ")");
	}

	// --- Step 2: Position standalone (ungrouped) nodes ---
	Index<String> grouped;
	for(const GroupDoc& g : doc.groups)
		for(const EntityId& nid : g.nodes)
			grouped.FindAdd(nid);

	for(NodeDoc& n : graph.GetDoc().nodes) {
		if(grouped.Find(n.id) >= 0) continue;
		int ri = group_rects.Find(n.id);
		if(ri < 0) continue;
		n.pos = Pointf(group_rects[ri].left  * coord_scale,
		               group_rects[ri].top   * coord_scale);
		graph.Invalidate(n.id);
	}

	// --- Step 3: Force-directed refinement ---
	if(force_refine_)
		ForceRefineGraph(graph);
}

} // namespace Node

} // namespace Upp
