// STL-backed CtrlCore API implementation

#include "CtrlPos.h"

namespace Upp {

// Positioning methods that aren't implemented in the header
CtrlPos& CtrlPos::Move(int x, int y) {
    Point pos = ctrl.GetPos();
    ctrl.SetPos(x, y);
    return *this;
}

CtrlPos& CtrlPos::Move(const Point& pt) {
    ctrl.SetPos(pt);
    return *this;
}

CtrlPos& CtrlPos::MoveDelta(int dx, int dy) {
    Point pos = ctrl.GetPos();
    ctrl.SetPos(pos.x + dx, pos.y + dy);
    return *this;
}

CtrlPos& CtrlPos::MoveDelta(const Point& delta) {
    return MoveDelta(delta.x, delta.y);
}

// Sizing methods
CtrlPos& CtrlPos::Size(int cx, int cy) {
    ctrl.SetSize(cx, cy);
    return *this;
}

CtrlPos& CtrlPos::Size(const Size& sz) {
    ctrl.SetSize(sz);
    return *this;
}

// Set position and size together
CtrlPos& CtrlPos::Rect(int x, int y, int cx, int cy) {
    ctrl.SetRect(x, y, cx, cy);
    return *this;
}

CtrlPos& CtrlPos::Rect(const Rect& r) {
    ctrl.SetRect(r);
    return *this;
}

// Align to parent control
CtrlPos& CtrlPos::CenterParent() {
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

CtrlPos& CtrlPos::CenterParentHorz() {
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

CtrlPos& CtrlPos::CenterParentVert() {
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
CtrlPos& CtrlPos::LeftPos(int left, int cx) {
    auto parent = ctrl.GetParent();
    if (parent) {
        Point pos = ctrl.GetPos();
        ctrl.SetPos(left, pos.y);
        Size sz = ctrl.GetSize();
        ctrl.SetSize(cx, sz.cy);
    }
    return *this;
}

CtrlPos& CtrlPos::TopPos(int top, int cy) {
    auto parent = ctrl.GetParent();
    if (parent) {
        Point pos = ctrl.GetPos();
        ctrl.SetPos(pos.x, top);
        Size sz = ctrl.GetSize();
        ctrl.SetSize(sz.cx, cy);
    }
    return *this;
}

CtrlPos& CtrlPos::HSizePos(int left, int right) {
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

CtrlPos& CtrlPos::VSizePos(int top, int bottom) {
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

CtrlPos& CtrlPos::SizePos(int left, int top, int right, int bottom) {
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
CtrlPos& CtrlPos::AnchorRect(const Rect& anchor, double left, double top, double right, double bottom) {
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
CtrlPos& CtrlPos::RelativeTo(const Ctrl& other, int offset_x, int offset_y) {
    Rect other_rect = other.GetScreenRect();
    ctrl.SetPos(other_rect.right + offset_x, other_rect.top + offset_y);
    return *this;
}

CtrlPos& CtrlPos::Above(const Ctrl& other, int offset) {
    Rect other_rect = other.GetScreenRect();
    Size this_size = ctrl.GetSize();
    ctrl.SetPos(other_rect.left, other_rect.top - this_size.cy - offset);
    return *this;
}

CtrlPos& CtrlPos::Below(const Ctrl& other, int offset) {
    Rect other_rect = other.GetScreenRect();
    ctrl.SetPos(other_rect.left, other_rect.bottom + offset);
    return *this;
}

CtrlPos& CtrlPos::LeftOf(const Ctrl& other, int offset) {
    Rect other_rect = other.GetScreenRect();
    Size this_size = ctrl.GetSize();
    ctrl.SetPos(other_rect.left - this_size.cx - offset, other_rect.top);
    return *this;
}

CtrlPos& CtrlPos::RightOf(const Ctrl& other, int offset) {
    Rect other_rect = other.GetScreenRect();
    ctrl.SetPos(other_rect.right + offset, other_rect.top);
    return *this;
}

// Fit to content (if the control supports it)
CtrlPos& CtrlPos::FitToContents() {
    // In a real implementation, this would calculate the size needed for the content
    return *this;
}

// Position using alignment
CtrlPos& CtrlPos::SetHAlign(HAlign align) {
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

CtrlPos& CtrlPos::SetVAlign(VAlign align) {
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

CtrlPos& CtrlPos::SetAlign(HAlign halign, VAlign valign) {
    return SetHAlign(halign).SetVAlign(valign);
}

// Stretch to fill available space
CtrlPos& CtrlPos::Stretch() {
    auto parent = ctrl.GetParent();
    if (parent) {
        ctrl.SetRect(parent->GetRect());
    }
    return *this;
}

CtrlPos& CtrlPos::HStretch() {
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

CtrlPos& CtrlPos::VStretch() {
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
CtrlPos& CtrlPos::Margin(int margin) {
    return Margin(margin, margin, margin, margin);
}

CtrlPos& CtrlPos::Margin(int left, int top, int right, int bottom) {
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
Point CtrlPos::GetScreenPos() const {
    return ctrl.GetScreenPoint(Point(0, 0));
}

// Get position relative to another control
Point CtrlPos::GetRelativePos(const Ctrl& parent) const {
    Point this_screen = ctrl.GetScreenPoint(Point(0, 0));
    Point parent_screen = parent.GetScreenPoint(Point(0, 0));
    return Point(this_screen.x - parent_screen.x, this_screen.y - parent_screen.y);
}

}