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
        } 
        // If Shift key is pressed, add all connected nodes to selection
        else if (key & K_SHIFT) {
            // Select this node and all connected nodes
            ClearSelection();
            Vector<Node*> connectedNodes;
            connectedNodes.Add(clickedNode);
            
            // Add all directly connected nodes
            for (int i = 0; i < clickedNode->edges.GetCount(); i++) {
                Edge* edge = clickedNode->edges[i];
                if (edge->source != clickedNode) {
                    connectedNodes.Add(edge->source);
                } else {
                    connectedNodes.Add(edge->target);
                }
            }
            
            // Select all found nodes
            for (int i = 0; i < connectedNodes.GetCount(); i++) {
                connectedNodes[i]->Select();
                selectedNodes.Add(connectedNodes[i]);
            }
            selectedNode = clickedNode;
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
    ClearSelection();
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
    
    // Allow box selection when no other operation is happening
    if (!isDragging && !isCreatingLink) {
        // Check if we're holding down the shift key to start box selection
        if (key & K_SHIFT && !selectedNode) {
            // Implementation of box selection would go here
            // For now, we'll just return
        }
    }
}

bool GraphNodeCtrl::Key(dword key, int count) {
    // Handle common keyboard shortcuts
    if (key == (K_CTRL|'C')) {  // Copy
        CopyNodes();
        return true;
    } else if (key == (K_CTRL|'V')) {  // Paste
        PasteNodes(dragStart);  // Paste at the last clicked/dragged position
        return true;
    } else if (key == (K_CTRL|'A')) {  // Select all
        ClearSelection();
        for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
            Node& node = GetGraph().GetNode(i);
            node.Select();
            selectedNodes.Add(&node);
        }
        Refresh();
        return true;
    } else if (key == K_DELETE || key == K_BACKSPACE) {  // Delete selected
        // Remove all selected edges first
        Vector<Edge*> edgesToRemove;
        for (int i = 0; i < selectedEdges.GetCount(); i++) {
            edgesToRemove.Add(selectedEdges[i]);
        }
        for (int i = 0; i < edgesToRemove.GetCount(); i++) {
            GetGraph().RemoveEdge(*edgesToRemove[i]);
        }
        
        // Remove all selected nodes
        Vector<String> nodesToRemove;
        for (int i = 0; i < selectedNodes.GetCount(); i++) {
            nodesToRemove.Add(selectedNodes[i]->id);
        }
        for (int i = 0; i < nodesToRemove.GetCount(); i++) {
            GetGraph().RemoveNode(nodesToRemove[i]);
        }
        
        // Clear the selection
        selectedNodes.Clear();
        selectedEdges.Clear();
        selectedNode = nullptr;
        
        Refresh();
        return true;
    }
    
    return Ctrl::Key(key, count);
}

