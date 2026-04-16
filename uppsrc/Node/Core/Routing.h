#ifndef _Node_Core_Routing_h_
#define _Node_Core_Routing_h_

#include "Core.h"

namespace Upp {

namespace Node {

enum class EdgeStyle {
	Simple,          // single cubic bezier on dominant axis (no avoidance)
	Curved,          // smooth bezier, routes shortest visible path around obstacles
	Schematic,       // right-angle orthogonal routing, avoids overlaps
	RealisticTight,  // catenary cable, slight sag (~18% slack)
	RealisticLoose,  // catenary cable, heavy sag (~50% slack)
	PCB,             // H/V/45° PCB traces, through-hole pads, via dots, dual-layer
};

struct RouteRequest {
	Pointf            source_pos;
	Pointf            target_pos;
	EdgeStyle         style = EdgeStyle::Curved;
	// Obstacle bounding boxes for avoidance (world space)
	Vector<Rectf>     obstacles;
	// Animation phase [0..1) for animated styles
	double            anim_phase = 0.0;
};

struct RouteResponse {
	Vector<Pointf> path;
	// PCB: each segment gets a layer tag (0=front/copper, 1=back/solder)
	// segment i connects path[i]→path[i+1] and is on layer seg_layer[i]
	Vector<int>    seg_layer;
	// PCB: indices in path[] that are via/bend points (draw via dot there)
	Vector<int>    via_indices;
};

class RoutingPolicy {
public:
	virtual ~RoutingPolicy() {}
	virtual RouteResponse Route(const RouteRequest& req) = 0;
};

// Routes all edges using the style specified per-request
class BezierRoutingPolicy : public RoutingPolicy {
public:
	virtual RouteResponse Route(const RouteRequest& req) override;
};

} // namespace Node

} // namespace Upp

#endif
