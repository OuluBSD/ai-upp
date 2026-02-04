#ifndef _GraphLib_GraphNodeCtrl_h_
#define _GraphLib_GraphNodeCtrl_h_

namespace GraphLib {

class GraphNodeCtrl : public Ctrl {
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
    virtual void MouseWheel(Point p, int zdelta, dword key) override;

    void TimerProc();
    
    // Animation methods
    void StartZoomAnimation(double targetZoom, Point targetPan);
    void UpdateAnimation();
    void ZoomToTarget();
    
    // Focus methods
    void FocusOnSelection();
    void FocusOnNode(Node& node);
    
    // Animation control methods
    void StartEdgeFlowAnimation();
    void StartEdgeFlowAnimationForEdge(Edge& edge);

    // Persistence methods
    void SaveNodePositions(const String& filename);
    void LoadNodePositions(const String& filename);
    void SaveGraph(const String& filename);
    void LoadGraph(const String& filename);

    // Public methods for node/link management
    Node& AddNode(String id, Point position);
    Edge& AddEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId, double weight=1.0);
    void RemoveNode(String id);
    void RemoveEdge(Edge& edge);
    void RemoveEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId);

    // Public methods for group management
    GroupNode& AddGroup(String id, String label, Point position, Size size);
    void RemoveGroup(String id);
    void AddNodeToGroup(String nodeId, String groupId);
    void RemoveNodeFromGroup(String nodeId, String groupId);

    // Selection methods
    void SelectNode(Node* node);
    void SelectPin(Pin* pin);
    void SelectEdge(Edge* edge);
    void SelectGroup(GroupNode* group);
    void ClearSelection();

    // Helper methods
    void StartLinkCreation(Pin* pin);
    void EndLinkCreation(Pin* endPin);
    void CancelLinkCreation();

    // Box selection helper
    Rect GetBoxSelectionRect() const;

    // Getters
    Graph& GetGraph() { return static_cast<Graph&>(graphLayout); }
    const Graph& GetGraph() const { return static_cast<const Graph&>(graphLayout); }

    // Clipboard operations
    void CopyNodes();
    void PasteNodes(Point at);

private:
    GraphLayout<Spring> graphLayout;
    Renderer* renderer;
    bool isDragging;
    Point dragStart;
    Node* selectedNode;
    Pin* selectedPin;
    GroupNode* selectedGroup;
    Vector<Node*> selectedNodes;
    Vector<Edge*> selectedEdges;
    Vector<GroupNode*> selectedGroups;
    Node* dragNode;
    GroupNode* dragGroup;

    // Box selection state
    bool isBoxSelecting;
    Point boxSelectStart;
    Point boxSelectEnd;

    // Hover state
    Node* hoveredNode;
    Pin* hoveredPin;
    Edge* hoveredEdge;
    GroupNode* hoveredGroup;

    // Link creation state
    bool isCreatingLink;
    Pin* linkStartPin;

    // Navigation (zoom/pan) state
    double zoomFactor;
    Point panOffset;
    Point lastPanPoint;
    bool isPanning;
    
    // Animation variables
    bool isZoomingToTarget;
    double targetZoomFactor;
    Point targetPanOffset;
    Point startPanOffset;
    double startZoomFactor;
    int animationStep;
    int animationSteps;

    void UpdateLayout();
    void UpdateNodePositions();
    void DrawLinkPreview(Draw& w, Point start, Point end);

    // Clipboard data
    Vector<Node> clipboardNodes;
    Vector<Edge> clipboardEdges;
};

}

#endif
