#ifndef _Node_Core_Routing_h_
#define _Node_Core_Routing_h_

#include "Core.h"

namespace Upp {

namespace Node {

enum class EdgeStyle {
	Simple,          // single cubic bezier on dominant axis (no avoidance)
	Curved,          // smooth bezier, visibility-graph shortest path + rounded corners
	Schematic,       // right-angle orthogonal routing, avoids overlaps
	RealisticTight,  // catenary cable, slight sag (~18% slack)
	RealisticLoose,  // catenary cable, heavy sag (~50% slack)
	PCBHV,           // PCB Manhattan (H/V only), weighted BFS maze router
	PCB45,           // PCB H/V/45°, weighted BFS maze router (default)
	PCBDiag,         // PCB diagonal-only (45° preferred, H/V penalized)
};

struct RouteRequest {
	Pointf            source_pos;
	Pointf            target_pos;
	EdgeStyle         style = EdgeStyle::Curved;
	// Obstacle bounding boxes for avoidance (node boxes only, not groups)
	Vector<Rectf>     obstacles;
	// Animation phase [0..1) for animated styles
	double            anim_phase = 0.0;
	// Net identity: source_pin_id + "→" + target_pin_id.
	// Traces belonging to the same net (shared source or target port) are
	// not penalized for overlap — they may share the same physical track.
	String            net_id;
};

struct RouteResponse {
	Vector<Pointf> path;
	// PCB: per-segment layer tag (0=front/copper, 1=back/solder).
	// seg_layer[i] is the layer for segment path[i]→path[i+1].
	Vector<int>    seg_layer;
	// PCB: indices in path[] that are via/bend points (render via dot there)
	Vector<int>    via_indices;
};

class RoutingPolicy {
public:
	virtual ~RoutingPolicy() {}
	virtual RouteResponse Route(const RouteRequest& req) = 0;
};

// Forward-declare PcbGrid (defined in Routing.cpp)
struct PcbGrid;

// Routes all edges using the style specified per-request.
// For PCB styles, call BeginBatch() before routing all edges so the grid
// accumulates occupied cells across nets (prevents overlaps).
class BezierRoutingPolicy : public RoutingPolicy {
	One<PcbGrid> pcb_grid; // non-null only during a PCB routing batch
public:
	// Call before routing all edges in a scene build.
	// scene_bounds: world-space bounding box of all nodes.
	// node_boxes:   obstacle boxes that block grid cells (no groups).
	void BeginBatch(const Rectf& scene_bounds,
	                const Vector<Rectf>& node_boxes,
	                EdgeStyle style);

	virtual RouteResponse Route(const RouteRequest& req) override;

	// PCB 2nd-pass helpers: only meaningful during a PCB batch.
	// Count how many cells of req's path overlap with OTHER nets' traces.
	// Returns 0 for non-PCB styles.
	int  CountOverlapCells(const RouteResponse& resp, const String& net_id) const;
	// Erase the cells belonging to net_id from the grid so it can be re-routed.
	void UnmarkNet(const String& net_id);
};

} // namespace Node

} // namespace Upp

#endif
