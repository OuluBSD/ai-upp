#pragma once
#ifndef _CtrlCore_CtrlPos_h_
#define _CtrlCore_CtrlPos_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"

namespace Upp {

// Logical positioning system for controls
// This header provides the interface for the positioning system
// that is actually built into the Ctrl class in U++

// Logical positioning constants
enum {
	CENTER   = 0,
	MIDDLE   = 0,
	LEFT     = 1,
	RIGHT    = 2,
	TOP      = 1,
	BOTTOM   = 2,
	SIZE     = 3,

	MINSIZE  = -16380,
	MAXSIZE  = -16381,
	STDSIZE  = -16382,
};

class CtrlPos {
protected:
    Ctrl& ctrl;
    
public:
    explicit CtrlPos(Ctrl& c) : ctrl(c) {}
    
    // Positioning helpers that map to Ctrl methods
    Ctrl& SetPos(int x, int y, int cx, int cy) { 
        ctrl.SetRect(x, y, cx, cy); 
        return ctrl; 
    }
    
    Ctrl& SetPos(const Rect& r) { 
        ctrl.SetRect(r); 
        return ctrl; 
    }
    
    Ctrl& LeftPos(int a, int size = STDSIZE) { 
        return ctrl.LeftPos(a, size); 
    }
    
    Ctrl& RightPos(int a, int size = STDSIZE) { 
        return ctrl.RightPos(a, size); 
    }
    
    Ctrl& TopPos(int a, int size = STDSIZE) { 
        return ctrl.TopPos(a, size); 
    }
    
    Ctrl& BottomPos(int a, int size = STDSIZE) { 
        return ctrl.BottomPos(a, size); 
    }
    
    Ctrl& HSizePos(int a = 0, int b = 0) { 
        return ctrl.HSizePos(a, b); 
    }
    
    Ctrl& VSizePos(int a = 0, int b = 0) { 
        return ctrl.VSizePos(a, b); 
    }
    
    Ctrl& SizePos() { 
        return ctrl.SizePos(); 
    }
    
    Ctrl& HCenterPos(int size = STDSIZE, int delta = 0) { 
        return ctrl.HCenterPos(size, delta); 
    }
    
    Ctrl& VCenterPos(int size = STDSIZE, int delta = 0) { 
        return ctrl.VCenterPos(size, delta); 
    }
    
    // Zoomed versions (for DPI scaling)
    Ctrl& LeftPosZ(int a, int size = STDSIZE) { 
        return ctrl.LeftPosZ(a, size); 
    }
    
    Ctrl& RightPosZ(int a, int size = STDSIZE) { 
        return ctrl.RightPosZ(a, size); 
    }
    
    Ctrl& TopPosZ(int a, int size = STDSIZE) { 
        return ctrl.TopPosZ(a, size); 
    }
    
    Ctrl& BottomPosZ(int a, int size = STDSIZE) { 
        return ctrl.BottomPosZ(a, size); 
    }
    
    Ctrl& HSizePosZ(int a = 0, int b = 0) { 
        return ctrl.HSizePosZ(a, b); 
    }
    
    Ctrl& VSizePosZ(int a = 0, int b = 0) { 
        return ctrl.VSizePosZ(a, b); 
    }
    
    // Get control position and size information
    Rect GetRect() const { return ctrl.GetRect(); }
    Rect GetScreenRect() const { return ctrl.GetScreenRect(); }
    Size GetSize() const { return ctrl.GetSize(); }
    Size GetMinSize() const { return ctrl.GetMinSize(); }
    Size GetStdSize() const { return ctrl.GetStdSize(); }
    Size GetMaxSize() const { return ctrl.GetMaxSize(); }
    
    // Parent positioning
    Ctrl& CenterParent() { 
        return ctrl.SetRect(Ctrl::PosCenter(ctrl.GetStdSize().cx), 
                           Ctrl::PosCenter(ctrl.GetStdSize().cy)); 
    }
    
    // Position relative to another control
    Ctrl& PositionRelative(const Ctrl& other, int dx, int dy) {
        Rect other_screen = other.GetScreenRect();
        return ctrl.SetRect(other_screen.right + dx, other_screen.top + dy, 
                           ctrl.GetRect().Width(), ctrl.GetRect().Height());
    }
    
    // Frame positioning (for controls with frames)
    Ctrl& SetFramePos(int x, int y, int cx, int cy) {
        ctrl.SetFrameRect(x, y, cx, cy);
        return ctrl;
    }
    
    Ctrl& SetFramePos(const Rect& r) {
        ctrl.SetFrameRect(r);
        return ctrl;
    }
    
    // Layout management
    void RefreshLayout() { ctrl.RefreshLayout(); }
    void UpdateLayout() { ctrl.UpdateLayout(); }
    void RefreshParentLayout() { ctrl.RefreshParentLayout(); }
    void UpdateParentLayout() { ctrl.UpdateParentLayout(); }
};

// Position calculation utilities
class PosUtils {
public:
    // Calculate position based on alignment within a container
    static Point AlignInRect(const Rect& container, Size ctrl_size, int halign, int valign) {
        int x = 0, y = 0;
        
        switch (halign) {
            case LEFT:   x = container.left; break;
            case RIGHT:  x = container.right - ctrl_size.cx; break;
            case CENTER: x = (container.left + container.right - ctrl_size.cx) / 2; break;
            case SIZE:   x = container.left; ctrl_size.cx = container.Width(); break;
        }
        
        switch (valign) {
            case TOP:    y = container.top; break;
            case BOTTOM: y = container.bottom - ctrl_size.cy; break;
            case MIDDLE: y = (container.top + container.bottom - ctrl_size.cy) / 2; break;
            case SIZE:   y = container.top; ctrl_size.cy = container.Height(); break;
        }
        
        return Point(x, y);
    }
    
    // Calculate relative position
    static Rect CalcRelative(const Rect& container, int left, int top, int right, int bottom) {
        int l = (left >= 0) ? left : container.right + left;
        int t = (top >= 0) ? top : container.bottom + top;
        int r = (right >= 0) ? container.right - right : right;
        int b = (bottom >= 0) ? container.bottom - bottom : bottom;
        
        return Rect(l, t, r, b);
    }
};

}

#endif