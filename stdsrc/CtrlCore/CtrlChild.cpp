// STL-backed CtrlCore API implementation

#include "CtrlChild.h"

namespace Upp {

// Destructor
CtrlChild::~CtrlChild() {
    Clear();
}

// Add a child control
CtrlChild& CtrlChild::Add(const std::shared_ptr<Ctrl>& child) {
    if (child && !child->GetParent().lock()) {
        children.Add(child);
        child->SetParent(ctrl.shared_from_this());
        child->Show();
    }
    return *this;
}

// Add a child control with position
CtrlChild& CtrlChild::Add(const std::shared_ptr<Ctrl>& child, int x, int y) {
    if (child && !child->GetParent().lock()) {
        child->SetPos(x, y);
        return Add(child);
    }
    return *this;
}

// Add a child control with rectangle
CtrlChild& CtrlChild::Add(const std::shared_ptr<Ctrl>& child, const Rect& r) {
    if (child && !child->GetParent().lock()) {
        child->SetRect(r);
        return Add(child);
    }
    return *this;
}

// Remove a child control
CtrlChild& CtrlChild::Remove(const std::shared_ptr<Ctrl>& child) {
    if (child) {
        int index = Find(child);
        if (index >= 0) {
            children.Remove(index);
            child->SetParent(nullptr);
        }
    }
    return *this;
}

// Find child index by pointer
int CtrlChild::Find(const std::shared_ptr<Ctrl>& child) const {
    for (int i = 0; i < children.GetCount(); i++) {
        if (children[i] == child) {
            return i;
        }
    }
    return -1;
}

// Get child by index
std::shared_ptr<Ctrl> CtrlChild::operator[](int i) const {
    return i >= 0 && i < children.GetCount() ? children[i] : nullptr;
}

// Get child by index
std::shared_ptr<Ctrl> CtrlChild::Get(int i) const {
    return (*this)[i];
}

// Get first child with given label
std::shared_ptr<Ctrl> CtrlChild::Get(const char *label) const {
    for (const auto& child : children) {
        if (child && child->GetLabel() == label) {
            return child;
        }
    }
    return nullptr;
}

// Get child count
int CtrlChild::GetCount() const { 
    return children.GetCount(); 
}

// Check if empty
bool CtrlChild::IsEmpty() const { 
    return children.IsEmpty(); 
}

// Clear all children
CtrlChild& CtrlChild::Clear() {
    for (auto& child : children) {
        if (child) {
            child->SetParent(nullptr);
        }
    }
    children.Clear();
    return *this;
}

// Hide all children
CtrlChild& CtrlChild::Hide() {
    for (auto& child : children) {
        if (child) {
            child->Hide();
        }
    }
    return *this;
}

// Show all children
CtrlChild& CtrlChild::Show() {
    for (auto& child : children) {
        if (child) {
            child->Show();
        }
    }
    return *this;
}

// Enable all children
CtrlChild& CtrlChild::Enable() {
    for (auto& child : children) {
        if (child) {
            child->Enable();
        }
    }
    return *this;
}

// Disable all children
CtrlChild& CtrlChild::Disable() {
    for (auto& child : children) {
        if (child) {
            child->Disable();
        }
    }
    return *this;
}

// Refresh all children
CtrlChild& CtrlChild::Refresh() {
    for (auto& child : children) {
        if (child) {
            child->Refresh();
        }
    }
    return *this;
}

// Find topmost child at given point
std::shared_ptr<Ctrl> CtrlChild::GetTopChild(const Point& pt) const {
    // Search from back to front (topmost to bottommost)
    for (int i = children.GetCount() - 1; i >= 0; i--) {
        auto child = children[i];
        if (child && child->IsVisible() && child->IsEnabled() && 
            child->IsPointInside(pt)) {
            return child;
        }
    }
    return nullptr;
}

// Find all children that contain the point
Vector<std::shared_ptr<Ctrl>> CtrlChild::GetChildrenAt(const Point& pt) const {
    Vector<std::shared_ptr<Ctrl>> result;
    for (auto& child : children) {
        if (child && child->IsVisible() && child->IsEnabled() && 
            child->IsPointInside(pt)) {
            result.Add(child);
        }
    }
    return result;
}

// Get visible children only
Vector<std::shared_ptr<Ctrl>> CtrlChild::GetVisibleChildren() const {
    Vector<std::shared_ptr<Ctrl>> result;
    for (auto& child : children) {
        if (child && child->IsVisible()) {
            result.Add(child);
        }
    }
    return result;
}

// Get enabled children only
Vector<std::shared_ptr<Ctrl>> CtrlChild::GetEnabledChildren() const {
    Vector<std::shared_ptr<Ctrl>> result;
    for (auto& child : children) {
        if (child && child->IsEnabled()) {
            result.Add(child);
        }
    }
    return result;
}

// Get children in z-order (as they would be painted)
Vector<std::shared_ptr<Ctrl>> CtrlChild::GetChildrenZOrder() const {
    // For now, just return all children
    return children;
}

// Set z-order of a child (bring to front/back)
CtrlChild& CtrlChild::BringToFront(const std::shared_ptr<Ctrl>& child) {
    int index = Find(child);
    if (index >= 0 && index < children.GetCount() - 1) {
        children.Remove(index);
        children.Add(child);
    }
    return *this;
}

// Bring to back
CtrlChild& CtrlChild::BringToBack(const std::shared_ptr<Ctrl>& child) {
    int index = Find(child);
    if (index > 0) {
        children.Remove(index);
        children.Insert(0, child);
    }
    return *this;
}

// Swap z-order of two children
CtrlChild& CtrlChild::SwapZOrder(int i, int j) {
    if (i >= 0 && i < children.GetCount() && 
        j >= 0 && j < children.GetCount() && i != j) {
        Upp::Swap(children[i], children[j]);
    }
    return *this;
}

// Recursively find child with given label
std::shared_ptr<Ctrl> CtrlChild::FindChildByLabel(const String& label) const {
    for (auto& child : children) {
        if (child) {
            if (child->GetLabel() == label) {
                return child;
            }
            // If child also has CtrlChild, recursively search
            if (auto child_ctrl = std::dynamic_pointer_cast<Ctrl>(child)) {
                // TODO: Implement recursive search if child has children
            }
        }
    }
    return nullptr;
}

// Get all children as a vector
const Vector<std::shared_ptr<Ctrl>>& CtrlChild::GetAllChildren() const { 
    return children; 
}

}