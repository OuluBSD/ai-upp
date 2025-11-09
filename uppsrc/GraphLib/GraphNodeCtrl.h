#ifndef _GraphLib_GraphNodeCtrl_h_
#define _GraphLib_GraphNodeCtrl_h_

#include "GraphLib.h"

namespace GraphLib {

// Forward declaration
struct Pin;

class GraphNodeCtrl : public Ctrl {
private:
    GraphLayout<Spring> graphLayout;
    Renderer* renderer;
    bool isDragging;
    Point dragStart;
    Node* selectedNode;
    Pin* selectedPin;
    Vector<Node*> selectedNodes;
    Vector<Edge*> selectedEdges;
    Node* dragNode; // Node being dragged
    
    // Link creation state
    bool isCreatingLink;
    Pin* linkStartPin;
    
public:
    typedef GraphNodeCtrl CLASSNAME;
    
    GraphNodeCtrl();
    virtual ~GraphNodeCtrl();

    virtual void Paint(Draw& w) override;
    virtual void LeftDown(Point p, dword key) override;
    virtual void LeftUp(Point p, dword key) override;
    virtual void MouseMove(Point p, dword key) override;
    virtual void RightDown(Point p, dword key) override;
    virtual bool Key(dword key, int count) override;
    
    // Public methods for node/link management
    Node& AddNode(String id, Point position);
    Edge& AddEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId, double weight=1.0);
    void RemoveNode(String id);
    void RemoveEdge(Edge& edge);
    void RemoveEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId);
    
    // Selection methods
    void SelectNode(Node* node);
    void SelectPin(Pin* pin);
    void SelectEdge(Edge* edge);
    void ClearSelection();
    
    // Helper methods
    void StartLinkCreation(Pin* pin);
    void EndLinkCreation(Pin* endPin);
    void CancelLinkCreation();
    
    // Getters
    Graph& GetGraph() { return static_cast<Graph&>(graphLayout); }
    const Graph& GetGraph() const { return static_cast<const Graph&>(graphLayout); }

    // Clipboard operations
    void CopyNodes();
    void PasteNodes(Point at);
    
private:
    void UpdateLayout();
    void UpdateNodePositions();
    void DrawLinkPreview(Draw& w, Point start, Point end);

    // Clipboard data
    Vector<Node> clipboardNodes;
    Vector<Edge> clipboardEdges;
};

}

#endif