#include "GraphNodeCtrl.h"

namespace GraphLib {

GraphNodeCtrl::GraphNodeCtrl() 
    : isDragging(false), selectedNode(nullptr), selectedPin(nullptr), 
      dragNode(nullptr), isCreatingLink(false), linkStartPin(nullptr) {
    // Add the graph layout to this control
    Add(graphLayout.SizePos());
    renderer = &graphLayout.GetRenderer();  // Get the renderer from the GraphLayout
}

GraphNodeCtrl::~GraphNodeCtrl() {
}

void GraphNodeCtrl::Paint(Draw& w) {
    // Call the parent paint method to draw the graph
    Ctrl::Paint(w);
    
    // Draw link preview if we're creating a link
    if (isCreatingLink && linkStartPin) {
        DrawLinkPreview(w, linkStartPin->position, Point(GetMousePos().x, GetMousePos().y));
    }
}

void GraphNodeCtrl::LeftDown(Point p, dword key) {
    // Check if an edge was clicked first (might be overlapping with other items)
    Edge* clickedEdge = nullptr;  // For now, we'll just check if we clicked near an edge
    // We'll implement a more sophisticated edge detection later if needed
    bool edgeClicked = false;
    
    // Check if a pin was clicked
    Pin* clickedPin = renderer->FindPin(p);
    if (clickedPin) {
        SelectPin(clickedPin);
        StartLinkCreation(clickedPin);
        Refresh();
        return;
    }
    
    // Check if a node was clicked
    Node* clickedNode = renderer->FindNode(p);
    if (clickedNode) {
        // If Control key is pressed, add to selection instead of clearing
        if (key & K_CTRL) {
            // Toggle selection
            bool alreadySelected = false;
            for (int i = 0; i < selectedNodes.GetCount(); i++) {
                if (selectedNodes[i] == clickedNode) {
                    // Deselect this node
                    clickedNode->Deselect();
                    selectedNodes.Remove(i);
                    if (selectedNode == clickedNode) selectedNode = nullptr;
                    alreadySelected = true;
                    break;
                }
            }
            if (!alreadySelected) {
                clickedNode->Select();
                selectedNodes.Add(clickedNode);
                selectedNode = clickedNode;
            }
        } else {
            SelectNode(clickedNode);
        }
        
        dragNode = clickedNode;
        isDragging = true;
        dragStart = p;
        Refresh();
        return;
    }
    
    // If nothing was clicked, clear selection
    if (!edgeClicked) {
        ClearSelection();
    }
    Refresh();
}

void GraphNodeCtrl::LeftUp(Point p, dword key) {
    if (isCreatingLink && linkStartPin) {
        // Try to find a pin at the current mouse position to complete the link
        Pin* endPin = renderer->FindPin(p);
        if (endPin && endPin != linkStartPin) {
            // Validate pin types (input vs output)
            if (linkStartPin->kind != endPin->kind) {
                // Find the nodes that own these pins
                Node* startNode = nullptr;
                Node* endNode = nullptr;
                
                // Find the start node by looking through all nodes
                for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
                    Node& node = GetGraph().GetNode(i);
                    for (int j = 0; j < node.pins.GetCount(); j++) {
                        if (&node.pins[j] == linkStartPin) {
                            startNode = &node;
                            break;
                        }
                    }
                    if (startNode) break;
                }
                
                // Find the end node by looking through all nodes
                for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
                    Node& node = GetGraph().GetNode(i);
                    for (int j = 0; j < node.pins.GetCount(); j++) {
                        if (&node.pins[j] == endPin) {
                            endNode = &node;
                            break;
                        }
                    }
                    if (endNode) break;
                }
                
                // Create the edge: output pin connects to input pin
                if (startNode && endNode) {
                    if (linkStartPin->kind == PinKind::Output && endPin->kind == PinKind::Input) {
                        AddEdge(startNode->id, linkStartPin->id, endNode->id, endPin->id);
                    } else if (linkStartPin->kind == PinKind::Input && endPin->kind == PinKind::Output) {
                        AddEdge(endNode->id, endPin->id, startNode->id, linkStartPin->id);
                    }
                }
            }
        }
        
