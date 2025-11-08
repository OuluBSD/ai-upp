#ifndef _GraphLib_Graph_h
#define _GraphLib_Graph_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

namespace GraphLib {

enum class PinKind {
    Input,
    Output
};

struct Edge;  // Forward declaration for Pin struct

struct Pin : Moveable<Pin> {
    String id;
    String label;
    PinKind kind;
    Pointf position;  // Position relative to node
    Color color;
    int type;         // For type matching during connections
    Vector<Edge*> connections;  // Edges connected to this pin
    Size size;
    bool isConnected;

    Pin() : kind(PinKind::Input), type(0), size(8, 8), isConnected(false) {}
    Pin(String id, PinKind kind, int type = 0) : id(id), kind(kind), type(type), size(8, 8), isConnected(false) {}

    // Define a custom copy constructor that doesn't copy connections
    Pin(const Pin& other) : id(other.id), label(other.label), kind(other.kind), 
                            position(other.position), color(other.color), type(other.type),
                            size(other.size), isConnected(other.isConnected) {
        // Don't copy connections - they need to be re-established after node copying
    }

    // Define assignment operator that doesn't copy connections
    Pin& operator=(const Pin& other) {
        if (this != &other) {
            id = other.id;
            label = other.label;
            kind = other.kind;
            position = other.position;
            color = other.color;
            type = other.type;
            size = other.size;
            isConnected = other.isConnected;
            // Don't copy connections - they need to be re-established after node copying
        }
        return *this;
    }

    Rect GetBounds() const { 
        return RectC(position.x - size.cx/2, position.y - size.cy/2, size.cx, size.cy); 
    }
};

struct Node;

struct Edge : Moveable<Edge> {

	Node *source, *target;
	Color stroke_clr;
	String label;
	double weight;
	int line_width;
	bool attraction;
	bool directed;
	bool isSelected;  // Added for selection system

	Edge() : source(NULL), target(NULL), attraction(false), weight(1), directed(false), isSelected(false) {
		stroke_clr = Black();
		line_width = 1;
		weight = 1;
	}

	Edge& SetDirected(bool b=true) {directed = b; return *this;}
	Edge& SetLabel(String lbl) {label = lbl; return *this;}
	Edge& SetStroke(int line_width, Color clr) {this->line_width = line_width; stroke_clr = clr; return *this;}
	Edge& SetWeight(double d) {weight = d; return *this;}

	// Selection
	Edge& Select() {isSelected = true; return *this;}
	Edge& Deselect() {isSelected = false; return *this;}

	double GetWeight() const {return weight;}

};

inline Size MinFactor(Size sz, int w, int h) {
	sz *= 2.2;
	double factor = (double)sz.cx / sz.cy;
	double target = (double)w / h;
	if (factor < target)
		sz.cx = sz.cy * target;
	return sz;
}

struct Node : Moveable<Node> {
	Node* predecessor;
	String id, label;
	Color line_clr, fill_clr;
	Pointf point;
	double layout_pos_x, layout_pos_y;
	double layout_force_x, layout_force_y;
	double distance;
	int line_width;
	int shape;
	int sort_importance;
	Size sz;
	bool optimized;
	bool isSelected;  // Added for selection system
	Vector<Edge*> edges;
	Vector<Pin> pins;  // Pins for this node

	enum {SHAPE_ELLIPSE, SHAPE_RECT, SHAPE_DIAMOND};

	Node();
	Node(const Node& src);

	Rect GetBoundingBox() const {return RectC(point.x - sz.cx/2, point.y - sz.cy/2, sz.cx, sz.cy);}

	Node& SetSize(Size sz) {this->sz = sz; return *this;}
	Node& SetLabel(String s);
	Node& SetShapeEllipse() {shape = SHAPE_ELLIPSE; return *this;}
	Node& SetShapeRectangle() {shape = SHAPE_RECT; return *this;}
	Node& SetShapeDiamond() {shape = SHAPE_DIAMOND; return *this;}
	Node& SetStroke(int line_width, Color clr) {this->line_width = line_width; line_clr = clr; return *this;}
	Node& SetFill(Color clr) {fill_clr = clr; return *this;}
	
