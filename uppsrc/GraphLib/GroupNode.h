#ifndef _GraphLib_GroupNode_h_
#define _GraphLib_GroupNode_h_

#include <Core/Core.h>  // Include Core U++ types

namespace GraphLib {

// A GroupNode represents a container that can hold other nodes
struct GroupNode : Upp::Moveable<GroupNode> {
    Upp::String id;
    Upp::String label;
    Upp::Pointf position;  // Position of the group (top-left)
    Upp::Size size;        // Size of the group
    Upp::Color header_clr, body_clr, border_clr;
    int border_width;
    bool is_collapsed;
    Upp::Vector<Upp::String> node_ids;  // IDs of nodes contained in this group
    bool isSelected;
    
    GroupNode() : size(200, 200), is_collapsed(false), border_width(1), isSelected(false) {
        header_clr = Upp::Color(50, 50, 50);  // Dark gray header
        body_clr = Upp::Color(30, 30, 30);    // Darker gray body
        border_clr = Upp::Color(100, 100, 100);
    }
    
    GroupNode(Upp::String id) : id(id), size(200, 200), is_collapsed(false), border_width(1), isSelected(false) {
        header_clr = Upp::Color(50, 50, 50);  // Dark gray header
        body_clr = Upp::Color(30, 30, 30);    // Darker gray body
        border_clr = Upp::Color(100, 100, 100);
    }
    
    Upp::Rect GetBoundingBox() const {
        return Upp::RectC(position.x, position.y, size.cx, size.cy);
    }
    
    Upp::Rect GetHeaderRect() const {
        // Header is typically at the top of the group with a fixed height
        return Upp::RectC(position.x, position.y, size.cx, 25);  // 25px header height
    }
    
    // Check if a point is within the group bounds
    bool Contains(Upp::Point pt) const {
        return GetBoundingBox().Contains(pt);
    }
    
    // Check if a point is within the header area (for dragging)
    bool ContainsHeader(Upp::Point pt) const {
        return GetHeaderRect().Contains(pt);
    }
    
    // Add a node to this group
    GroupNode& AddNode(Upp::String nodeId) {
        // Check if node is already in the group
        for (int i = 0; i < node_ids.GetCount(); i++) {
            if (node_ids[i] == nodeId) return *this;  // Already added
        }
        node_ids.Add(nodeId);
        return *this;
    }
    
    // Remove a node from this group
    GroupNode& RemoveNode(Upp::String nodeId) {
        for (int i = 0; i < node_ids.GetCount(); i++) {
            if (node_ids[i] == nodeId) {
                node_ids.Remove(i);
                break;
            }
        }
        return *this;
    }
    
    // Check if this group contains a specific node
    bool ContainsNode(Upp::String nodeId) const {
        for (int i = 0; i < node_ids.GetCount(); i++) {
            if (node_ids[i] == nodeId) return true;
        }
        return false;
    }
    
    // Selection methods
    GroupNode& Select() { isSelected = true; return *this; }
    GroupNode& Deselect() { isSelected = false; return *this; }
};

}

#endif