        EndLinkCreation(endPin);
        Refresh();
        return;
    }
    
    isDragging = false;
    dragNode = nullptr;
}

void GraphNodeCtrl::MouseMove(Point p, dword key) {
    if (isDragging && dragNode) {
        // Calculate movement delta
        int dx = p.x - dragStart.x;
        int dy = p.y - dragStart.y;
        
        // Update node position
        dragNode->point.x += dx;
        dragNode->point.y += dy;
        
        // Update pin positions
        for (int i = 0; i < dragNode->pins.GetCount(); i++) {
            Pin& pin = dragNode->pins[i];
            pin.position.x += dx;
            pin.position.y += dy;
        }
        
        // Update drag start to current position for incremental movement
        dragStart = p;
        
        Refresh();
        return;
    }
    
    if (isCreatingLink && linkStartPin) {
        Refresh();  // Redraw to update link preview
        return;
    }
}

void GraphNodeCtrl::RightDown(Point p, dword key) {
    // If an edge is selected, remove it
    if (selectedEdges.GetCount() > 0) {
        // Remove the first selected edge (in a more complex implementation, 
        // you could remove all selected edges)
        RemoveEdge(*selectedEdges[0]);
        selectedEdges.Clear();
    }
    // If a node is selected, remove it
    else if (selectedNodes.GetCount() > 0) {
        RemoveNode(selectedNodes[0]->id);
        selectedNodes.Clear();
    }
    // If nothing is selected, just clear selection
    else {
        ClearSelection();
    }
    Refresh();
}

Node& GraphNodeCtrl::AddNode(String id, Point position) {
    Node& node = GetGraph().AddNode(id);
    node.layout_pos_x = position.x;
    node.layout_pos_y = position.y;
    node.point = Point(position.x, position.y);
    
    // Update renderer with new size information
    Size currentSize = GetSize();
    if (currentSize.cx > 0 && currentSize.cy > 0) {
        graphLayout.RefreshBuffer();
    }
    
    return node;
}

Edge& GraphNodeCtrl::AddEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId, double weight) {
    Edge& edge = GetGraph().AddEdge(sourceNodeId, sourcePinId, targetNodeId, targetPinId, weight);
    return edge;
}

void GraphNodeCtrl::RemoveNode(String id) {
    GetGraph().RemoveNode(id);
    Refresh();
}

void GraphNodeCtrl::RemoveEdge(Edge& edge) {
    GetGraph().RemoveEdge(edge);
    Refresh();
}

void GraphNodeCtrl::RemoveEdge(String sourceNodeId, String sourcePinId, String targetNodeId, String targetPinId) {
    Node& sourceNode = GetGraph().AddNode(sourceNodeId);
    Node& targetNode = GetGraph().AddNode(targetNodeId);
    
    // Find the specific edge that connects these specific pins
    for (int i = 0; i < GetGraph().GetEdgeCount(); i++) {
        Edge& edge = GetGraph().GetEdge(i);
        
        // Check if this edge connects the specified nodes
        if (edge.source == &sourceNode && edge.target == &targetNode) {
            // Check if this edge is connected to the specified pins
            bool sourcePinMatch = false;
            bool targetPinMatch = false;
            
            // Check if the source node's pins include this edge
            for (int j = 0; j < sourceNode.pins.GetCount(); j++) {
                Pin& pin = sourceNode.pins[j];
                if (pin.id == sourcePinId) {
                    // Check if this edge is in the pin's connections
                    for (int k = 0; k < pin.connections.GetCount(); k++) {
                        if (pin.connections[k] == &edge) {
                            sourcePinMatch = true;
                            break;
                        }
                    }
                    break;
                }
            }
            
            // Check if the target node's pins include this edge
            for (int j = 0; j < targetNode.pins.GetCount(); j++) {
                Pin& pin = targetNode.pins[j];
                if (pin.id == targetPinId) {
                    // Check if this edge is in the pin's connections
                    for (int k = 0; k < pin.connections.GetCount(); k++) {
                        if (pin.connections[k] == &edge) {
                            targetPinMatch = true;
                            break;
                        }
                    }
                    break;
                }
            }
            
            // If both pins match, remove this edge
            if (sourcePinMatch && targetPinMatch) {
                GetGraph().RemoveEdge(edge);
                break;
            }
        }
    }
    Refresh();
}

