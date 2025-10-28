#ifndef _CtrlCore_CtrlMouse_h_
#define _CtrlCore_CtrlMouse_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "MKeys.h"
#include <functional>

// Mouse handling for controls
class CtrlMouse {
protected:
    Ctrl& ctrl;
    std::function<bool(Point, dword)> mouse_move;
    std::function<bool(Point, dword)> mouse_leave;
    std::function<bool(Point, dword)> left_down;
    std::function<bool(Point, dword)> left_up;
    std::function<bool(Point, dword)> left_double;
    std::function<bool(Point, dword)> right_down;
    std::function<bool(Point, dword)> right_up;
    std::function<bool(Point, dword)> right_double;
    std::function<bool(Point, dword)> middle_down;
    std::function<bool(Point, dword)> middle_up;
    std::function<bool(Point, dword)> middle_double;
    std::function<bool(int, Point)> mouse_wheel;
    std::function<bool(Point, dword)> mouse_enter;
    std::function<bool(Point)> cursor;
    std::function<void()> mouse_captured;
    std::function<void()> mouse_uncaptured;
    
    bool capturing;
    Point last_mouse_pos;
    dword mouse_flags;
    
public:
    explicit CtrlMouse(Ctrl& c) : ctrl(c), capturing(false), mouse_flags(0) {}
    
    // Mouse event handlers
    CtrlMouse& WhenMouseMove(std::function<bool(Point, dword)> h) { mouse_move = h; return *this; }
    CtrlMouse& WhenMouseLeave(std::function<bool(Point, dword)> h) { mouse_leave = h; return *this; }
    CtrlMouse& WhenLeftDown(std::function<bool(Point, dword)> h) { left_down = h; return *this; }
    CtrlMouse& WhenLeftUp(std::function<bool(Point, dword)> h) { left_up = h; return *this; }
    CtrlMouse& WhenLeftDouble(std::function<bool(Point, dword)> h) { left_double = h; return *this; }
    CtrlMouse& WhenRightDown(std::function<bool(Point, dword)> h) { right_down = h; return *this; }
    CtrlMouse& WhenRightUp(std::function<bool(Point, dword)> h) { right_up = h; return *this; }
    CtrlMouse& WhenRightDouble(std::function<bool(Point, dword)> h) { right_double = h; return *this; }
    CtrlMouse& WhenMiddleDown(std::function<bool(Point, dword)> h) { middle_down = h; return *this; }
    CtrlMouse& WhenMiddleUp(std::function<bool(Point, dword)> h) { middle_up = h; return *this; }
    CtrlMouse& WhenMiddleDouble(std::function<bool(Point, dword)> h) { middle_double = h; return *this; }
    CtrlMouse& WhenMouseWheel(std::function<bool(int, Point)> h) { mouse_wheel = h; return *this; }
    CtrlMouse& WhenMouseEnter(std::function<bool(Point, dword)> h) { mouse_enter = h; return *this; }
    CtrlMouse& WhenCursor(std::function<bool(Point)> h) { cursor = h; return *this; }
    CtrlMouse& WhenMouseCapture(std::function<void()> h) { mouse_captured = h; return *this; }
    CtrlMouse& WhenMouseUncapture(std::function<void()> h) { mouse_uncaptured = h; return *this; }
    
    // Process mouse move
    virtual bool ProcessMouseMove(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (mouse_move) return mouse_move(pos, flags);
        return false;
    }
    
    // Process mouse leave
    virtual bool ProcessMouseLeave(Point pos, dword flags) {
        if (mouse_leave) return mouse_leave(pos, flags);
        return false;
    }
    
    // Process left mouse button down
    virtual bool ProcessLeftDown(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (left_down) return left_down(pos, flags);
        return false;
    }
    
    // Process left mouse button up
    virtual bool ProcessLeftUp(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (left_up) return left_up(pos, flags);
        return false;
    }
    
    // Process left mouse double click
    virtual bool ProcessLeftDouble(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (left_double) return left_double(pos, flags);
        return false;
    }
    
    // Process right mouse button down
    virtual bool ProcessRightDown(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (right_down) return right_down(pos, flags);
        return false;
    }
    
    // Process right mouse button up
    virtual bool ProcessRightUp(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (right_up) return right_up(pos, flags);
        return false;
    }
    
    // Process right mouse double click
    virtual bool ProcessRightDouble(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (right_double) return right_double(pos, flags);
        return false;
    }
    
    // Process middle mouse button down
    virtual bool ProcessMiddleDown(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (middle_down) return middle_down(pos, flags);
        return false;
    }
    
