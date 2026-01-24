#include "Renderer.h"

namespace GraphLib {

void Renderer::Paint(Draw& w) {
	w.DrawRect(sz, White());
	
	// Draw groups (behind nodes)
	for(int i = 0; i < graph.GetGroupCount(); i++) {
		const GroupNode& g = graph.GetGroup(i);
		Rect r = g.GetBoundingBox();
		Rect hr = g.GetHeaderRect();
		
		// Convert to screen coordinates
		// Note: This basic renderer assumes 1:1 mapping if not transformed.
		// GraphLayout handles the transformation logic in its Paint but here we draw relative to 0,0 
		// if we assume w is already transformed or we draw into an ImageDraw buffer of full size.
		// Given GraphLayout usage: rend.SetImageSize(id_sz); rend.Paint(*id);
		// So we draw in graph coordinates.
		
		w.DrawRect(r, g.body_clr);
		w.DrawRect(hr, g.header_clr);
		DrawFrame(w, r, g.border_clr);
		w.DrawText(hr.left + 5, hr.top + 2, g.label);
		
		if(g.isSelected)
			DrawFrame(w, r.Inflated(2), LtBlue());
	}
	
	// Draw edges
	for(int i = 0; i < graph.GetEdgeCount(); i++) {
		const Edge& e = graph.GetEdge(i);
		Point p1 = (Point)e.source->point + e.source->sz/2; // Simple center connection if no pins
		Point p2 = (Point)e.target->point + e.target->sz/2;
		
		// If pins are connected, use pin positions
		// This requires reverse lookup or Edge storing Pin pointers directly.
		// Current Edge struct only has Node* source/target.
		// However, Pins have 'connections' vector.
		// We can scan nodes to find pins connected to this edge.
		
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
		
		w.DrawLine(pin1, pin2, e.line_width, c);
		
		if(e.directed) {
			// Draw arrow
			// ...
		}
		
		if(!e.label.IsEmpty()) {
			Point mid = (pin1 + pin2) / 2;
			w.DrawText(mid.x, mid.y, e.label);
		}
	}
	
	// Draw nodes
	for(int i = 0; i < graph.GetNodeCount(); i++) {
		const Node& n = graph.GetNode(i);
		Rect r = n.GetBoundingBox();
		
		if(n.shape == 0) // Rect
			w.DrawRect(r, n.fill_clr);
		else if(n.shape == 1) // Circle
			w.DrawEllipse(r, n.fill_clr);
			
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
	for(int i = graph.GetNodeCount() - 1; i >= 0; i--) {
		Node& n = graph.GetNode(i);
		if(n.GetBoundingBox().Contains(pt))
			return &n;
	}
	return NULL;
}

Pin* Renderer::FindPin(Point pt) {
	for(int i = graph.GetNodeCount() - 1; i >= 0; i--) {
		Node& n = graph.GetNode(i);
		for(int j = 0; j < n.pins.GetCount(); j++) {
			Pin& p = n.pins[j];
			Point pp = (Point)n.point + (Point)p.position;
			Rect pr(pp.x - p.size.cx/2, pp.y - p.size.cy/2, p.size.cx, p.size.cy);
			// Add a small hit tolerance
			if(pr.Inflated(2).Contains(pt))
				return &p;
		}
	}
	return NULL;
}

GroupNode* Renderer::FindGroup(Point pt) {
	for(int i = graph.GetGroupCount() - 1; i >= 0; i--) {
		GroupNode& g = graph.GetGroup(i);
		if(g.GetBoundingBox().Contains(pt))
			return &g;
	}
	return NULL;
}

}
