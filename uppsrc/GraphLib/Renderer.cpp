#include "GraphLib.h"

namespace GraphLib {

static void GetConnectionPoints(const Node& obj1, const Node& obj2, Vector<Point>& pts) {
	Rect bb1 = obj1.GetBoundingBox();
	Rect bb2 = obj2.GetBoundingBox();
	int bb1_width = bb1.GetWidth();
	int bb2_width = bb2.GetWidth();
	int bb1_height = bb1.GetHeight();
	int bb2_height = bb2.GetHeight();
	int bb1_x = bb1.left;
	int bb2_x = bb2.left;
	int bb1_y = bb1.top;
	int bb2_y = bb2.top;
	
	pts.SetCount(8);
	pts[0] = Point(bb1_x + bb1_width / 2, bb1_y);               // NORTH 1
	pts[1] = Point(bb1_x + bb1_width / 2, bb1_y + bb1_height);  // SOUTH 1
	pts[2] = Point(bb1_x, bb1_y + bb1_height / 2);              // WEST 1
	pts[3] = Point(bb1_x + bb1_width, bb1_y + bb1_height / 2);  // EAST 1
	pts[4] = Point(bb2_x + bb2_width / 2, bb2_y);               // NORTH 2
	pts[5] = Point(bb2_x + bb2_width / 2, bb2_y + bb2_height);  // SOUTH 2
	pts[6] = Point(bb2_x, bb2_y + bb2_height / 2);              // WEST 2
	pts[7] = Point(bb2_x + bb2_width, bb2_y + bb2_height / 2);  // EAST 2
}

static void DrawCurvedEdge(Draw& w, const Edge& edge, Color stroke) {
	Vector<Point> p;
	GetConnectionPoints(*edge.source, *edge.target, p);
	
	typedef Tuple<int, int> Comb;
	Vector<Comb> d;
	Vector<int> dis;
	
	for (int i = 0; i < 4; i++) {
		for (int j = 4; j < 8; j++) {
			int dx = abs(p[i].x - p[j].x);
			int dy = abs(p[i].y - p[j].y);
			if ((i == j - 4) || (((i != 3 && j != 6) || p[i].x < p[j].x) &&
			                     ((i != 2 && j != 7) || p[i].x > p[j].x) &&
			                     ((i != 0 && j != 5) || p[i].y > p[j].y) &&
			                     ((i != 1 && j != 4) || p[i].y < p[j].y))) {
				dis.Add((int)sqrt((double)(dx * dx + dy * dy)));
				Comb& c = d.Add();
				c.a = i;
				c.b = j;
			}
		}
	}
	
	Comb res;
	if (dis.IsEmpty()) {
		res.a = 0;
		res.b = 4;
	}
	else {
		int min_d = INT_MAX;
		int min_pos = 0;
		for (int i = 0; i < dis.GetCount(); i++) {
			if (dis[i] < min_d) {
				min_d = dis[i];
				min_pos = i;
			}
		}
		res = d[min_pos];
	}
	
	int x1 = p[res.a].x;
	int y1 = p[res.a].y;
	int x4 = p[res.b].x;
	int y4 = p[res.b].y;
	int dx = max(abs(x1 - x4) / 2, 10);
	int dy = max(abs(y1 - y4) / 2, 10);
	
	int x2 = x1, y2 = y1, x3 = x4, y3 = y4;
	switch (res.a) {
		case 0: x2 = x1;      y2 = y1 - dy; break;
		case 1: x2 = x1;      y2 = y1 + dy; break;
		case 2: x2 = x1 - dx; y2 = y1;      break;
		case 3: x2 = x1 + dx; y2 = y1;      break;
		default: break;
	}
	switch (res.b) {
		case 4: x3 = x4;      y3 = y1 + dy; break;
		case 5: x3 = x4;      y3 = y1 - dy; break;
		case 6: x3 = x4 - dx; y3 = y4;      break;
		case 7: x3 = x4 + dx; y3 = y4;      break;
		default: break;
	}
	
	Vector<Point> bezier_path;
	bezier_path.Reserve(24);
	for (double t = 0; t <= 1.01; t += 0.05) {
		double xa = x1 + ((x2 - x1) * t);
		double ya = y1 + ((y2 - y1) * t);
		double xb = x2 + ((x3 - x2) * t);
		double yb = y2 + ((y3 - y2) * t);
		double xc = x3 + ((x4 - x3) * t);
		double yc = y3 + ((y4 - y3) * t);
		double xd = xa + ((xb - xa) * t);
		double yd = ya + ((yb - ya) * t);
		double xe = xb + ((xc - xb) * t);
		double ye = yb + ((yc - yb) * t);
		double xf = xd + ((xe - xd) * t);
		double yf = yd + ((ye - yd) * t);
		bezier_path.Add(Point((int)xf, (int)yf));
	}
	w.DrawPolyline(bezier_path, edge.line_width, stroke);
	
	if (edge.directed) {
		double mag = sqrt((double)((y4 - y3) * (y4 - y3) + (x4 - x3) * (x4 - x3)));
		if (mag > 1e-6) {
			double nx = -(x4 - x3) * 10.0 / mag;
			double ny = -(y4 - y3) * 10.0 / mag;
			Vector<Point> arrow;
			arrow.SetCount(3);
			arrow[0].x = (int)(nx + ny + x4);
			arrow[0].y = (int)(ny + nx + y4);
			arrow[1].x = x4;
			arrow[1].y = y4;
			arrow[2].x = (int)(nx - ny + x4);
			arrow[2].y = (int)(ny - nx + y4);
			w.DrawPolyline(arrow, edge.line_width, stroke);
		}
	}
	
	if (!edge.label.IsEmpty()) {
		w.DrawText((x1 + x4) / 2, (y1 + y4) / 2, edge.label);
	}
}

Renderer::Renderer(Graph& graph) : graph(&graph), pt_sz(0, 0) {}

void Renderer::SetImageSize(Size s, int border) {
	sz = s;
	if (!graph || graph->GetNodeCount() <= 0)
		return;
	
	if (border < 0)
		border = (int)(min(sz.cx, sz.cy) * 0.2f);
	if (border < 0)
		border = 0;
	
	double min_x = DBL_MAX;
	double max_x = -DBL_MAX;
	double min_y = DBL_MAX;
	double max_y = -DBL_MAX;
	for(int i = 0; i < graph->GetNodeCount(); i++) {
		const Node& node = graph->GetNode(i);
		if (node.layout_pos_x < min_x) min_x = node.layout_pos_x;
		if (node.layout_pos_x > max_x) max_x = node.layout_pos_x;
		if (node.layout_pos_y < min_y) min_y = node.layout_pos_y;
		if (node.layout_pos_y > max_y) max_y = node.layout_pos_y;
	}
	
	double range_x = max_x - min_x;
	double range_y = max_y - min_y;
	if (fabs(range_x) < 1e-9) range_x = 1.0;
	if (fabs(range_y) < 1e-9) range_y = 1.0;
	
	double usable_x = max(1, sz.cx - border * 2);
	double usable_y = max(1, sz.cy - border * 2);
	
	for(int i = 0; i < graph->GetNodeCount(); i++) {
		Node& node = graph->GetNode(i);
		double nx = (node.layout_pos_x - min_x) / range_x;
		double ny = (node.layout_pos_y - min_y) / range_y;
		if (nx < 0.0) nx = 0.0;
		else if (nx > 1.0) nx = 1.0;
		if (ny < 0.0) ny = 0.0;
		else if (ny > 1.0) ny = 1.0;
		double avail_x = usable_x - node.sz.cx;
		double avail_y = usable_y - node.sz.cy;
		if (avail_x < 1.0) avail_x = 1.0;
		if (avail_y < 1.0) avail_y = 1.0;
		node.point.x = border + nx * avail_x;
		node.point.y = border + ny * avail_y;
	}
}

void Renderer::Paint(Draw& w) {
	w.DrawRect(sz, White());
	
	// Draw groups (behind nodes)
	for(int i = 0; i < graph->GetGroupCount(); i++) {
		const GroupNode& g = graph->GetGroup(i);
		Rect r = g.GetBoundingBox();
		Rect hr = g.GetHeaderRect();
		
		w.DrawRect(r, g.body_clr);
		w.DrawRect(hr, g.header_clr);
		DrawFrame(w, r, g.border_clr);
		w.DrawText(hr.left + 5, hr.top + 2, g.label);
		
		if(g.isSelected)
			DrawFrame(w, r.Inflated(2), LtBlue());
	}
	
		// Draw edges
		for(int i = 0; i < graph->GetEdgeCount(); i++) {
			const Edge& e = graph->GetEdge(i);
			Point p1 = (Point)e.source->point + e.source->sz/2;
			Point p2 = (Point)e.target->point + e.target->sz/2;
			
			Point pin1 = p1;
			Point pin2 = p2;
			bool found1 = false;
			bool found2 = false;
		
		for(int j=0; j<e.source->pins.GetCount(); j++) {
			const Pin& p = e.source->pins[j];
			for(int k=0; k<p.connections.GetCount(); k++)
				if(p.connections[k] == &e) {
					pin1 = (Point)e.source->point + (Point)p.position;
					found1 = true;
					break;
				}
			if(found1) break;
		}
		
		for(int j=0; j<e.target->pins.GetCount(); j++) {
			const Pin& p = e.target->pins[j];
			for(int k=0; k<p.connections.GetCount(); k++)
				if(p.connections[k] == &e) {
					pin2 = (Point)e.target->point + (Point)p.position;
					found2 = true;
					break;
				}
			if(found2) break;
			}
			
			Color c = e.stroke_clr;
			if(e.isSelected) c = LtBlue();
			
			// Pin-connected edges are drawn directly between pin anchors.
			// Regular node-to-node edges use curved routing like classic GraphLib.
			if (found1 || found2) {
				w.DrawLine(pin1, pin2, e.line_width, c);
				if(!e.label.IsEmpty()) {
					Point mid = (pin1 + pin2) / 2;
					w.DrawText(mid.x, mid.y, e.label);
				}
			}
			else {
				DrawCurvedEdge(w, e, c);
			}
		}
	
		// Draw nodes
		for(int i = 0; i < graph->GetNodeCount(); i++) {
			const Node& n = graph->GetNode(i);
			Rect r = n.GetBoundingBox();
			
			if(n.shape == Node::SHAPE_RECT)
				w.DrawRect(r, n.fill_clr);
			else if(n.shape == Node::SHAPE_ELLIPSE)
				w.DrawEllipse(r, n.fill_clr);
			else if(n.shape == Node::SHAPE_DIAMOND) {
				Vector<Point> diam;
				diam.SetCount(5);
				diam[0] = Point(r.left + r.GetWidth() / 2, r.top);
				diam[1] = Point(r.right, r.top + r.GetHeight() / 2);
				diam[2] = Point(r.left + r.GetWidth() / 2, r.bottom);
				diam[3] = Point(r.left, r.top + r.GetHeight() / 2);
				diam[4] = diam[0];
				w.DrawPolygon(diam, n.fill_clr, n.line_width, n.line_clr);
			}
				
			if (n.shape != Node::SHAPE_DIAMOND)
				DrawFrame(w, r, n.line_clr);
		
		Size tsz = GetTextSize(n.label, StdFont());
		w.DrawText(r.left + (r.GetWidth() - tsz.cx)/2, r.top + (r.GetHeight() - tsz.cy)/2, n.label);
		
		if(n.isSelected)
			DrawFrame(w, r.Inflated(3), LtBlue());
			
		// Draw pins
		for(int j = 0; j < n.pins.GetCount(); j++) {
			const Pin& p = n.pins[j];
			Point pp = (Point)n.point + (Point)p.position;
			Rect pr(pp.x - p.size.cx/2, pp.y - p.size.cy/2, p.size.cx, p.size.cy);
			w.DrawRect(pr, p.color);
			DrawFrame(w, pr, Black());
		}
	}
}

Node* Renderer::FindNode(Point pt) {
	for(int i = graph->GetNodeCount() - 1; i >= 0; i--) {
		Node& n = graph->GetNode(i);
		if(n.GetBoundingBox().Contains(pt))
			return &n;
	}
	return NULL;
}

Pin* Renderer::FindPin(Point pt) {
	for(int i = graph->GetNodeCount() - 1; i >= 0; i--) {
		Node& n = graph->GetNode(i);
		for(int j = 0; j < n.pins.GetCount(); j++) {
			Pin& p = n.pins[j];
			Point pp = (Point)n.point + (Point)p.position;
			Rect pr(pp.x - p.size.cx/2, pp.y - p.size.cy/2, p.size.cx, p.size.cy);
			if(pr.Inflated(2).Contains(pt))
				return &p;
		}
	}
	return NULL;
}

GroupNode* Renderer::FindGroup(Point pt) {
	for(int i = graph->GetGroupCount() - 1; i >= 0; i--) {
		GroupNode& g = graph->GetGroup(i);
		if(g.GetBoundingBox().Contains(pt))
			return &g;
	}
	return NULL;
}

}
