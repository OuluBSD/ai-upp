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
				cursor_y += h + group_padding * 8;
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
		double cursor_y = group_padding;
		double cursor_x = group_padding;
		
		// Use viewport height if available
		double available_height = has_viewport ? (viewport.Height() - 2 * group_padding) : 2000.0;

		for(int i = 0; i < items.GetCount(); i++) {
			ConnectionInfo& item = items[i];
			double w = item.bounds.Width();
			double h = item.bounds.Height();

			// Try to fit in current column
			bool fits_in_current = !columns.IsEmpty() && (cursor_y + h + group_padding <= available_height);

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

				item.bounds = Rectf(cursor_x, group_padding, cursor_x + w, group_padding + h);
				
				col.y = group_padding + h + group_padding;
				cursor_x += w + group_padding * 8;  // More horizontal spacing for WIDE

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

} // namespace Node

} // namespace Upp
