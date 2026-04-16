#ifndef _Node_Core_Routing_h_
#define _Node_Core_Routing_h_

#include "Core.h"

namespace Upp {

namespace Node {

enum class EdgeStyle {
	Simple,    // current: single cubic bezier, control points on dominant axis
	Curved,    // improved: S-curve with right-exit/left-entry + vertical jog, avoids boxes
	Schematic, // right-angle orthogonal routing (like electronic schematics)
	Realistic, // catenary cable sag, animated droop
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
