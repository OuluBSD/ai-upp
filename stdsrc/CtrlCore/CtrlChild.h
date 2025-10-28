#ifndef _CtrlCore_CtrlChild_h_
#define _CtrlCore_CtrlChild_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include <algorithm>
#include <functional>

// Child control management system
class CtrlChild {
protected:
    Ctrl& ctrl;
    Vector<std::shared_ptr<Ctrl>> children;
    Vector<std::shared_ptr<Ctrl>> free_list; // Reusable child controls
    
public:
    explicit CtrlChild(Ctrl& c) : ctrl(c) {}
    
    virtual ~CtrlChild() {
        Clear();
    }
    
    // Add a child control
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child) {
        if (child && !child->GetParent().lock()) {
            children.Add(child);
            child->SetParent(ctrl.shared_from_this());
            child->Show();
        }
        return *this;
    }
    
    // Add a child control with position
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child, int x, int y) {
        if (child && !child->GetParent().lock()) {
            child->SetPos(x, y);
            return Add(child);
        }
        return *this;
    }
    
    // Add a child control with rectangle
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child, const Rect& r) {
        if (child && !child->GetParent().lock()) {
            child->SetRect(r);
            return Add(child);
        }
        return *this;
    }
    
    // Remove a child control
    virtual CtrlChild& Remove(const std::shared_ptr<Ctrl>& child) {
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
    virtual int Find(const std::shared_ptr<Ctrl>& child) const {
        for (int i = 0; i < children.GetCount(); i++) {
            if (children[i] == child) {
                return i;
            }
        }
        return -1;
    }
    
    // Get child by index
    virtual std::shared_ptr<Ctrl> operator[](int i) const {
        return i >= 0 && i < children.GetCount() ? children[i] : nullptr;
    }
    
    // Get child by index
    virtual std::shared_ptr<Ctrl> Get(int i) const {
        return (*this)[i];
    }
    
    // Get first child with given label
    virtual std::shared_ptr<Ctrl> Get(const char *label) const {
        for (const auto& child : children) {
            if (child && child->GetLabel() == label) {
                return child;
            }
        }
        return nullptr;
    }
    
    // Get child count
    virtual int GetCount() const { return children.GetCount(); }
    
    // Check if empty
    virtual bool IsEmpty() const { return children.IsEmpty(); }
    
    // Clear all children
    virtual CtrlChild& Clear() {
        for (auto& child : children) {
            if (child) {
                child->SetParent(nullptr);
            }
        }
        children.Clear();
        return *this;
    }
    
    // Hide all children
    virtual CtrlChild& Hide() {
        for (auto& child : children) {
            if (child) {
                child->Hide();
            }
        }
        return *this;
    }
    
    // Show all children
    virtual CtrlChild& Show() {
        for (auto& child : children) {
            if (child) {
                child->Show();
            }
        }
        return *this;
    }
    
    // Enable all children
    virtual CtrlChild& Enable() {
        for (auto& child : children) {
            if (child) {
                child->Enable();
            }
        }
        return *this;
    }
    
    // Disable all children
    virtual CtrlChild& Disable() {
        for (auto& child : children) {
            if (child) {
                child->Disable();
            }
        }
        return *this;
    }
    
    // Refresh all children
    virtual CtrlChild& Refresh() {
        for (auto& child : children) {
            if (child) {
                child->Refresh();
            }
        }
        return *this;
    }
    
    // Find topmost child at given point
    virtual std::shared_ptr<Ctrl> GetTopChild(const Point& pt) const {
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
    virtual Vector<std::shared_ptr<Ctrl>> GetChildrenAt(const Point& pt) const {
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
    virtual Vector<std::shared_ptr<Ctrl>> GetVisibleChildren() const {
        Vector<std::shared_ptr<Ctrl>> result;
        for (auto& child : children) {
            if (child && child->IsVisible()) {
                result.Add(child);
            }
        }
        return result;
    }
    
    // Get enabled children only
    virtual Vector<std::shared_ptr<Ctrl>> GetEnabledChildren() const {
        Vector<std::shared_ptr<Ctrl>> result;
        for (auto& child : children) {
            if (child && child->IsEnabled()) {
                result.Add(child);
            }
        }
        return result;
    }
    
    // Get children in z-order (as they would be painted)
    virtual Vector<std::shared_ptr<Ctrl>> GetChildrenZOrder() const {
        // For now, just return all children
        return children;
    }
    
    // Set z-order of a child (bring to front/back)
    virtual CtrlChild& BringToFront(const std::shared_ptr<Ctrl>& child) {
        int index = Find(child);
        if (index >= 0 && index < children.GetCount() - 1) {
            children.Remove(index);
            children.Add(child);
        }
        return *this;
    }
    
    virtual CtrlChild& BringToBack(const std::shared_ptr<Ctrl>& child) {
        int index = Find(child);
        if (index > 0) {
            children.Remove(index);
            children.Insert(0, child);
        }
        return *this;
    }
    
    // Swap z-order of two children
    virtual CtrlChild& SwapZOrder(int i, int j) {
        if (i >= 0 && i < children.GetCount() && 
            j >= 0 && j < children.GetCount() && i != j) {
            Upp::Swap(children[i], children[j]);
        }
        return *this;
    }
    
    // Find child by predicate
    template<typename Predicate>
    std::shared_ptr<Ctrl> FindChild(Predicate pred) const {
        for (auto& child : children) {
            if (child && pred(child)) {
                return child;
            }
        }
        return nullptr;
    }
    
    // ForEach over all children
    template<typename Function>
    void ForEach(Function f) {
        for (auto& child : children) {
            if (child) {
                f(child);
            }
        }
    }
    
    // ForEach over all children (const)
    template<typename Function>
    void ForEach(Function f) const {
        for (auto& child : children) {
            if (child) {
                f(child);
            }
        }
    }
    
    // Recursively find child with given label
    std::shared_ptr<Ctrl> FindChildByLabel(const String& label) const {
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
    const Vector<std::shared_ptr<Ctrl>>& GetAllChildren() const { return children; }
};

// Helper class for creating controls with automatic parenting
template<typename T>
class WithChild : public CtrlChild {
public:
    explicit WithChild(Ctrl& c) : CtrlChild(c) {}
    
    // Create and add a new child control
    template<typename... Args>
    T& Create(Args&&... args) {
        auto child = std::make_shared<T>(std::forward<Args>(args)...);
        Add(child);
        return *child;
    }
    
    // Create and add a new child control with position
    template<typename... Args>
    T& Create(int x, int y, Args&&... args) {
        auto child = std::make_shared<T>(std::forward<Args>(args)...);
        child->SetPos(x, y);
        Add(child);
        return *child;
    }
    
    // Create and add a new child control with rectangle
    template<typename... Args>
    T& Create(const Rect& r, Args&&... args) {
        auto child = std::make_shared<T>(std::forward<Args>(args)...);
        child->SetRect(r);
        Add(child);
        return *child;
    }
};

// Macro for convenient child control creation
#define WITH_CHILD(ctrl) CtrlChild(ctrl)

// Template class for controls with built-in child management
template<typename BaseCtrl>
class ChildCtrl : public BaseCtrl, public CtrlChild {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    ChildCtrl() : BaseCtrl(), CtrlChild(*this) {}
    explicit ChildCtrl(const Rect& r) : BaseCtrl(r), CtrlChild(*this) {}
    
    // Override paint to also paint children
    virtual void Paint(Draw& draw) const override {
        BaseCtrl::Paint(draw); // Paint self first
        // Then paint all visible children
        for (const auto& child : children) {
            if (child && child->IsVisible()) {
                // In a real implementation, this would paint the child
                child->Paint(draw);
            }
        }
    }
    
    // Override mouse events to delegate to children first
    virtual bool IsPointInside(const Point& pt) const override {
        // Check if point is in any child first
        for (const auto& child : children) {
            if (child && child->IsVisible() && child->IsEnabled() && 
                child->IsPointInside(pt)) {
                return true;
            }
        }
        // If not in any child, check parent
        return BaseCtrl::IsPointInside(pt);
    }
    
    // Add helper methods using fluent interface
    template<typename T>
    WithChild<T> With() { return WithChild<T>(*this); }
};

#endif