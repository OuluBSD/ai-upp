#ifndef _GraphLib_Graph_h_
#define _GraphLib_Graph_h_

#include <CtrlLib/CtrlLib.h>

namespace GraphLib {

using namespace Upp;

enum class PinKind { Input, Output };

struct Pin : Moveable<Pin> {
	String id;
	String label;
	PinKind kind;
	Pointf position;
	Color color;
	int type;
	Size size;
	Vector<struct Edge*> connections;
	
	Pin() : kind(PinKind::Input), color(Black()), type(0), size(10, 10) {}
};

struct Node : Moveable<Node> {
	String id;
	String label;
	Pointf point;
	
	double layout_pos_x, layout_pos_y;
	
	Color fill_clr, line_clr;
	int line_width;
	int shape; // 0:rect, 1:circle
	Size sz;
	
	Vector<Pin> pins;
	Vector<struct Edge*> edges;
	
	// Dijkstra
	Node* predecessor;
	double distance;
	
	bool isSelected;
	
	// Animation state
	Pointf targetPoint;
	bool isAnimating;
	
	Node() : layout_pos_x(0), layout_pos_y(0), 
		fill_clr(White()), line_clr(Black()), line_width(1), shape(0), sz(100, 50),
		predecessor(NULL), distance(1e300), isSelected(false), isAnimating(false) {}
		
	Node& SetLabel(const String& s) {label = s; return *this;}
	Node& SetFill(Color c) {fill_clr = c; return *this;}
	Node& SetStroke(int w, Color c) {line_width = w; line_clr = c; return *this;}
	
	Rect GetBoundingBox() const {
		return RectC((int)point.x, (int)point.y, sz.cx, sz.cy);
	}
	
	RectC GetScreenRect(Point offset, double zoom) const {
		return RectC((int)((point.x + offset.x) * zoom), (int)((point.y + offset.y) * zoom), 
					 (int)(sz.cx * zoom), (int)(sz.cy * zoom));
	}
	
	Point CenterPoint() const { return Point((int)point.x + sz.cx/2, (int)point.y + sz.cy/2); }
	
	Pin& AddPin(String id, PinKind kind, int type=0);
	Pin* FindPin(String id);
	
	void Select() { isSelected = true; }
	void Deselect() { isSelected = false; }
	
	void StartMovementAnimation(Pointf target) {
		targetPoint = target;
		isAnimating = true;
	}
	
	void UpdateMovementAnimation() {
		if (isAnimating) {
			double dx = targetPoint.x - point.x;
			double dy = targetPoint.y - point.y;
			double dist = sqrt(dx*dx + dy*dy);
			if (dist < 1.0) {
				point = targetPoint;
				isAnimating = false;
			} else {
				point.x += dx * 0.1;
				point.y += dy * 0.1;
			}
		}
	}
};

struct GroupNode : Moveable<GroupNode> {
	String id;
	String label;
	Pointf position;
	Size size;
	
	Color header_clr;
	Color body_clr;
	Color border_clr;
	int border_width;
	
	bool is_collapsed;
	bool isSelected;
	
	Vector<String> node_ids;
	
	GroupNode() : position(0,0), size(200, 150), 
		header_clr(LtBlue()), body_clr(White()), border_clr(Black()), border_width(1),
		is_collapsed(false), isSelected(false) {}
		
	Rect GetBoundingBox() const {
		return RectC((int)position.x, (int)position.y, size.cx, size.cy);
	}
	
	Rect GetHeaderRect() const {
		return RectC((int)position.x, (int)position.y, size.cx, 20); // Assuming 20px header
	}
	
	bool ContainsHeader(Point p) const {
		return GetHeaderRect().Contains(p);
	}
	
	void Select() { isSelected = true; }
	void Deselect() { isSelected = false; }
};

struct Edge : Moveable<Edge> {
	Node* source;
	Node* target;
	String label;
	double weight;
	int line_width;
	Color stroke_clr;
	bool directed;
	
	bool isSelected;
	
	// Animation state
	double flowOffset;
	bool isFlowAnimating;
	double flowSpeed;
	
	Edge() : weight(1.0), line_width(1), stroke_clr(Black()), directed(false), isSelected(false), 
		flowOffset(0), isFlowAnimating(false), flowSpeed(0) {}
	
	Edge& SetLabel(const String& s) {label = s; return *this;}
	Edge& SetWeight(double w) {weight = w; return *this;}
	Edge& SetStroke(int w, Color c) {line_width = w; stroke_clr = c; return *this;}
	Edge& SetDirected(bool b=true) {directed = b; return *this;}
	
	double GetWeight() const {return weight;}
	
	void Select() { isSelected = true; }
	void Deselect() { isSelected = false; }
	
	void StartFlowAnimation() { isFlowAnimating = true; }
	void StopFlowAnimation() { isFlowAnimating = false; }
	void SetFlowSpeed(double speed) { flowSpeed = speed; }
	
	void UpdateFlowAnimation() {
		if (isFlowAnimating) {
			flowOffset += flowSpeed;
			if (flowOffset > 1.0) flowOffset -= 1.0;
		}
	}
};

class Graph {
protected:
	Vector<Node> nodes;
	Vector<Edge> edges;
	Vector<GroupNode> groups;
	
public:
	Node& AddNode(const String& id);
	void RemoveNode(const String& id);
	int FindNode(const String& id);
	Node& GetNode(int i) { return nodes[i]; }
	const Node& GetNode(int i) const { return nodes[i]; }
	int GetNodeCount() const { return nodes.GetCount(); }
	
	Edge& AddEdge(const String& n1, const String& n2, double weight=1.0);
	Edge& AddEdge(const String& n1, const String& p1, const String& n2, const String& p2, double weight=1.0);
	void RemoveEdge(Edge& e);
	Edge& GetEdge(int i) { return edges[i]; }
	const Edge& GetEdge(int i) const { return edges[i]; }
	int GetEdgeCount() const { return edges.GetCount(); }
	
	GroupNode& AddGroup(const String& id);
	void RemoveGroup(const String& id);
	int FindGroup(const String& id);
	GroupNode& GetGroup(int i) { return groups[i]; }
	const GroupNode& GetGroup(int i) const { return groups[i]; }
	int GetGroupCount() const { return groups.GetCount(); }
	
	void MoveNodeToGroup(const String& nodeId, const String& groupId);
	
	void Clear();
};

}

#endif