    // Process middle mouse button up
    virtual bool ProcessMiddleUp(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (middle_up) return middle_up(pos, flags);
        return false;
    }
    
    // Process middle mouse double click
    virtual bool ProcessMiddleDouble(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (middle_double) return middle_double(pos, flags);
        return false;
    }
    
    // Process mouse wheel
    virtual bool ProcessMouseWheel(int delta, Point pos) {
        if (mouse_wheel) return mouse_wheel(delta, pos);
        return false;
    }
    
    // Process mouse enter
    virtual bool ProcessMouseEnter(Point pos, dword flags) {
        last_mouse_pos = pos;
        mouse_flags = flags;
        if (mouse_enter) return mouse_enter(pos, flags);
        return false;
    }
    
    // Process cursor
    virtual bool ProcessCursor(Point pos) {
        if (cursor) return cursor(pos);
        // Default cursor based on position
        return false;
    }
    
    // Mouse capture functions
    virtual bool CaptureMouse() {
        capturing = true;
        if (mouse_captured) mouse_captured();
        return true;
    }
    
    virtual bool ReleaseMouseCapture() {
        if (capturing) {
            capturing = false;
            if (mouse_uncaptured) mouse_uncaptured();
            return true;
        }
        return false;
    }
    
    virtual bool IsMouseCaptured() const {
        return capturing;
    }
    
    // Get last mouse position
    Point GetLastMousePos() const {
        return last_mouse_pos;
    }
    
    dword GetMouseFlags() const {
        return mouse_flags;
    }
    
    // Check if specific mouse button is down
    bool IsLeftDown() const { return (mouse_flags & MK_LBUTTON) != 0; }
    bool IsRightDown() const { return (mouse_flags & MK_RBUTTON) != 0; }
    bool IsMiddleDown() const { return (mouse_flags & MK_MBUTTON) != 0; }
    bool IsShiftDown() const { return (mouse_flags & MK_SHIFT) != 0; }
    bool IsCtrlDown() const { return (mouse_flags & MK_CTRL) != 0; }
    
    // Get mouse cursor position in screen coordinates
    static Point GetScreenMousePos();
    
    // Get mouse cursor position in control coordinates
    Point GetMousePos() const {
        // In a real implementation, this would convert screen coordinates to control coordinates
        return last_mouse_pos;
    }
    
    // Set cursor type
    enum CursorType {
        CURSOR_DEFAULT,
        CURSOR_ARROW,
        CURSOR_IBEAM,
        CURSOR_WAIT,
        CURSOR_CROSS,
        CURSOR_UPARROW,
        CURSOR_SIZE,
        CURSOR_ICON,
        CURSOR_SIZENWSE,
        CURSOR_SIZENESW,
        CURSOR_SIZEWE,
        CURSOR_SIZENS,
        CURSOR_SIZEALL,
        CURSOR_NO,
        CURSOR_HAND,
        CURSOR_HELP
    };
    
    virtual bool SetCursor(CursorType cursor_type) {
        // In a real implementation, this would set the cursor type
        return true;
    }
    
    // Drag and drop support
    CtrlMouse& WhenDragStart(std::function<bool(Point, dword)> handler);
    CtrlMouse& WhenDragOver(std::function<bool(Point, dword)> handler);
    CtrlMouse& WhenDragDrop(std::function<bool(Point, dword)> handler);
    CtrlMouse& WhenDragLeave(std::function<bool(Point, dword)> handler);
    
    // Common mouse operations
    CtrlMouse& Click(std::function<void()> handler) {
        return WhenLeftUp([handler](Point, dword) { handler(); return true; });
    }
    
    CtrlMouse& DoubleClick(std::function<void()> handler) {
        return WhenLeftDouble([handler](Point, dword) { handler(); return true; });
    }
    
    // Context menu
    CtrlMouse& ContextMenu(std::function<void(Point)> handler) {
        return WhenRightUp([handler](Point pos, dword) { handler(pos); return true; });
    }
    
    // Hover effects
    CtrlMouse& Hover(std::function<void(bool over)> handler) {
        WhenMouseEnter([handler](Point, dword) { handler(true); return false; });
        WhenMouseLeave([handler](Point, dword) { handler(false); return false; });
        return *this;
    }
};

// Mouse utility functions
class MouseUtil {
public:
    // Convert screen coordinates to control coordinates
    static Point ScreenToCtrl(const Ctrl& ctrl, Point screen_pt) {
        Rect screen_rect = ctrl.GetScreenRect();
        return Point(screen_pt.x - screen_rect.left, screen_pt.y - screen_rect.top);
    }
    
