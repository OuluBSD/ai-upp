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
    
    virtual ~CtrlChild();
    
    // Add a child control
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child);
    
    // Add a child control with position
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child, int x, int y);
    
    // Add a child control with rectangle
    virtual CtrlChild& Add(const std::shared_ptr<Ctrl>& child, const Rect& r);
    
    // Remove a child control
    virtual CtrlChild& Remove(const std::shared_ptr<Ctrl>& child);
    
    // Find child index by pointer
    virtual int Find(const std::shared_ptr<Ctrl>& child) const;
    
    // Get child by index
    virtual std::shared_ptr<Ctrl> operator[](int i) const;
    
    // Get child by index
    virtual std::shared_ptr<Ctrl> Get(int i) const;
    
    // Get first child with given label
    virtual std::shared_ptr<Ctrl> Get(const char *label) const;
    
    // Get child count
    virtual int GetCount() const;
    
    // Check if empty
    virtual bool IsEmpty() const;
    
    // Clear all children
    virtual CtrlChild& Clear();
    
    // Hide all children
    virtual CtrlChild& Hide();
    
    // Show all children
    virtual CtrlChild& Show();
    
    // Enable all children
    virtual CtrlChild& Enable();
    
    // Disable all children
    virtual CtrlChild& Disable();
    
    // Refresh all children
    virtual CtrlChild& Refresh();
    
    // Find topmost child at given point
    virtual std::shared_ptr<Ctrl> GetTopChild(const Point& pt) const;
    
    // Find all children that contain the point
    virtual Vector<std::shared_ptr<Ctrl>> GetChildrenAt(const Point& pt) const;
    
    // Get visible children only
    virtual Vector<std::shared_ptr<Ctrl>> GetVisibleChildren() const;
    
    // Get enabled children only
    virtual Vector<std::shared_ptr<Ctrl>> GetEnabledChildren() const;
    
    // Get children in z-order (as they would be painted)
    virtual Vector<std::shared_ptr<Ctrl>> GetChildrenZOrder() const;
    
    // Set z-order of a child (bring to front/back)
    virtual CtrlChild& BringToFront(const std::shared_ptr<Ctrl>& child);
    
    virtual CtrlChild& BringToBack(const std::shared_ptr<Ctrl>& child);
    
    // Swap z-order of two children
    virtual CtrlChild& SwapZOrder(int i, int j);
    
    // Find child by predicate - keeping this as template in header
    template<typename Predicate>
    std::shared_ptr<Ctrl> FindChild(Predicate pred) const {
        for (auto& child : children) {
            if (child && pred(child)) {
                return child;
            }
        }
        return nullptr;
    }
    
    // ForEach over all children - keeping this as template in header
    template<typename Function>
    void ForEach(Function f) {
        for (auto& child : children) {
            if (child) {
                f(child);
            }
        }
    }
    
    // ForEach over all children (const) - keeping this as template in header
    template<typename Function>
    void ForEach(Function f) const {
        for (auto& child : children) {
            if (child) {
                f(child);
            }
        }
    }
    
    // Recursively find child with given label
    std::shared_ptr<Ctrl> FindChildByLabel(const String& label) const;
    
    // Get all children as a vector
    const Vector<std::shared_ptr<Ctrl>>& GetAllChildren() const;
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