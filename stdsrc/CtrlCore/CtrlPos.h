#ifndef _CtrlCore_CtrlPos_h_
#define _CtrlCore_CtrlPos_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"
#include <functional>

// Control positioning and layout management
class CtrlPos {
protected:
    Ctrl& ctrl;
    
public:
    explicit CtrlPos(Ctrl& c) : ctrl(c) {}
    
    // Positioning methods
    virtual CtrlPos& Move(int x, int y) {
        Point pos = ctrl.GetPos();
        ctrl.SetPos(x, y);
        return *this;
    }
    
    virtual CtrlPos& Move(const Point& pt) {
        ctrl.SetPos(pt);
        return *this;
    }
    
    virtual CtrlPos& MoveDelta(int dx, int dy) {
        Point pos = ctrl.GetPos();
        ctrl.SetPos(pos.x + dx, pos.y + dy);
        return *this;
    }
    
    virtual CtrlPos& MoveDelta(const Point& delta) {
        return MoveDelta(delta.x, delta.y);
    }
    
    // Sizing methods
    virtual CtrlPos& Size(int cx, int cy) {
        ctrl.SetSize(cx, cy);
        return *this;
    }
    
    virtual CtrlPos& Size(const Size& sz) {
        ctrl.SetSize(sz);
        return *this;
    }
    
    // Set position and size together
    virtual CtrlPos& Rect(int x, int y, int cx, int cy) {
        ctrl.SetRect(x, y, cx, cy);
        return *this;
    }
    
    virtual CtrlPos& Rect(const Rect& r) {
        ctrl.SetRect(r);
        return *this;
    }
    
