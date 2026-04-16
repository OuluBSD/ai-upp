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
	// Configuration
	double group_padding        = 30.0;  // Space around groups
	double node_padding         = 20.0;  // Space between nodes inside groups
	double group_inner_padding  = 25.0;  // Space from group edge to nodes
	double min_node_spacing     = 15.0;  // Minimum spacing between any nodes
	
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
	
	// Main entry point
	void Pack(Graph& graph);
};

} // namespace Node

} // namespace Upp

#endif
