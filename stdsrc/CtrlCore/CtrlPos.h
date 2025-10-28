#pragma once
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
    virtual CtrlPos& Move(int x, int y);
    
    virtual CtrlPos& Move(const Point& pt);
    
    virtual CtrlPos& MoveDelta(int dx, int dy);
    
    virtual CtrlPos& MoveDelta(const Point& delta);
    
    // Sizing methods
    virtual CtrlPos& Size(int cx, int cy);
    
    virtual CtrlPos& Size(const Size& sz);
    
    // Set position and size together
    virtual CtrlPos& Rect(int x, int y, int cx, int cy);
    
    virtual CtrlPos& Rect(const Rect& r);
    
    // Align to parent control
    virtual CtrlPos& CenterParent();
    
    virtual CtrlPos& CenterParentHorz();
    
    virtual CtrlPos& CenterParentVert();
    
    // Position relative to parent
    virtual CtrlPos& LeftPos(int left, int cx);
    
    virtual CtrlPos& TopPos(int top, int cy);
    
    virtual CtrlPos& HSizePos(int left, int right);
    
    virtual CtrlPos& VSizePos(int top, int bottom);
    
    virtual CtrlPos& SizePos(int left = 0, int top = 0, int right = 0, int bottom = 0);
    
    // Anchor positioning
    virtual CtrlPos& AnchorRect(const Rect& anchor, double left = 0, double top = 0, double right = 0, double bottom = 0);
    
    // Position relative to another control
    virtual CtrlPos& RelativeTo(const Ctrl& other, int offset_x = 0, int offset_y = 0);
    
    virtual CtrlPos& Above(const Ctrl& other, int offset = 0);
    
    virtual CtrlPos& Below(const Ctrl& other, int offset = 0);
    
    virtual CtrlPos& LeftOf(const Ctrl& other, int offset = 0);
    
    virtual CtrlPos& RightOf(const Ctrl& other, int offset = 0);
    
    // Fit to content (if the control supports it)
    virtual CtrlPos& FitToContents();
    
    // Position using alignment
    enum class HAlign { LEFT, CENTER, RIGHT };
    enum class VAlign { TOP, MIDDLE, BOTTOM };
    
    virtual CtrlPos& SetHAlign(HAlign align);
    
    virtual CtrlPos& SetVAlign(VAlign align);
    
    virtual CtrlPos& SetAlign(HAlign halign, VAlign valign);
    
    // Stretch to fill available space
    virtual CtrlPos& Stretch();
    
    virtual CtrlPos& HStretch();
    
    virtual CtrlPos& VStretch();
    
    // Position with margins
    virtual CtrlPos& Margin(int margin);
    
    virtual CtrlPos& Margin(int left, int top, int right, int bottom);
    
    // Get screen position
    virtual Point GetScreenPos() const;
    
    // Get position relative to another control
    virtual Point GetRelativePos(const Ctrl& parent) const;
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
    void left() { SetHAlign(CtrlPos::HAlign::LEFT); }
    void centerh() { SetHAlign(CtrlPos::HAlign::CENTER); }
    void right() { SetHAlign(CtrlPos::HAlign::RIGHT); }
    void top() { SetVAlign(CtrlPos::VAlign::TOP); }
    void middle() { SetVAlign(CtrlPos::VAlign::MIDDLE); }
    void bottom() { SetVAlign(CtrlPos::VAlign::BOTTOM); }
    
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