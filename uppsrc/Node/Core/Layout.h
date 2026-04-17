#ifndef _Node_Core_Layout_h_
#define _Node_Core_Layout_h_

#include "Core.h"

namespace Upp {

namespace Node {

class Layout {
public:
	virtual ~Layout() {}
	virtual void Run(Graph& graph, Vector<NodeState>& states) = 0;
};

class SpringLayout : public Layout {
	int iterations = 500;
	double max_repulsive_dist = 500.0;
	double k = 100.0;
	double c = 0.1;
	double max_vertex_move = 10.0;

	uint64 seed = 0;

public:
	SpringLayout& Iterations(int n) { iterations = n; return *this; }
	SpringLayout& Seed(uint64 s)    { seed = s; return *this; }

	virtual void Run(Graph& graph, Vector<NodeState>& states) override;
};

// ---------------------------------------------------------------------------
// SmartPacker: Two-level packing for groups + ungrouped nodes
// ---------------------------------------------------------------------------

class SmartPacker {
public:
	// Layout orientation
	enum LayoutOrientation { LAYOUT_TALL, LAYOUT_WIDE, LAYOUT_WINDOW };

private:
	// Configuration
	double group_padding        = 30.0;  // Space around groups
	double node_padding         = 20.0;  // Space between nodes inside groups
	double group_inner_padding  = 25.0;  // Space from group edge to nodes
	double min_node_spacing     = 15.0;  // Minimum spacing between any nodes

	LayoutOrientation orientation = LAYOUT_TALL;  // LAYOUT_TALL = shelf packing (rows), LAYOUT_WIDE = column packing
	
	// Viewport for aspect ratio
	Rectf viewport;
	bool    has_viewport = false;
	
	// Connection graph: maps group/ungrouped-node ID to connection counts
	struct ConnectionInfo {
		String id;           // Group path or node ID
		bool   is_group;     // true = group, false = ungrouped node
		int    node_count;   // For groups: number of nodes inside
		Rectf  bounds;       // Estimated or actual bounds
		Vector<String> connection_ids;  // Connected item IDs
		Vector<int> connection_counts;  // Connection counts (parallel to connection_ids)
		
		ConnectionInfo() : is_group(false), node_count(0) {}
		ConnectionInfo(const ConnectionInfo&) = delete;
		ConnectionInfo& operator=(const ConnectionInfo&) = delete;
		ConnectionInfo(ConnectionInfo&& o) { *this = pick(o); }
		ConnectionInfo& operator=(ConnectionInfo&& o) {
			id = pick(o.id); is_group = o.is_group; node_count = o.node_count;
			bounds = o.bounds; connection_ids = pick(o.connection_ids);
			connection_counts = pick(o.connection_counts); return *this;
		}
	};
	
	Array<ConnectionInfo> items;  // All packable items (groups + ungrouped nodes)
	
	// Helpers
	void   AnalyzeGraph(Graph& graph);
	void   EstimateItemBounds(ConnectionInfo& item, Graph& graph);
	void   PackGlobal();
	void   PackNodesInGroupAtPosition(Graph& graph, const String& group_path, Pointf pos);
	void   AdjustAspectRatio();
	void   ValidateLayout(Graph& graph);
	Rectf  ComputeGroupBounds(Graph& graph, const GroupDoc& g);
	
	int    CountConnections(const String& id1, bool is_group1, const String& id2, bool is_group2, Graph& graph);
	Rectf  GetNodeBounds(const NodeDoc& n);
	
	// Helper to get total connection count for an item
	int    GetTotalConnectionCount(const ConnectionInfo& item) const;
	
public:
	SmartPacker() {}
	
	// Fluent interface
	SmartPacker& Viewport(Rectf r)  { viewport = r; has_viewport = true; return *this; }
	SmartPacker& GroupPadding(double d)   { group_padding = d; return *this; }
	SmartPacker& NodePadding(double d)    { node_padding = d; return *this; }
	SmartPacker& GroupInnerPadding(double d) { group_inner_padding = d; return *this; }
	SmartPacker& Orientation(LayoutOrientation o) { orientation = o; return *this; }
	
	// Main entry point
	void Pack(Graph& graph);
};

// ---------------------------------------------------------------------------
// ScriptedLayout: positions groups from prescribed coordinates,
// auto-packs nodes inside each group, and uniformly scales so the
// tightest group doesn't overflow its allotted area.
//
// Usage:
//   ScriptedLayout sl;
//   sl.SetGroupRect("/enc",  Rectf(1700, 100, 6000, 1700));
//   sl.SetGroupRect("/dec",  Rectf(6100, 100, 10700, 1700));
//   // … repeat for every group and top-level node …
//   sl.SetScaleRef("/enc/node80", Rectf(1750, 430, 2450, 1630)); // known node rect for calibration
//   sl.Run(graph);
// ---------------------------------------------------------------------------

class ScriptedLayout {
public:
	ScriptedLayout() {}

	// Register a prescribed bounding rect for a group (vfs_path, e.g. "/enc")
	// or for a standalone top-level node (node id).
	// Format: [x, y, x+width, y+height] in the source coordinate space.
	ScriptedLayout& SetGroupRect(const String& vfs_path_or_node_id, Rectf r)
	{
		group_rects.Add(vfs_path_or_node_id, r);
		return *this;
	}

	// Provide a single reference node whose prescribed rect is known.
	// This lets us compute how much the source coordinate space differs
	// from the actual rendered node size, so we can scale group areas
	// to guarantee the nodes fit.
	// ref_node_id: node id (e.g. "enc_node80")
	// prescribed_rect: the rect from the layout file for that node
	ScriptedLayout& SetScaleRef(const String& ref_node_id, Rectf prescribed_rect)
	{
		ref_node_id_     = ref_node_id;
		ref_prescribed_  = prescribed_rect;
		has_scale_ref_   = true;
		return *this;
	}

	// Padding around nodes inside a group
	ScriptedLayout& NodePadding(double d)  { node_padding_ = d; return *this; }
	// Padding between the group border and inner nodes
	ScriptedLayout& GroupInnerPadding(double d) { inner_padding_ = d; return *this; }

	// Main entry point
	void Run(Graph& graph);

private:
	VectorMap<String, Rectf> group_rects;  // prescribed rects (source-space)

	String ref_node_id_;
	Rectf  ref_prescribed_;
	bool   has_scale_ref_ = false;

	double node_padding_  = 20.0;
	double inner_padding_ = 25.0;

	// Auto-pack nodes inside a group using SA+GRASP; return bounding rect.
	// avail_w/avail_h: inner available area (already scaled to world coords).
	Rectf PackGroupNodes(Graph& graph, const GroupDoc& grp, double avail_w, double avail_h);
};

} // namespace Node

} // namespace Upp

#endif
