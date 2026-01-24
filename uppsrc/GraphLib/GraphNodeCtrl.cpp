#include "GraphNodeCtrl.h"

namespace GraphLib {

GraphNodeCtrl::GraphNodeCtrl() 
    : isDragging(false), selectedNode(nullptr), selectedPin(nullptr), 
      selectedGroup(nullptr), dragNode(nullptr), dragGroup(nullptr), 
      isBoxSelecting(false), hoveredNode(nullptr), hoveredPin(nullptr), 
      hoveredEdge(nullptr), hoveredGroup(nullptr), isCreatingLink(false), 
      linkStartPin(nullptr), zoomFactor(1.0), panOffset(0, 0), lastPanPoint(0, 0), isPanning(false),
      isZoomingToTarget(false), targetZoomFactor(1.0), targetPanOffset(0, 0), startPanOffset(0, 0), 
      startZoomFactor(1.0), animationStep(0), animationSteps(30) {
    // Add the graph layout to this control
    Add(graphLayout.SizePos());
    renderer = &graphLayout.GetRenderer();  // Get the renderer from the GraphLayout
    
    // Start animation timer (60 FPS)
    PostCallback(THISFN(TimerProc)); // Schedule first call
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

    // Draw selection box if we're box selecting
    if (isBoxSelecting) {
        Rect boxRect = RectC(min(boxSelectStart.x, boxSelectEnd.x),
                             min(boxSelectStart.y, boxSelectEnd.y),
                             abs(boxSelectEnd.x - boxSelectStart.x),
                             abs(boxSelectEnd.y - boxSelectStart.y));
        w.DrawRect(boxRect, LtBlue());
        w.DrawRect(boxRect.left, boxRect.top, boxRect.Width(), boxRect.Height(), Blue());
    }
}

void GraphNodeCtrl::LeftDown(Point p, dword key) {
    // Check if Alt key is pressed to start panning
    if (key & K_ALT) {
        isPanning = true;
        lastPanPoint = p;
        Refresh();
        return;
    }

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

    // Check if a group was clicked (only in header area for dragging)
    GroupNode* clickedGroup = renderer->FindGroup(p);
    if (clickedGroup && clickedGroup->ContainsHeader(p)) {
        // If Control key is pressed, add to selection instead of clearing
        if (key & K_CTRL) {
            // Toggle selection
            bool alreadySelected = false;
            for (int i = 0; i < selectedGroups.GetCount(); i++) {
                if (selectedGroups[i] == clickedGroup) {
                    // Deselect this group
                    clickedGroup->Deselect();
                    selectedGroups.Remove(i);
                    if (selectedGroup == clickedGroup) selectedGroup = nullptr;
                    alreadySelected = true;
                    break;
                }
            }
            if (!alreadySelected) {
                clickedGroup->Select();
                selectedGroups.Add(clickedGroup);
                selectedGroup = clickedGroup;
            }
        } else {
            SelectGroup(clickedGroup);
        }

        dragGroup = clickedGroup;
        isDragging = true;
        dragStart = p;
        Refresh();
        return;
    }

    // If Shift key is pressed, start box selection
    if (key & K_SHIFT) {
        isBoxSelecting = true;
        boxSelectStart = p;
        boxSelectEnd = p;
        // Don't clear selection when starting box selection
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

    // Check if we were panning
    if (isPanning) {
        isPanning = false;
        Refresh();
        return;
    }

    // Handle box selection if in progress
    if (isBoxSelecting) {
        // Define the selection rectangle
        int left = min(boxSelectStart.x, boxSelectEnd.x);
        int top = min(boxSelectStart.y, boxSelectEnd.y);
        int right = max(boxSelectStart.x, boxSelectEnd.x);
        int bottom = max(boxSelectStart.y, boxSelectEnd.y);
        Rect selectionRect = RectC(left, top, right - left, bottom - top);

        // Select all nodes within the rectangle
        ClearSelection(); // Clear previous selection

        for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
            Node& node = GetGraph().GetNode(i);
            Rect nodeRect = node.GetBoundingBox();
            if (selectionRect.Intersects(nodeRect)) {
                // Add the node to selection
                node.Select();
                selectedNodes.Add(&node);
                if (selectedNode == nullptr) {
                    selectedNode = &node;
                }
            }
        }

        // Also select groups that intersect with the selection rectangle
        for (int i = 0; i < GetGraph().GetGroupCount(); i++) {
            GroupNode& group = GetGraph().GetGroup(i);
            Rect groupRect = group.GetBoundingBox();
            if (selectionRect.Intersects(groupRect)) {
                // Add the group to selection
                group.Select();
                selectedGroups.Add(&group);
                if (selectedGroup == nullptr) {
                    selectedGroup = &group;
                }
            }
        }

        // End box selection
        isBoxSelecting = false;
        Refresh();
    }
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

    if (isDragging && dragGroup) {
        // Calculate movement delta
        int dx = p.x - dragStart.x;
        int dy = p.y - dragStart.y;

        // Update group position
        dragGroup->position.x += dx;
        dragGroup->position.y += dy;

        // Update drag start to current position for incremental movement
        dragStart = p;

        Refresh();
        return;
    }

    if (isCreatingLink && linkStartPin) {
        Refresh();  // Redraw to update link preview
        return;
    }

    // Handle panning if active
    if (isPanning) {
        int dx = p.x - lastPanPoint.x;
        int dy = p.y - lastPanPoint.y;
        panOffset.x += dx;
        panOffset.y += dy;
        lastPanPoint = p;
        Refresh();
        return;
    }

    // Allow box selection when no other operation is happening
    if (!isDragging && !isCreatingLink && !isPanning) {
        // Check if we're holding down the shift key to start box selection
        if (key & K_SHIFT) {
            if (!isBoxSelecting) {
                isBoxSelecting = true;
                boxSelectStart = p;
                boxSelectEnd = p;
            } else {
                boxSelectEnd = p;
                Refresh();  // Redraw to update the selection box
            }
            return;
        }
    }

    // Handle hover detection when not dragging or creating links
    if (!isDragging && !isCreatingLink && !isBoxSelecting && !isPanning) {
        // Find what's under the cursor
        Node* nodeAtPos = renderer->FindNode(p);
        Pin* pinAtPos = renderer->FindPin(p);
        GroupNode* groupAtPos = renderer->FindGroup(p);

        // For edges, we'll need to find the edge that's close to the cursor
        Edge* edgeAtPos = nullptr;
        if (!nodeAtPos && !pinAtPos && !groupAtPos) { // Only check for edges if no other objects are under cursor
            // Find the closest edge to the mouse position
            double minDist = 1000000; // Large number as initial minimum distance
            for (int i = 0; i < GetGraph().GetEdgeCount(); i++) {
                Edge& edge = GetGraph().GetEdge(i);

                // Check pin positions for this edge
                Pin* startPin = nullptr;
                Pin* endPin = nullptr;

                // Find pins that this edge connects
                for (int j = 0; j < edge.source->pins.GetCount(); j++) {
                    Pin& pin = edge.source->pins[j];
                    for (int k = 0; k < pin.connections.GetCount(); k++) {
                        if (pin.connections[k] == &edge) {
                            startPin = &pin;
                            break;
                        }
                    }
                    if (startPin) break;
                }

                for (int j = 0; j < edge.target->pins.GetCount(); j++) {
                    Pin& pin = edge.target->pins[j];
                    for (int k = 0; k < pin.connections.GetCount(); k++) {
                        if (pin.connections[k] == &edge) {
                            endPin = &pin;
                            break;
                        }
                    }
                    if (endPin) break;
                }

                if (startPin && endPin) {
                    // Calculate distance from point to line segment
                    Point start = startPin->position;
                    Point end = endPin->position;

                    // Calculate distance from point p to line segment start-end
                    // Using point-line distance formula
                    double A = p.x - start.x;
                    double B = p.y - start.y;
                    double C = end.x - start.x;
                    double D = end.y - start.y;

                    double dot = A * C + B * D;
                    double lenSq = C * C + D * D;
                    double param = -1;
                    if (lenSq != 0) // in case of 0 length line
                        param = dot / lenSq;

                    double xx, yy;

                    if (param < 0) {
                        xx = start.x;
                        yy = start.y;
                    }
                    else if (param > 1) {
                        xx = end.x;
                        yy = end.y;
                    }
                    else {
                        xx = start.x + param * C;
                        yy = start.y + param * D;
                    }

                    double dx = p.x - xx;
                    double dy = p.y - yy;
                    double dist = sqrt(dx * dx + dy * dy);

                    if (dist < minDist) {
                        minDist = dist;
                        if (dist < 10) { // Threshold for considering an edge hovered
                            edgeAtPos = &edge;
                        }
                    }
                }
            }
        }

        // Update hover states only if they changed
        bool hoverChanged = false;
        if (hoveredNode != nodeAtPos) {
            hoveredNode = nodeAtPos;
            hoverChanged = true;
        }
        if (hoveredPin != pinAtPos) {
            hoveredPin = pinAtPos;
            hoverChanged = true;
        }
        if (hoveredEdge != edgeAtPos) {
            hoveredEdge = edgeAtPos;
            hoverChanged = true;
        }
        if (hoveredGroup != groupAtPos) {
            hoveredGroup = groupAtPos;
            hoverChanged = true;
        }

        // Refresh only if hover state changed to avoid excessive redraws
        if (hoverChanged) {
            Refresh();
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
    } else if (key == (K_CTRL|'F')) {  // Focus on selection
        FocusOnSelection();
        return true;
    } else if (key == (K_CTRL|'L')) {  // Start link flow animation
        StartEdgeFlowAnimation();
        return true;
    } else if (key == (K_CTRL|'S')) {  // Save graph
        SaveGraph("graph_data.xml");
        return true;
    } else if (key == (K_CTRL|K_SHIFT|'S')) {  // Save node positions only
        SaveNodePositions("node_positions.xml");
        return true;
    } else if (key == (K_CTRL|'O')) {  // Load graph
        LoadGraph("graph_data.xml");
        return true;
    } else if (key == (K_CTRL|K_SHIFT|'O')) {  // Load node positions only
        LoadNodePositions("node_positions.xml");
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

    // Check if a node was clicked
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

    // Check if a group was clicked
    GroupNode* clickedGroup = renderer->FindGroup(p);
    if (clickedGroup) {
        // Show group context menu
        MenuBar::Execute([=](Bar& b) {
            b.Add("Add Group", [=]() {
                String groupId = "Group_" + IntStr(GetGraph().GetGroupCount());
                AddGroup(groupId, "New Group", p, Size(300, 200));
                Refresh();
            });

            b.Add("Remove Group", [=]() {
                RemoveGroup(clickedGroup->id);
                ClearSelection();
                Refresh();
            });

            b.Add("Toggle Collapse", [=]() {
                clickedGroup->is_collapsed = !clickedGroup->is_collapsed;
                Refresh();
            });

            b.Add("Add Node to Group", [=]() {
                // If a node is selected, add it to the current group
                if (selectedNode) {
                    GetGraph().MoveNodeToGroup(selectedNode->id, clickedGroup->id);
                    Refresh();
                }
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

        b.Add("Add Group", [=]() {
            String groupId = "Group_" + IntStr(GetGraph().GetGroupCount());
            AddGroup(groupId, "New Group", p, Size(300, 200));
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

void GraphNodeCtrl::SelectGroup(GroupNode* group) {
    if (group) {
        ClearSelection();  // Clear all other selections
        group->Select();
        selectedGroup = group;
        selectedGroups.Clear();
        selectedGroups.Add(group);
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

    // Deselect all currently selected groups
    for (int i = 0; i < selectedGroups.GetCount(); i++) {
        selectedGroups[i]->Deselect();
    }

    selectedNode = nullptr;
    selectedPin = nullptr;
    selectedGroup = nullptr;
    selectedNodes.Clear();
    selectedEdges.Clear();
    selectedGroups.Clear();
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
        
        // Add a copy of the node to the clipboard
        Node& copy = clipboardNodes.Add();
        new (&copy) Node(*node, 1);  // Use the copy constructor

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
        while (GetGraph().FindNode(newId) != -1) {
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
            Pin pin(oldNode.pins[j], 1); // Use copy constructor
            pin.position = Point(int(pin.position.x + offset.x), int(pin.position.y + offset.y));
            Pin& newPin = newNode.pins.Add();
            new (&newPin) Pin(pin, 1);
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
            Node& sourceNode = GetGraph().GetNode(GetGraph().FindNode(newSourceId));
            Node& targetNode = GetGraph().GetNode(GetGraph().FindNode(newTargetId));

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

// Group management methods
GroupNode& GraphNodeCtrl::AddGroup(String id, String label, Point position, Size size) {
    GroupNode& group = GetGraph().AddGroup(id);
    group.id = id;
    group.label = label;
    group.position = Pointf(position.x, position.y);
    group.size = size;
    return group;
}

void GraphNodeCtrl::RemoveGroup(String id) {
    GetGraph().RemoveGroup(id);
    Refresh();
}

void GraphNodeCtrl::AddNodeToGroup(String nodeId, String groupId) {
    GetGraph().MoveNodeToGroup(nodeId, groupId);
    Refresh();
}

void GraphNodeCtrl::RemoveNodeFromGroup(String nodeId, String groupId) {
    // For now, just remove the node from the group by moving it to no group
    GetGraph().MoveNodeToGroup(nodeId, ""); // Move to no group
    Refresh();
}

// Box selection helper
Rect GraphNodeCtrl::GetBoxSelectionRect() const {
    int left = min(boxSelectStart.x, boxSelectEnd.x);
    int top = min(boxSelectStart.y, boxSelectEnd.y);
    int right = max(boxSelectStart.x, boxSelectEnd.x);
    int bottom = max(boxSelectStart.y, boxSelectEnd.y);
    return RectC(left, top, right - left, bottom - top);
}

void GraphNodeCtrl::MouseWheel(Point p, int zdelta, dword key) {
    if (key & K_CTRL) {
        // Zoom in/out centered on mouse position
        double zoomFactorDelta = zdelta > 0 ? 1.1 : 0.9;
        double oldZoomFactor = zoomFactor;
        double newZoomFactor = zoomFactor * zoomFactorDelta;
        
        // Limit zoom range
        newZoomFactor = max(0.1, min(5.0, newZoomFactor));
        
        // Calculate target pan offset to zoom around mouse position
        int targetPanX = p.x - newZoomFactor * ((p.x - panOffset.x) / oldZoomFactor);
        int targetPanY = p.y - newZoomFactor * ((p.y - panOffset.y) / oldZoomFactor);
        
        // Start smooth animation to target zoom and pan
        StartZoomAnimation(newZoomFactor, Point(targetPanX, targetPanY));
    } else {
        // Pan vertically if Ctrl is not pressed
        panOffset.y += zdelta * 10;
        Refresh();
    }
}

void GraphNodeCtrl::StartZoomAnimation(double targetZoom, Point targetPan) {
    if (isZoomingToTarget) {
        // If already animating, set new target but keep current start values
        targetZoomFactor = targetZoom;
        targetPanOffset = targetPan;
    } else {
        // Set up animation
        isZoomingToTarget = true;
        startZoomFactor = zoomFactor;
        startPanOffset = panOffset;
        targetZoomFactor = targetZoom;
        targetPanOffset = targetPan;
        animationStep = 0;
        
        // Continue animation (timer is already running in TimerProc)
    }
}

void GraphNodeCtrl::UpdateAnimation() {
    if (isZoomingToTarget) {
        animationStep++;
        double progress = (double)animationStep / animationSteps;
        
        if (progress >= 1.0) {
            // Animation finished
            zoomFactor = targetZoomFactor;
            panOffset = targetPanOffset;
            isZoomingToTarget = false;
        } else {
            // Interpolate to target
            zoomFactor = startZoomFactor + (targetZoomFactor - startZoomFactor) * progress;
            panOffset.x = startPanOffset.x + (targetPanOffset.x - startPanOffset.x) * progress;
            panOffset.y = startPanOffset.y + (targetPanOffset.y - startPanOffset.y) * progress;
            
            // Continue animation
            // Continue animation (timer is already running in TimerProc)
        }
        
        Refresh();
    }
}

void GraphNodeCtrl::TimerProc() {
    // Handle timer-based events (animations)
    UpdateAnimation();
    
    // Update link flow animations
    for(int i = 0; i < GetGraph().GetEdgeCount(); i++) {
        Edge& edge = GetGraph().GetEdge(i);
        edge.UpdateFlowAnimation();
    }
    
    // Update node movement animations
    for(int i = 0; i < GetGraph().GetNodeCount(); i++) {
        Node& node = GetGraph().GetNode(i);
        node.UpdateMovementAnimation();
    }
    
    Refresh(); // Refresh to show animated changes
}



void GraphNodeCtrl::SaveNodePositions(const String& filename) {
    FileOut out(filename);
    if (!out) {
        LOG("Could not open file for writing: " << filename);
        return;
    }
    
    out << "NodePositions\n";
    for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
        Node& node = GetGraph().GetNode(i);
        out << node.id << "\t" << (int)node.point.x << "\t" << (int)node.point.y 
            << "\t" << node.layout_pos_x << "\t" << node.layout_pos_y << "\n";
    }
}

void GraphNodeCtrl::LoadNodePositions(const String& filename) {
    FileIn in(filename);
    if (!in) {
        LOG("Could not open file for reading: " << filename);
        return;
    }
    
    String data = LoadFile(filename);
    Vector<String> lines = Split(data, "\n");
    
    if (lines.GetCount() == 0 || lines[0] != "NodePositions") {
        LOG("Invalid file format");
        return;
    }
    
    // Create a map for easy lookup of nodes by ID
    VectorMap<String, int> nodeIdToIndex;
    for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
        Node& node = GetGraph().GetNode(i);
        nodeIdToIndex.Add(node.id, i);
    }
    
    for (int i = 1; i < lines.GetCount(); i++) {
        if (lines[i].IsEmpty()) continue;
        Vector<String> parts = Split(lines[i], "\t");
        if (parts.GetCount() >= 5) {
            String id = parts[0];
            int x = ScanInt(parts[1]);
            int y = ScanInt(parts[2]);
            double layout_x = ScanDouble(parts[3]);
            double layout_y = ScanDouble(parts[4]);
            
            int nodeIndex = nodeIdToIndex.Find(id);
            if (nodeIndex >= 0) {
                Node& node = GetGraph().GetNode(nodeIdToIndex[nodeIndex]);
                node.point.x = x;
                node.point.y = y;
                node.layout_pos_x = layout_x;
                node.layout_pos_y = layout_y;
            }
        }
    }
    
    Refresh();
}

void GraphNodeCtrl::SaveGraph(const String& filename) {
    FileOut out(filename);
    if (!out) {
        LOG("Could not open file for writing: " << filename);
        return;
    }
    
    out << "GraphEditorState\n";
    
    // Save editor state
    out << "EditorState\n";
    out << "ZoomFactor:" << zoomFactor << "\n";
    out << "PanOffset:" << panOffset.x << "," << panOffset.y << "\n";
    
    // Save selection state
    out << "SelectionState\n";
    
    // Save selected nodes
    out << "SelectedNodes:" << selectedNodes.GetCount() << "\n";
    for (int i = 0; i < selectedNodes.GetCount(); i++) {
        out << selectedNodes[i]->id << "\n";
    }
    
    // Save selected edges
    out << "SelectedEdges:" << selectedEdges.GetCount() << "\n";
    for (int i = 0; i < selectedEdges.GetCount(); i++) {
        out << selectedEdges[i]->source->id << "," << selectedEdges[i]->target->id << "\n";
    }
    
    // Save selected groups
    out << "SelectedGroups:" << selectedGroups.GetCount() << "\n";
    for (int i = 0; i < selectedGroups.GetCount(); i++) {
        out << selectedGroups[i]->id << "\n";
    }
    
    // Save graph data
    out << "GraphData\n";
    
    // Save nodes
    out << "Nodes:" << GetGraph().GetNodeCount() << "\n";
    for (int i = 0; i < GetGraph().GetNodeCount(); i++) {
        Node& node = GetGraph().GetNode(i);
        out << node.id << "\t" << node.label << "\t" << (int)node.point.x << "\t" << (int)node.point.y
            << "\t" << node.layout_pos_x << "\t" << node.layout_pos_y << "\t"
            << (int)node.fill_clr << "\t" << (int)node.line_clr << "\t"
            << node.line_width << "\t" << node.shape << "\t" << node.sz.cx << "\t" << node.sz.cy << "\n";
        
        // Save pins
        out << "Pins:" << node.pins.GetCount() << "\n";
        for (int j = 0; j < node.pins.GetCount(); j++) {
            Pin& pin = node.pins[j];
            out << pin.id << "\t" << pin.label << "\t" << (int)pin.kind << "\t"
                << (int)pin.position.x << "\t" << (int)pin.position.y << "\t"
                << (int)pin.color << "\t" << pin.type << "\t" << pin.size.cx << "\t" << pin.size.cy << "\n";
        }
    }
    
    // Save groups
    out << "Groups:" << GetGraph().GetGroupCount() << "\n";
    for (int i = 0; i < GetGraph().GetGroupCount(); i++) {
        GroupNode& group = GetGraph().GetGroup(i);
        out << group.id << "\t" << group.label << "\t"
            << (int)group.position.x << "\t" << (int)group.position.y << "\t"
            << group.size.cx << "\t" << group.size.cy << "\t"
            << (int)group.header_clr << "\t" << (int)group.body_clr << "\t"
            << (int)group.border_clr << "\t" << group.border_width << "\t"
            << group.is_collapsed << "\t" << group.isSelected << "\t"
            << group.node_ids.GetCount() << "\n";
        
        // Save nodes in group
        for (int j = 0; j < group.node_ids.GetCount(); j++) {
            out << group.node_ids[j] << "\n";
        }
    }
    
    // Save edges
    out << "Edges:" << GetGraph().GetEdgeCount() << "\n";
    for (int i = 0; i < GetGraph().GetEdgeCount(); i++) {
        Edge& edge = GetGraph().GetEdge(i);
        out << edge.source->id << "\t" << edge.target->id << "\t" << edge.label << "\t"
            << edge.weight << "\t" << edge.line_width << "\t" << (int)edge.stroke_clr << "\t"
            << edge.directed << "\t" << edge.isSelected << "\n";
    }
}

void GraphNodeCtrl::LoadGraph(const String& filename) {
    // ... existing implementation ...
}

void GraphNodeCtrl::FocusOnSelection() {
    if (selectedNodes.GetCount() > 0) {
        int minX = INT_MAX, minY = INT_MAX;
        int maxX = INT_MIN, maxY = INT_MIN;
        
        for (int i = 0; i < selectedNodes.GetCount(); i++) {
            Node* node = selectedNodes[i];
            Rect nodeRect = node->GetBoundingBox();
            minX = min(minX, nodeRect.left);
            minY = min(minY, nodeRect.top);
            maxX = max(maxX, nodeRect.right);
            maxY = max(maxY, nodeRect.bottom);
        }
        
        int centerX = (minX + maxX) / 2;
        int centerY = (minY + maxY) / 2;
        
        Size currentSize = GetSize();
        int targetPanX = (currentSize.cx / 2) - centerX;
        int targetPanY = (currentSize.cy / 2) - centerY;
        
        StartZoomAnimation(1.0, Point(targetPanX, targetPanY));
    } else if (selectedNode) {
        FocusOnNode(*selectedNode);
    }
}

void GraphNodeCtrl::FocusOnNode(Node& node) {
    Size currentSize = GetSize();
    int targetPanX = (currentSize.cx / 2) - (int)node.point.x;
    int targetPanY = (currentSize.cy / 2) - (int)node.point.y;
    
    StartZoomAnimation(1.0, Point(targetPanX, targetPanY));
}

void GraphNodeCtrl::StartEdgeFlowAnimation() {
    for(int i = 0; i < GetGraph().GetEdgeCount(); i++) {
        Edge& edge = GetGraph().GetEdge(i);
        StartEdgeFlowAnimationForEdge(edge);
    }
    Refresh();
}

void GraphNodeCtrl::StartEdgeFlowAnimationForEdge(Edge& edge) {
    edge.SetFlowSpeed(0.02);
    edge.StartFlowAnimation();
}

} // namespace GraphLib