    // Align to parent control
    virtual CtrlPos& CenterParent() {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Size ctrl_size = ctrl.GetSize();
            int x = parent_rect.left + (parent_rect.Width() - ctrl_size.cx) / 2;
            int y = parent_rect.top + (parent_rect.Height() - ctrl_size.cy) / 2;
            ctrl.SetPos(x, y);
        }
        return *this;
    }
    
    virtual CtrlPos& CenterParentHorz() {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Size ctrl_size = ctrl.GetSize();
            int x = parent_rect.left + (parent_rect.Width() - ctrl_size.cx) / 2;
            Point pos = ctrl.GetPos();
            ctrl.SetPos(x, pos.y);
        }
        return *this;
    }
    
    virtual CtrlPos& CenterParentVert() {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Size ctrl_size = ctrl.GetSize();
            int y = parent_rect.top + (parent_rect.Height() - ctrl_size.cy) / 2;
            Point pos = ctrl.GetPos();
            ctrl.SetPos(pos.x, y);
        }
        return *this;
    }
    
    // Position relative to parent
    virtual CtrlPos& LeftPos(int left, int cx) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Point pos = ctrl.GetPos();
            ctrl.SetPos(left, pos.y);
            Size sz = ctrl.GetSize();
            ctrl.SetSize(cx, sz.cy);
        }
        return *this;
    }
    
    virtual CtrlPos& TopPos(int top, int cy) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Point pos = ctrl.GetPos();
            ctrl.SetPos(pos.x, top);
            Size sz = ctrl.GetSize();
            ctrl.SetSize(sz.cx, cy);
        }
        return *this;
    }
    
    virtual CtrlPos& HSizePos(int left, int right) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Point pos = ctrl.GetPos();
            Size sz = ctrl.GetSize();
            
            int new_x = parent_rect.left + left;
            int new_cx = parent_rect.Width() - left - right;
            
            ctrl.SetPos(new_x, pos.y);
            ctrl.SetSize(new_cx, sz.cy);
        }
        return *this;
    }
    
    virtual CtrlPos& VSizePos(int top, int bottom) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Point pos = ctrl.GetPos();
            Size sz = ctrl.GetSize();
            
            int new_y = parent_rect.top + top;
            int new_cy = parent_rect.Height() - top - bottom;
            
            ctrl.SetPos(pos.x, new_y);
            ctrl.SetSize(sz.cx, new_cy);
        }
        return *this;
    }
    
    virtual CtrlPos& SizePos(int left = 0, int top = 0, int right = 0, int bottom = 0) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            int new_x = parent_rect.left + left;
            int new_y = parent_rect.top + top;
            int new_cx = parent_rect.Width() - left - right;
            int new_cy = parent_rect.Height() - top - bottom;
            
            ctrl.SetRect(new_x, new_y, new_cx, new_cy);
        }
        return *this;
    }
    
    // Anchor positioning
    virtual CtrlPos& AnchorRect(const Rect& anchor, double left = 0, double top = 0, double right = 0, double bottom = 0) {
        // Position and size the control based on anchor ratios
        Rect current = ctrl.GetRect();
        Rect new_rect = current;  // Default to current if no parent
        
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            
            // Calculate new position and size based on anchor ratios
            new_rect.left = (int)(parent_rect.left + parent_rect.Width() * left);
            new_rect.top = (int)(parent_rect.top + parent_rect.Height() * top);
            new_rect.right = (int)(parent_rect.left + parent_rect.Width() * (1.0 - right));
            new_rect.bottom = (int)(parent_rect.top + parent_rect.Height() * (1.0 - bottom));
        }
        
        ctrl.SetRect(new_rect);
        return *this;
    }
    
    // Position relative to another control
    virtual CtrlPos& RelativeTo(const Ctrl& other, int offset_x = 0, int offset_y = 0) {
        Rect other_rect = other.GetScreenRect();
        ctrl.SetPos(other_rect.right + offset_x, other_rect.top + offset_y);
        return *this;
    }
    
    virtual CtrlPos& Above(const Ctrl& other, int offset = 0) {
        Rect other_rect = other.GetScreenRect();
        Size this_size = ctrl.GetSize();
        ctrl.SetPos(other_rect.left, other_rect.top - this_size.cy - offset);
        return *this;
    }
    
    virtual CtrlPos& Below(const Ctrl& other, int offset = 0) {
        Rect other_rect = other.GetScreenRect();
        ctrl.SetPos(other_rect.left, other_rect.bottom + offset);
        return *this;
    }
    
    virtual CtrlPos& LeftOf(const Ctrl& other, int offset = 0) {
        Rect other_rect = other.GetScreenRect();
        Size this_size = ctrl.GetSize();
        ctrl.SetPos(other_rect.left - this_size.cx - offset, other_rect.top);
        return *this;
    }
    
    virtual CtrlPos& RightOf(const Ctrl& other, int offset = 0) {
        Rect other_rect = other.GetScreenRect();
        ctrl.SetPos(other_rect.right + offset, other_rect.top);
        return *this;
    }
    
    // Fit to content (if the control supports it)
    virtual CtrlPos& FitToContents() {
        // In a real implementation, this would calculate the size needed for the content
        return *this;
    }
    
    // Position using alignment
    enum class HAlign { LEFT, CENTER, RIGHT };
    enum class VAlign { TOP, MIDDLE, BOTTOM };
    
    virtual CtrlPos& SetHAlign(HAlign align) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Size ctrl_size = ctrl.GetSize();
            int x = 0;
            
            switch (align) {
                case HAlign::LEFT:   x = parent_rect.left; break;
                case HAlign::CENTER: x = parent_rect.left + (parent_rect.Width() - ctrl_size.cx) / 2; break;
                case HAlign::RIGHT:  x = parent_rect.right - ctrl_size.cx; break;
            }
            
            Point pos = ctrl.GetPos();
            ctrl.SetPos(x, pos.y);
        }
        return *this;
    }
    
    virtual CtrlPos& SetVAlign(VAlign align) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Size ctrl_size = ctrl.GetSize();
            int y = 0;
            
            switch (align) {
                case VAlign::TOP:    y = parent_rect.top; break;
                case VAlign::MIDDLE: y = parent_rect.top + (parent_rect.Height() - ctrl_size.cy) / 2; break;
                case VAlign::BOTTOM: y = parent_rect.bottom - ctrl_size.cy; break;
            }
            
            Point pos = ctrl.GetPos();
            ctrl.SetPos(pos.x, y);
        }
        return *this;
    }
    
    virtual CtrlPos& SetAlign(HAlign halign, VAlign valign) {
        return SetHAlign(halign).SetVAlign(valign);
    }
    
    // Stretch to fill available space
    virtual CtrlPos& Stretch() {
        auto parent = ctrl.GetParent();
        if (parent) {
            ctrl.SetRect(parent->GetRect());
        }
        return *this;
    }
    
    virtual CtrlPos& HStretch() {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Point pos = ctrl.GetPos();
            Size sz = ctrl.GetSize();
            ctrl.SetPos(parent_rect.left, pos.y);
            ctrl.SetSize(parent_rect.Width(), sz.cy);
        }
        return *this;
    }
    
    virtual CtrlPos& VStretch() {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            Point pos = ctrl.GetPos();
            Size sz = ctrl.GetSize();
            ctrl.SetPos(pos.x, parent_rect.top);
            ctrl.SetSize(sz.cx, parent_rect.Height());
        }
        return *this;
    }
    
    // Position with margins
    virtual CtrlPos& Margin(int margin) {
        return Margin(margin, margin, margin, margin);
    }
    
    virtual CtrlPos& Margin(int left, int top, int right, int bottom) {
        auto parent = ctrl.GetParent();
        if (parent) {
            Rect parent_rect = parent->GetRect();
            int x = parent_rect.left + left;
            int y = parent_rect.top + top;
            int cx = parent_rect.Width() - left - right;
            int cy = parent_rect.Height() - top - bottom;
            ctrl.SetRect(x, y, cx, cy);
        }
        return *this;
    }
    
    // Get screen position
    virtual Point GetScreenPos() const {
        return ctrl.GetScreenPoint(Point(0, 0));
    }
    
    // Get position relative to another control
    virtual Point GetRelativePos(const Ctrl& parent) const {
        Point this_screen = ctrl.GetScreenPoint(Point(0, 0));
        Point parent_screen = parent.GetScreenPoint(Point(0, 0));
        return Point(this_screen.x - parent_screen.x, this_screen.y - parent_screen.y);
    }
};