void GraphNodeCtrl::SelectNode(Node* node) {
    if (node) {
        ClearSelection();
        node->Select();
        selectedNode = node;
        selectedNodes.Clear();
        selectedNodes.Add(node);
    }
}

void GraphNodeCtrl::SelectPin(Pin* pin) {
    selectedPin = pin;
}

void GraphNodeCtrl::SelectEdge(Edge* edge) {
    if (edge) {
        ClearSelection();
        edge->Select();
        selectedEdges.Clear();
        selectedEdges.Add(edge);
    }
}

void GraphNodeCtrl::ClearSelection() {
    // Deselect all currently selected nodes
    for (int i = 0; i < selectedNodes.GetCount(); i++) {
        selectedNodes[i]->Deselect();
    }
    
    // Deselect all currently selected edges
    for (int i = 0; i < selectedEdges.GetCount(); i++) {
        selectedEdges[i]->Deselect();
    }
    
    selectedNode = nullptr;
    selectedPin = nullptr;
    selectedNodes.Clear();
    selectedEdges.Clear();
}

void GraphNodeCtrl::StartLinkCreation(Pin* pin) {
    isCreatingLink = true;
    linkStartPin = pin;
}

void GraphNodeCtrl::EndLinkCreation(Pin* endPin) {
    isCreatingLink = false;
    linkStartPin = nullptr;
}

void GraphNodeCtrl::CancelLinkCreation() {
    isCreatingLink = false;
    linkStartPin = nullptr;
}

void GraphNodeCtrl::DrawLinkPreview(Draw& w, Point start, Point end) {
    if (isCreatingLink && linkStartPin) {
        Vector<Point> bezier_path;
        #define GET_POINT(n1 , n2 , perc) (n1 + ( (n2 - n1) * perc ))
        
        int x1 = start.x;
        int y1 = start.y;
        int x4 = end.x;
        int y4 = end.y;
        
        // Calculate control points to make a curved line
        int dx = abs(x1 - x4);
        int dy = abs(y1 - y4);
        
        double x2, y2, x3, y3;
        if(start.x < end.x) {
            // Left to right - curve downward then upward
            x2 = x1 + dx * 0.3;  // First control point extends right
            y2 = y1;             // Same height as start
            x3 = x4 - dx * 0.3;  // Second control point extends left  
            y3 = y4;             // Same height as end
        } else {
            // Right to left - curve upward then downward
            x2 = x1 - dx * 0.3;  // First control point extends left
            y2 = y1;             // Same height as start
            x3 = x4 + dx * 0.3;  // Second control point extends right
            y3 = y4;             // Same height as end
        }
        
        // Create bezier curve for preview
        bezier_path.SetCount(0);
        for (double i = 0 ; i <= 1.01 ; i += 0.05) {
            // The Green Line
            double xa = GET_POINT(x1, x2, i);
            double ya = GET_POINT(y1, y2, i);
            double xb = GET_POINT(x2, x3, i);
            double yb = GET_POINT(y2, y3, i);
            double xc = GET_POINT(x3, x4, i);
            double yc = GET_POINT(y3, y4, i);
            
            double xd = GET_POINT(xa, xb, i);
            double yd = GET_POINT(ya, yb, i);
            double xe = GET_POINT(xb, xc, i);
            double ye = GET_POINT(yb, yc, i);
            
            double xf = GET_POINT(xd, xe, i);
            double yf = GET_POINT(yd, ye, i);
            
            bezier_path.Add(Point(xf,yf));
        }
        
        w.DrawPolyline(bezier_path, 1, GrayColor(128));
    }
}

void GraphNodeCtrl::UpdateLayout() {
    graphLayout.RefreshLayout();
}

void GraphNodeCtrl::UpdateNodePositions() {
    // This would be called when layout changes to update all node and pin positions
}

}