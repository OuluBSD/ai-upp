#include "Routing.h"

namespace Upp {

namespace Node {

RouteResponse BezierRoutingPolicy::Route(const RouteRequest& req)
{
	RouteResponse res;
	Pointf p1 = req.source_pos;
	Pointf p4 = req.target_pos;
	
	double dx = max(abs(p1.x - p4.x) / 2.0, 10.0);
	double dy = max(abs(p1.y - p4.y) / 2.0, 10.0);
	
	Pointf p2 = p1;
	Pointf p3 = p4;
	
	if(abs(p1.x - p4.x) > abs(p1.y - p4.y)) {
		p2.x += (p4.x > p1.x ? dx : -dx);
		p3.x -= (p4.x > p1.x ? dx : -dx);
	}
	else {
		p2.y += (p4.y > p1.y ? dy : -dy);
		p3.y -= (p4.y > p1.y ? dy : -dy);
	}
	
	res.path.Reserve(24);
	for (double t = 0; t <= 1.01; t += 0.05) {
		double xa = p1.x + ((p2.x - p1.x) * t);
		double ya = p1.y + ((p2.y - p1.y) * t);
		double xb = p2.x + ((p3.x - p2.x) * t);
		double yb = p2.y + ((p3.y - p2.y) * t);
		double xc = p3.x + ((p4.x - p3.x) * t);
		double yc = p3.y + ((p4.y - p3.y) * t);
		
		double xm = xa + ((xb - xa) * t);
		double ym = ya + ((yb - ya) * t);
		double xn = xb + ((xc - xb) * t);
		double yn = yb + ((yc - yb) * t);
		
		res.path.Add(Pointf(xm + ((xn - xm) * t), ym + ((yn - ym) * t)));
	}
	
	return res;
}

} // namespace Node

} // namespace Upp