	// Selection
	Node& Select() {isSelected = true; return *this;}
	Node& Deselect() {isSelected = false; return *this;}
	
	// Pin management
	Node& AddPin(String pinId, PinKind kind, int type = 0) { 
		pins.Add().id = pinId; 
		pins.Top().kind = kind; 
		pins.Top().type = type; 
		return *this; 
	}
	Node& SetPinPosition(String pinId, Pointf pos) { 
		for(int i = 0; i < pins.GetCount(); i++) {
			if(pins[i].id == pinId) {
				pins[i].position = pos;
				break;
			}
		}
		return *this; 
	}
	Pin* FindPin(String pinId) { 
		for(int i = 0; i < pins.GetCount(); i++) {
			if(pins[i].id == pinId) {
				return &pins[i];
			}
		}
		return nullptr; 
	}
	const Pin* FindPin(String pinId) const { 
		for(int i = 0; i < pins.GetCount(); i++) {
			if(pins[i].id == pinId) {
				return &pins[i];
			}
		}
		return nullptr; 
	}
};


// Graph Data Structure
class Graph {

protected:
	friend class Renderer;
	friend class Layout;
	friend class TournamentTree;
	friend class TopologicalSort;
	friend class Spring;
	friend class TournamentTree;
	friend class OrderedTree;
	friend void Dijkstra(Graph& g, Node& source);

	ArrayMap<String, Node> nodes;
	Array<Edge> edges;
	Color fill_clr, border_clr, line_clr;
	double layout_max_x, layout_min_x;
	double layout_max_y, layout_min_y;
	int node_line_width, edge_line_width;

public:
	Graph();


	Graph& SetFillColor(Color clr) {fill_clr = clr; return *this;}
	Graph& SetNodeStroke(int line_width, Color clr) {node_line_width = line_width; border_clr = clr; return *this;}
	Graph& SetEdgeStroke(int line_width, Color clr) {edge_line_width = line_width; line_clr = clr; return *this;}

	int GetEdgeCount() const {return edges.GetCount();}
	int GetNodeCount() const {return nodes.GetCount();}
	Edge& GetEdge(int i) {return edges[i];}
	Node& GetNode(int i) {return nodes[i];}

	//    Add node if it doesn't exist yet.
	//
	//    This method does not update an existing node.
	//    If you want to update a node, just update the node object.
	//
	//    @param {string|number|object} id or node data
	//    @param {object|} node_data (optional)
	//    @returns {Node} the new or existing node
	Node& AddNode(String id, Node* copy_data_from=NULL);

	//    @param {string|number|object} source node or ID
	//    @param {string|number|object} target node or ID
	//    @param {object|} (optional) edge data, e.g. styles
	//    @returns {Edge}

	Edge& AddEdge(int source, int target, double weight=1.0, Edge* copy_data_from=NULL);
	Edge& AddEdge(String source, String target, double weight=1.0, Edge* copy_data_from=NULL);
	Edge& AddEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId, double weight=1.0, Edge* copy_data_from=NULL);

	//    @param {string|number|Node} node node or ID
	//    @return {Node}
	void RemoveNode(String id);

	//    Remove an edge by providing either two nodes (or ids) or the edge instance
	//    @param {string|number|Node|Edge} node edge, node or ID
	//    @param {string|number|Node} node node or ID
	//    @return {Edge}
	void RemoveEdge(Edge& source_edge);

	// Public access methods for nodes
	int FindNode(String id) const { return nodes.Find(id); }
	Node& GetNode(String id) { return nodes.Get(id); }
	const Node& GetNode(String id) const { return nodes.Get(id); }


	void Clear();
};

}

#endif