void GraphNodeCtrl::RightDown(Point p, dword key) {
    // Store the click position for context menu
    dragStart = p;
    
    // Check if we clicked on a pin
    Pin* clickedPin = renderer->FindPin(p);
    if (clickedPin) {
        // Show pin context menu
        MenuBar::Execute([=](Bar& b) {
            b.Add("Break Links", [=]() {
                // Break all links connected to this pin
                Node* pinNode = nullptr;
                
                // Find which node this pin belongs to
                for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
                    Node& node = GetGraph().GetNode(i);
                    for (int j = 0; j < node.pins.GetCount(); j++) {
                        if (&node.pins[j] == clickedPin) {
                            pinNode = &node;
                            break;
                        }
                    }
                    if (pinNode) break;
                }
                
                if (pinNode) {
                    // Remove all edges connected to this pin
                    Vector<Edge*> edgesToRemove;
                    for (int i = 0; i < clickedPin->connections.GetCount(); i++) {
                        edgesToRemove.Add(clickedPin->connections[i]);
                    }
                    
                    for (int i = 0; i < edgesToRemove.GetCount(); i++) {
                        GetGraph().RemoveEdge(*edgesToRemove[i]);
                    }
                }
                Refresh();
            });
        });
        return;
    }
    
    // Check if we clicked on a node
    Node* clickedNode = renderer->FindNode(p);
    if (clickedNode) {
        // Show node context menu
        MenuBar::Execute([=](Bar& b) {
            b.Add("Add Input Pin", [=]() {
                Node& node = *clickedNode;
                String newPinId = "in_" + IntStr(node.pins.GetCount());
                node.AddPin(newPinId, PinKind::Input);
                Refresh();
            });
            
            b.Add("Add Output Pin", [=]() {
                Node& node = *clickedNode;
                String newPinId = "out_" + IntStr(node.pins.GetCount());
                node.AddPin(newPinId, PinKind::Output);
                Refresh();
            });
            
            b.Add("Remove Node", [=]() {
                RemoveNode(clickedNode->id);
                ClearSelection();
                Refresh();
            });
        });
        return;
    }
    
    // Check if we clicked on an edge
    Edge* clickedEdge = nullptr;  // We'll implement edge detection in a more complex way in future
    // For now, we'll just check if any edge is selected
    if (selectedEdges.GetCount() > 0) {
        MenuBar::Execute([=](Bar& b) {
            b.Add("Remove Edge", [=]() {
                RemoveEdge(*selectedEdges[0]);
                selectedEdges.Clear();
                Refresh();
            });
            
            b.Add("Change Color", [=]() {
                // Change color of selected edge
                if (selectedEdges.GetCount() > 0) {
                    selectedEdges[0]->stroke_clr = RandomColor();
                    Refresh();
                }
            });
        });
        return;
    }
    
    // If no specific item was clicked, show background context menu
    MenuBar::Execute([=](Bar& b) {
        b.Add("Add Node", [=]() {
            String nodeId = "Node_" + IntStr(GetGraph().GetNodeCount());
            AddNode(nodeId, p);
            Refresh();
        });
        
        b.Add("Clear Selection", [=]() {
            ClearSelection();
            Refresh();
        });
        
        b.Add("Clear All", [=]() {
            ClearSelection();
            GetGraph().Clear();
            Refresh();
        });
    });
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

void GraphNodeCtrl::CopyNodes() {
    // Clear previous clipboard data
    clipboardNodes.Clear();
    clipboardEdges.Clear();
    
    // Copy selected nodes to clipboard
    for (int i = 0; i < selectedNodes.GetCount(); i++) {
        Node* node = selectedNodes[i];
        clipboardNodes.Add(*node);  // Copy the node
        
        // Copy edges connected to this node
        for (int j = 0; j < node->edges.GetCount(); j++) {
            Edge* edge = node->edges[j];
            
            // Only copy edges where both source and target are in the selection
            bool sourceSelected = false;
            bool targetSelected = false;
            
            for (int k = 0; k < selectedNodes.GetCount(); k++) {
                if (selectedNodes[k] == edge->source) sourceSelected = true;
                if (selectedNodes[k] == edge->target) targetSelected = true;
            }
            
            if (sourceSelected && targetSelected) {
                // Check if this edge is already added to avoid duplicates
                bool alreadyAdded = false;
                for (int k = 0; k < clipboardEdges.GetCount(); k++) {
                    if (clipboardEdges[k].source == edge->source && 
                        clipboardEdges[k].target == edge->target) {
                        alreadyAdded = true;
                        break;
                    }
                }
                
                if (!alreadyAdded) {
                    clipboardEdges.Add(*edge);  // Copy the edge
                }
            }
        }
    }
}

