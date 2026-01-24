#ifndef _GraphLib_Renderer_h_
#define _GraphLib_Renderer_h_

#include "Graph.h"

namespace GraphLib {

using namespace Upp;

class Renderer {
	Vector<Point> bezier_path, diam, arrow;
	
	Graph* graph;
	
	Size pt_sz;
	Size sz;
	
	void DrawArrow(Draw& w, Point p1, Point p2, double scale);
	
public:
	Renderer(Graph& graph);
	
	Node* FindNode(Point pt);
	Pin* FindPin(Point pt);
	GroupNode* FindGroup(Point pt);
	
	void SetImageSize(Size sz, int border=-1);
	
	void Paint(Draw& w);
};

}

#endif
