#include "Snap.h"

namespace Upp {

namespace Node {

SnapResponse SnapToGrid(const SnapRequest& req)
{
	SnapResponse res;
	res.snapped_point = req.point;
	
	if(req.grid_step > 0) {
		double sx = round(req.point.x / req.grid_step) * req.grid_step;
		if(abs(sx - req.point.x) <= req.tolerance) {
			res.snapped_point.x = sx;
			res.snapped_x = true;
		}
		
		double sy = round(req.point.y / req.grid_step) * req.grid_step;
		if(abs(sy - req.point.y) <= req.tolerance) {
			res.snapped_point.y = sy;
			res.snapped_y = true;
		}
	}
	
	return res;
}

Vector<Guide> FindGuides(const Graph& graph, const GuideRequest& req)
{
	Vector<Guide> res;
	const GraphDoc& doc = graph.GetDoc();
	
	double mx[3] = { req.moving_rect.left, req.moving_rect.CenterPoint().x, req.moving_rect.right };
	double my[3] = { req.moving_rect.top,  req.moving_rect.CenterPoint().y, req.moving_rect.bottom };
	
	for(const auto& n : doc.nodes) {
		if(req.ignore_ids.Find(n.id) >= 0) continue;
		
		Rectf r(n.pos, n.sz);
		double rx[3] = { r.left, r.CenterPoint().x, r.right };
		double ry[3] = { r.top,  r.CenterPoint().y, r.bottom };
		
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				if(abs(mx[i] - rx[j]) <= req.tolerance) {
					Guide& g = res.Add();
					g.orientation = Guide::VERTICAL;
					g.pos = rx[j];
					g.ref_entity = n.id;
				}
				if(abs(my[i] - ry[j]) <= req.tolerance) {
					Guide& g = res.Add();
					g.orientation = Guide::HORIZONTAL;
					g.pos = ry[j];
					g.ref_entity = n.id;
				}
			}
		}
	}
	
	return res;
}

} // namespace Node

} // namespace Upp
