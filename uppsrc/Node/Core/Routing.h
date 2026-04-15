#ifndef _Node_Core_Routing_h_
#define _Node_Core_Routing_h_

#include "Core.h"

namespace Upp {

namespace Node {

struct RouteRequest {
	Pointf source_pos;
	Pointf target_pos;
	// Future: source/target orientation, obstacles, etc.
};

struct RouteResponse {
	Vector<Pointf> path;
};

class RoutingPolicy {
public:
	virtual ~RoutingPolicy() {}
	virtual RouteResponse Route(const RouteRequest& req) = 0;
};

class BezierRoutingPolicy : public RoutingPolicy {
public:
	virtual RouteResponse Route(const RouteRequest& req) override;
};

} // namespace Node

} // namespace Upp

#endif