// Layout manager interface
class LayoutManager {
public:
    virtual ~LayoutManager() = default;
    virtual void Layout(Ctrl& ctrl) = 0;
    virtual Size GetMinSize(const Ctrl& ctrl) const = 0;
    virtual Size GetPreferredSize(const Ctrl& ctrl) const = 0;
};

// Common layout managers
class BorderLayout : public LayoutManager {
public:
    enum Region { NORTH, SOUTH, EAST, WEST, CENTER };
    
    virtual void Layout(Ctrl& container) override {
        // Implementation would position child controls in border regions
    }
    
    virtual Size GetMinSize(const Ctrl& ctrl) const override {
        return Size(0, 0); // Implement based on child controls
    }
    
    virtual Size GetPreferredSize(const Ctrl& ctrl) const override {
        return Size(100, 100); // Implement based on child controls
    }
};

class FlowLayout : public LayoutManager {
private:
    bool horizontal;
    int hgap, vgap;
    
public:
    FlowLayout(bool horz = true, int h = 5, int v = 5) : horizontal(horz), hgap(h), vgap(v) {}
    
    virtual void Layout(Ctrl& container) override {
        // Arrange child controls in a flow
    }
    
    virtual Size GetMinSize(const Ctrl& ctrl) const override {
        return Size(0, 0);
    }
    
    virtual Size GetPreferredSize(const Ctrl& ctrl) const override {
        return Size(100, 100);
    }
};

class GridLayout : public LayoutManager {
private:
    int rows, cols;
    int hgap, vgap;
    
public:
    GridLayout(int r, int c, int h = 5, int v = 5) : rows(r), cols(c), hgap(h), vgap(v) {}
    
    virtual void Layout(Ctrl& container) override {
        // Arrange child controls in a grid
    }
    
    virtual Size GetMinSize(const Ctrl& ctrl) const override {
        return Size(0, 0);
    }
    
    virtual Size GetPreferredSize(const Ctrl& ctrl) const override {
        return Size(100, 100);
    }
};