    // Convert control coordinates to screen coordinates
    static Point CtrlToScreen(const Ctrl& ctrl, Point ctrl_pt) {
        Rect screen_rect = ctrl.GetScreenRect();
        return Point(ctrl_pt.x + screen_rect.left, ctrl_pt.y + screen_rect.top);
    }
    
    // Check if point is within control bounds
    static bool IsPtInCtrl(const Ctrl& ctrl, Point pt) {
        return ctrl.IsPointInside(pt);
    }
    
    // Get the control under the mouse cursor
    static std::shared_ptr<Ctrl> GetCtrlAtMousePos();
    
    // Set global mouse capture
    static bool SetGlobalCapture(const std::shared_ptr<Ctrl>& ctrl);
    
    // Release global mouse capture
    static bool ReleaseGlobalCapture();
    
    // Get current captured control
    static std::shared_ptr<Ctrl> GetCapturedCtrl();
    
    // Show/hide cursor
    static bool ShowCursor(bool show);
    
    // Get cursor position
    static Point GetCursorPos();
    
    // Set cursor position
    static bool SetCursorPos(Point pos);
    
    // Check if mouse is visible
    static bool IsCursorVisible();
};

// Helper class for creating controls with mouse support
template<typename BaseCtrl>
class MouseCtrl : public BaseCtrl, public CtrlMouse {
public:
    using BaseCtrl::BaseCtrl; // Inherit constructors
    
    MouseCtrl() : BaseCtrl(), CtrlMouse(*this) {}
    explicit MouseCtrl(const Rect& r) : BaseCtrl(r), CtrlMouse(*this) {}
    
    // Convenience methods
    MouseCtrl& OnMouseEnter(std::function<bool(Point, dword)> handler) {
        return WhenMouseEnter(handler);
    }
    
    MouseCtrl& OnMouseLeave(std::function<bool(Point, dword)> handler) {
        return WhenMouseLeave(handler);
    }
    
    MouseCtrl& OnLeftDown(std::function<bool(Point, dword)> handler) {
        return WhenLeftDown(handler);
    }
    
    MouseCtrl& OnLeftUp(std::function<bool(Point, dword)> handler) {
        return WhenLeftUp(handler);
    }
    
    MouseCtrl& OnLeftDouble(std::function<bool(Point, dword)> handler) {
        return WhenLeftDouble(handler);
    }
    
    MouseCtrl& OnRightDown(std::function<bool(Point, dword)> handler) {
        return WhenRightDown(handler);
    }
    
    MouseCtrl& OnRightUp(std::function<bool(Point, dword)> handler) {
        return WhenRightUp(handler);
    }
    
    MouseCtrl& OnMouseMove(std::function<bool(Point, dword)> handler) {
        return WhenMouseMove(handler);
    }
    
    MouseCtrl& OnMouseWheel(std::function<bool(int, Point)> handler) {
        return WhenMouseWheel(handler);
    }
    
    MouseCtrl& OnClick(std::function<void()> handler) {
        return Click(handler);
    }
    
    MouseCtrl& OnDoubleClick(std::function<void()> handler) {
        return DoubleClick(handler);
    }
    
    MouseCtrl& OnContextMenu(std::function<void(Point)> handler) {
        return ContextMenu(handler);
    }
    
    MouseCtrl& OnHover(std::function<void(bool over)> handler) {
        return Hover(handler);
    }
    
    // Capture mouse within this control
    bool Capture() { return CaptureMouse(); }
    
    // Release mouse capture
    bool ReleaseCapture() { return ReleaseMouseCapture(); }
    
    // Check if mouse is captured by this control
    bool IsCaptured() const { return IsMouseCaptured(); }
    
    // Get current mouse position relative to this control
    Point GetMousePos() const { 
        Point screen_pos = MouseUtil::GetCursorPos();
        return MouseUtil::ScreenToCtrl(*this, screen_pos); 
    }
    
    // Check if mouse is currently over this control
    bool IsMouseOver() const {
        Point mouse_pos = GetMousePos();
        return IsPointInside(mouse_pos);
    }
    
    // Set cursor when mouse is over this control
    MouseCtrl& SetHoverCursor(CtrlMouse::CursorType cursor_type) {
        return Hover([this, cursor_type](bool over) {
            if (over) SetCursor(cursor_type);
        });
    }
};

// Global mouse functions
inline Point GetMousePos() {
    return MouseUtil::GetCursorPos();
}

inline bool IsMouseLeftDown() {
    // In a real implementation, this would check the actual mouse state
    return false;
}

inline bool IsMouseRightDown() {
    // In a real implementation, this would check the actual mouse state
    return false;
}

inline bool IsMouseMiddleDown() {
    // In a real implementation, this would check the actual mouse state
    return false;
}

#endif