void GraphNodeCtrl::PasteNodes(Point at) {
    if (clipboardNodes.GetCount() == 0) return;

    // Clear current selection
    ClearSelection();

    // Calculate offset to paste at the new location
    Point offset = at;
    if (clipboardNodes.GetCount() > 0) {
        // Calculate the center of the copied nodes to determine offset
        double minX = clipboardNodes[0].point.x;
        double minY = clipboardNodes[0].point.y;
        double maxX = minX;
        double maxY = minY;

        for (int i = 0; i < clipboardNodes.GetCount(); i++) {
            Node& node = clipboardNodes[i];
            minX = min(minX, node.point.x);
            minY = min(minY, node.point.y);
            maxX = max(maxX, node.point.x);
            maxY = max(maxY, node.point.y);
        }
        
        Point center = Point(int((minX + maxX) / 2), int((minY + maxY) / 2));
        offset = at - center;
    }

    // Create mapping from old node IDs to new node IDs
    VectorMap<String, String> nodeIdMap;

    // Add nodes to the graph with new IDs
    Vector<Node*> newNodes;
    for (int i = 0; i < clipboardNodes.GetCount(); i++) {
        Node& oldNode = clipboardNodes[i];
        String newId = oldNode.id + "_copy";
        
        // Make sure the new ID is unique
        int suffix = 1;
        String originalNewId = newId;
        while (GetGraph().nodes.Find(newId) != -1) {
            newId = originalNewId + IntStr(suffix++);
        }
        
        Node& newNode = AddNode(newId, Point(int(oldNode.point.x + offset.x), int(oldNode.point.y + offset.y)));
        
        // Copy node properties
        newNode.line_clr = oldNode.line_clr;
        newNode.fill_clr = oldNode.fill_clr;
        newNode.line_width = oldNode.line_width;
        newNode.shape = oldNode.shape;
        newNode.sz = oldNode.sz;
        
        // Copy pins
        newNode.pins.Clear();
        for (int j = 0; j < oldNode.pins.GetCount(); j++) {
            Pin pin = oldNode.pins[j]; // This makes a copy
            pin.position = Point(int(pin.position.x + offset.x), int(pin.position.y + offset.y));
            newNode.pins.Add(pin);
        }
        
        newNodes.Add(&newNode);
        nodeIdMap.Add(oldNode.id, newId);
    }

    // Add edges to connect the new nodes
    for (int i = 0; i < clipboardEdges.GetCount(); i++) {
        Edge& oldEdge = clipboardEdges[i];
        
        // Find the new source and target node IDs
        String newSourceId, newTargetId;
        for (int j = 0; j < clipboardNodes.GetCount(); j++) {
            if (&clipboardNodes[j] == oldEdge.source) {
                newSourceId = nodeIdMap.Get(clipboardNodes[j].id);
            }
            if (&clipboardNodes[j] == oldEdge.target) {
                newTargetId = nodeIdMap.Get(clipboardNodes[j].id);
            }
        }
        
        if (!newSourceId.IsEmpty() && !newTargetId.IsEmpty()) {
            // Find pins that were connected in the original
            // We'll need to connect the same pin IDs between the new nodes
            Node& sourceNode = GetGraph().GetNode(GetGraph().nodes.Find(newSourceId));
            Node& targetNode = GetGraph().GetNode(GetGraph().nodes.Find(newTargetId));
            
            // For now, connect the first available pins of appropriate types
            // A more sophisticated implementation would track the specific pin connections
            if (sourceNode.pins.GetCount() > 0 && targetNode.pins.GetCount() > 0) {
                // Find output pin on source and input pin on target 
                String sourcePinId = "";
                String targetPinId = "";
                
                for (int j = 0; j < sourceNode.pins.GetCount(); j++) {
                    if (sourceNode.pins[j].kind == PinKind::Output) {
                        sourcePinId = sourceNode.pins[j].id;
                        break;
                    }
                }
                
                for (int j = 0; j < targetNode.pins.GetCount(); j++) {
                    if (targetNode.pins[j].kind == PinKind::Input) {
                        targetPinId = targetNode.pins[j].id;
                        break;
                    }
                }
                
                if (!sourcePinId.IsEmpty() && !targetPinId.IsEmpty()) {
                    AddEdge(newSourceId, sourcePinId, newTargetId, targetPinId, oldEdge.weight);
                }
            }
        }
    }

    // Select the new nodes
    for (int i = 0; i < newNodes.GetCount(); i++) {
        SelectNode(newNodes[i]);
    }

    Refresh();
}

void GraphNodeCtrl::UpdateLayout() {
    graphLayout.RefreshLayout();
}

void GraphNodeCtrl::UpdateNodePositions() {
    // This would be called when layout changes to update all node and pin positions
}

}