// Helper class for creating controls with positioning support
template<typename BaseCtrl>
class PosCtrl : public BaseCtrl, public CtrlPos {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    PosCtrl() : BaseCtrl(), CtrlPos(*this) {}
    explicit PosCtrl(const Rect& r) : BaseCtrl(r), CtrlPos(*this) {}
    
    // Fluent interface for positioning
    PosCtrl& at(int x, int y) { return Move(x, y); }
    PosCtrl& at(const Point& pt) { return Move(pt); }
    PosCtrl& size(int cx, int cy) { return Size(cx, cy); }
    PosCtrl& size(const Size& sz) { return Size(sz); }
    PosCtrl& rect(int x, int y, int cx, int cy) { return Rect(x, y, cx, cy); }
    PosCtrl& rect(const Rect& r) { return Rect(r); }
    PosCtrl& center() { return CenterParent(); }
    PosCtrl& hsize(int left, int right) { return HSizePos(left, right); }
    PosCtrl& vsize(int top, int bottom) { return VSizePos(top, bottom); }
    PosCtrl& pos(int left, int top, int right, int bottom) { return SizePos(left, top, right, bottom); }
    PosCtrl& margin(int m) { return Margin(m); }
    PosCtrl& margin(int left, int top, int right, int bottom) { return Margin(left, top, right, bottom); }
    
    // Alignment helpers
    PosCtrl& left() { return SetHAlign(CtrlPos::HAlign::LEFT); }
    PosCtrl& centerh() { return SetHAlign(CtrlPos::HAlign::CENTER); }
    PosCtrl& right() { return SetHAlign(CtrlPos::HAlign::RIGHT); }
    PosCtrl& top() { return SetVAlign(CtrlPos::VAlign::TOP); }
    PosCtrl& middle() { return SetVAlign(CtrlPos::VAlign::MIDDLE); }
    PosCtrl& bottom() { return SetVAlign(CtrlPos::VAlign::BOTTOM); }
    
    // Stretch helpers
    PosCtrl& hstretch() { return HStretch(); }
    PosCtrl& vstretch() { return VStretch(); }
    PosCtrl& stretch() { return Stretch(); }
};

// Global positioning utilities
class PosUtil {
public:
    // Calculate position based on alignment
    static Point AlignPos(const Size& container_size, const Size& ctrl_size, 
                         CtrlPos::HAlign halign, CtrlPos::VAlign valign) {
        int x = 0, y = 0;
        
        switch (halign) {
            case CtrlPos::HAlign::LEFT:   x = 0; break;
            case CtrlPos::HAlign::CENTER: x = (container_size.cx - ctrl_size.cx) / 2; break;
            case CtrlPos::HAlign::RIGHT:  x = container_size.cx - ctrl_size.cx; break;
        }
        
        switch (valign) {
            case CtrlPos::VAlign::TOP:    y = 0; break;
            case CtrlPos::VAlign::MIDDLE: y = (container_size.cy - ctrl_size.cy) / 2; break;
            case CtrlPos::VAlign::BOTTOM: y = container_size.cy - ctrl_size.cy; break;
        }
        
        return Point(x, y);
    }
    
    // Calculate centered position
    static Point CenterPos(const Size& container_size, const Size& ctrl_size) {
        return Point((container_size.cx - ctrl_size.cx) / 2, 
                    (container_size.cy - ctrl_size.cy) / 2);
    }
    
    // Fit rectangle inside another rectangle while preserving aspect ratio
    static Rect FitPreserveAspect(const Rect& container, const Size& content_size) {
        double container_ratio = (double)container.Width() / container.Height();
        double content_ratio = (double)content_size.cx / content_size.cy;
        
        int fit_width, fit_height;
        if (content_ratio > container_ratio) {
            // Width is the limiting factor
            fit_width = container.Width();
            fit_height = (int)(fit_width / content_ratio);
        } else {
            // Height is the limiting factor
            fit_height = container.Height();
            fit_width = (int)(fit_height * content_ratio);
        }
        
        int x = container.left + (container.Width() - fit_width) / 2;
        int y = container.top + (container.Height() - fit_height) / 2;
        
        return Rect(x, y, x + fit_width, y + fit_height);
    }
};

#endif