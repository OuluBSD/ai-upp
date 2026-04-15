#ifndef _Node_Core_Snap_h_
#define _Node_Core_Snap_h_

#include "Core.h"

namespace Upp {

namespace Node {

struct SnapRequest {
	Pointf point;
	double grid_step = 10.0;
	double tolerance = 5.0;
};

struct SnapResponse {
	Pointf snapped_point;
	bool   snapped_x = false;
	bool   snapped_y = false;
};

SnapResponse SnapToGrid(const SnapRequest& req);

struct Guide : Moveable<Guide> {
	enum Orientation { HORIZONTAL, VERTICAL };
	Orientation orientation;
	double      pos;
	EntityId    ref_entity;
};

struct GuideRequest {
	Rectf    moving_rect;
	double   tolerance = 5.0;
	// Entity IDs to ignore (e.g. the ones being moved)
	Index<EntityId> ignore_ids;
};

Vector<Guide> FindGuides(const Graph& graph, const GuideRequest& req);

} // namespace Node

} // namespace Upp

#endif
