#ifndef _Node_Core_Layout_h_
#define _Node_Core_Layout_h_

#include "Core.h"
#include "ForceLayout.h"
#include "Spiral.h"

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
	enum LayoutOrientation {
		LAYOUT_TALL,    // shelf packing (horizontal rows)
		LAYOUT_WIDE,    // column packing (vertical columns)
		LAYOUT_WINDOW,  // 2-D grid fitted to viewport aspect ratio
		LAYOUT_SPIRAL,  // Archimedean spiral group+node placement
		LAYOUT_CIRCLE,  // uniform circle group+node placement
	};

private:
	// Configuration
	double group_padding        = 30.0;
	double node_padding         = 20.0;
	double group_inner_padding  = 25.0;
	double min_node_spacing     = 15.0;

	LayoutOrientation orientation  = LAYOUT_TALL;
	bool              force_refine = true;   // run ForceRefine pass after placement

	// Viewport for aspect ratio
	Rectf viewport;
	bool  has_viewport = false;

	// Connection graph: maps group/ungrouped-node ID to connection counts
	struct ConnectionInfo {
		String id;
		bool   is_group;
		int    node_count;
		Rectf  bounds;
		Vector<String> connection_ids;
		Vector<int>    connection_counts;

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

	Array<ConnectionInfo> items;

	// Helpers
	void   AnalyzeGraph(Graph& graph);
	void   EstimateItemBounds(ConnectionInfo& item, Graph& graph);
	void   PackGlobal();
	void   PackGlobalWindow();                          // LAYOUT_WINDOW: 2-D grid
	void   PackGlobalSpiral(bool circle);               // LAYOUT_SPIRAL / LAYOUT_CIRCLE
	void   PackNodesInGroupAtPosition(Graph& graph, const String& group_path,
	                                  Pointf pos, bool use_spiral, bool use_circle);
	void   AdjustAspectRatio();
	void   ValidateLayout(Graph& graph);
	Rectf  ComputeGroupBounds(Graph& graph, const GroupDoc& g);

	int    CountConnections(const String& id1, bool is_group1,
	                        const String& id2, bool is_group2, Graph& graph);
	Rectf  GetNodeBounds(const NodeDoc& n);
	int    GetTotalConnectionCount(const ConnectionInfo& item) const;

	// Compute a cell_size equivalent from node boxes (matches PCB router formula)
	static double ComputeCellSize(const Graph& graph);

public:
	SmartPacker() {}

	// Fluent interface
	SmartPacker& Viewport(Rectf r)              { viewport = r; has_viewport = true; return *this; }
	SmartPacker& GroupPadding(double d)         { group_padding = d; return *this; }
	SmartPacker& NodePadding(double d)          { node_padding = d; return *this; }
	SmartPacker& GroupInnerPadding(double d)    { group_inner_padding = d; return *this; }
	SmartPacker& Orientation(LayoutOrientation o){ orientation = o; return *this; }
	SmartPacker& UseForceRefine(bool b)         { force_refine = b; return *this; }

	// Main entry point
	void Pack(Graph& graph);
};

// ---------------------------------------------------------------------------
// ScriptedLayout: positions groups from prescribed coordinates,
// auto-packs nodes inside each group via SA+GRASP, then optionally
// runs a ForceRefine pass.
// ---------------------------------------------------------------------------

class ScriptedLayout {
public:
	ScriptedLayout() {}

	ScriptedLayout& SetGroupRect(const String& vfs_path_or_node_id, Rectf r)
	{
		group_rects.Add(vfs_path_or_node_id, r);
		return *this;
	}

	ScriptedLayout& SetScaleRef(const String& ref_node_id, Rectf prescribed_rect)
	{
		ref_node_id_    = ref_node_id;
		ref_prescribed_ = prescribed_rect;
		has_scale_ref_  = true;
		return *this;
	}

	ScriptedLayout& NodePadding(double d)       { node_padding_  = d; return *this; }
	ScriptedLayout& GroupInnerPadding(double d) { inner_padding_ = d; return *this; }
	ScriptedLayout& UseForceRefine(bool b)      { force_refine_  = b; return *this; }

	void Run(Graph& graph);

private:
	VectorMap<String, Rectf> group_rects;

	String ref_node_id_;
	Rectf  ref_prescribed_;
	bool   has_scale_ref_ = false;

	double node_padding_  = 20.0;
	double inner_padding_ = 25.0;
	bool   force_refine_  = true;

	Rectf PackGroupNodes(Graph& graph, const GroupDoc& grp, double avail_w, double avail_h);
};

} // namespace Node

} // namespace Upp

#endif
