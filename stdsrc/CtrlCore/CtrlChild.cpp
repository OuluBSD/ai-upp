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
            // Note: In a real implementation, we'd need to check if the child implements child management
            // For now, we just check if it's a Ctrl that might have children
            if (auto child_ctrl = std::dynamic_pointer_cast<Ctrl>(child)) {
                // In a real implementation, this would check if the control has child management capability
                // For now, we assume all Ctrl objects can potentially have children
                // This would require querying if the child has its own child management
            }
        }
    }
    return nullptr;
}

// Get all children as a vector
const Vector<std::shared_ptr<Ctrl>>& CtrlChild::GetAllChildren() const { 
    return children; 
}

// Sort children by z-order (custom comparator)
CtrlChild& CtrlChild::SortChildren(std::function<bool(const std::shared_ptr<Ctrl>&, const std::shared_ptr<Ctrl>&)> compare) {
    std::sort(children.begin(), children.end(), compare);
    return *this;
}

// Get children filtered by predicate
Vector<std::shared_ptr<Ctrl>> CtrlChild::GetFilteredChildren(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    Vector<std::shared_ptr<Ctrl>> result;
    for (const auto& child : children) {
        if (child && predicate(child)) {
            result.Add(child);
        }
    }
    return result;
}

// Count children matching predicate
int CtrlChild::CountChildren(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    int count = 0;
    for (const auto& child : children) {
        if (child && predicate(child)) {
            count++;
        }
    }
    return count;
}

// Check if any child matches predicate
bool CtrlChild::AnyChild(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    for (const auto& child : children) {
        if (child && predicate(child)) {
            return true;
        }
    }
    return false;
}

// Check if all children match predicate
bool CtrlChild::AllChildren(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    if (children.IsEmpty()) return true;
    for (const auto& child : children) {
        if (child && !predicate(child)) {
            return false;
        }
    }
    return true;
}

// Get child index by predicate
int CtrlChild::FindIndex(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    for (int i = 0; i < children.GetCount(); i++) {
        if (children[i] && predicate(children[i])) {
            return i;
        }
    }
    return -1;
}

// Get first child matching predicate
std::shared_ptr<Ctrl> CtrlChild::FindFirst(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    for (const auto& child : children) {
        if (child && predicate(child)) {
            return child;
        }
    }
    return nullptr;
}

// Get last child matching predicate
std::shared_ptr<Ctrl> CtrlChild::FindLast(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) const {
    for (int i = children.GetCount() - 1; i >= 0; i--) {
        if (children[i] && predicate(children[i])) {
            return children[i];
        }
    }
    return nullptr;
}

// Remove children matching predicate
CtrlChild& CtrlChild::RemoveIf(std::function<bool(const std::shared_ptr<Ctrl>&)> predicate) {
    for (int i = children.GetCount() - 1; i >= 0; i--) {
        if (children[i] && predicate(children[i])) {
            auto child = children[i];
            children.Remove(i);
            child->SetParent(nullptr);
        }
    }
    return *this;
}

// Apply function to each child
CtrlChild& CtrlChild::ForEachChild(std::function<void(const std::shared_ptr<Ctrl>&)> func) {
    for (auto& child : children) {
        if (child) {
            func(child);
        }
    }
    return *this;
}

// Apply function to each child (const version)
void CtrlChild::ForEachChild(std::function<void(const std::shared_ptr<Ctrl>&)> func) const {
    for (auto& child : children) {
        if (child) {
            func(child);
        }
    }
}

// Get children of specific type (using RTTI)
template<typename T>
Vector<std::shared_ptr<T>> CtrlChild::GetChildrenOfType() const {
    Vector<std::shared_ptr<T>> result;
    for (const auto& child : children) {
        if (child) {
            auto typed_child = std::dynamic_pointer_cast<T>(child);
            if (typed_child) {
                result.Add(typed_child);
            }
        }
    }
    return result;
}

// Move child to new position
CtrlChild& CtrlChild::MoveChild(const std::shared_ptr<Ctrl>& child, int new_index) {
    int old_index = Find(child);
    if (old_index >= 0 && new_index >= 0 && new_index < children.GetCount() && old_index != new_index) {
        children.Remove(old_index);
        children.Insert(min(new_index, children.GetCount()), child);
    }
    return *this;
}

// Swap positions of two children
CtrlChild& CtrlChild::SwapChildren(const std::shared_ptr<Ctrl>& child1, const std::shared_ptr<Ctrl>& child2) {
    int index1 = Find(child1);
    int index2 = Find(child2);
    if (index1 >= 0 && index2 >= 0 && index1 != index2) {
        Upp::Swap(children[index1], children[index2]);
    }
    return *this;
}

// Get child bounds (bounding rectangle of all children)
Rect CtrlChild::GetChildrenBounds() const {
    if (children.IsEmpty()) {
        return Rect(0, 0, 0, 0);
    }
    
    Rect bounds = children[0]->GetRect();
    for (int i = 1; i < children.GetCount(); i++) {
        if (children[i]) {
            bounds |= children[i]->GetRect();
        }
    }
    return bounds;
}

// Get child at specific position in container coordinates
std::shared_ptr<Ctrl> CtrlChild::GetChildAtPoint(const Point& pt) const {
    for (const auto& child : children) {
        if (child && child->IsVisible()) {
            Rect child_rect = child->GetRect();
            if (child_rect.IsPtInside(pt)) {
                return child;
            }
        }
    }
    return nullptr;
}

// Check if point is inside any child
bool CtrlChild::IsPointInChild(const Point& pt) const {
    return GetChildAtPoint(pt) != nullptr;
}

}