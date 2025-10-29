// STL-backed CtrlCore frame functionality implementation

#include "CtrlFrame.h"
#include <algorithm>

namespace Upp {

// CtrlFrame base class implementations
void CtrlFrame::FramePaint(Draw& w, const Rect& r) {
    // Default implementation does nothing
}

void CtrlFrame::FrameAdd(Ctrl& parent) {
    // Default implementation does nothing
}

void CtrlFrame::FrameRemove() {
    // Default implementation does nothing
}

int CtrlFrame::OverPaint() const {
    return 0;
}

// NullFrameClass implementations
void NullFrameClass::FrameLayout(Rect& r) {
    // Null frame does nothing to layout
}

void NullFrameClass::FramePaint(Draw& w, const Rect& r) {
    // Null frame paints nothing
}

void NullFrameClass::FrameAddSize(Size& sz) {
    // Null frame adds no size
}

// Static null frame instance
static NullFrameClass s_null_frame;

CtrlFrame& NullFrame() {
    return s_null_frame;
}

// MarginFrame implementations
MarginFrame::MarginFrame() : owner(nullptr), color(Color::White()) {
    margins = Rect(0, 0, 0, 0);
}

void MarginFrame::FrameLayout(Rect& r) {
    r.left += margins.left;
    r.top += margins.top;
    r.right -= margins.right;
    r.bottom -= margins.bottom;
}

void MarginFrame::FramePaint(Draw& w, const Rect& r) {
    if (margins.left > 0) w.DrawRect(r.left, r.top, margins.left, r.Height(), color);
    if (margins.top > 0) w.DrawRect(r.left, r.top, r.Width(), margins.top, color);
    if (margins.right > 0) w.DrawRect(r.right - margins.right, r.top, margins.right, r.Height(), color);
    if (margins.bottom > 0) w.DrawRect(r.left, r.bottom - margins.bottom, r.Width(), margins.bottom, color);
}

void MarginFrame::FrameAddSize(Size& sz) {
    sz.cx += margins.left + margins.right;
    sz.cy += margins.top + margins.bottom;
}

void MarginFrame::FrameAdd(Ctrl& parent) {
    owner = &parent;
}

void MarginFrame::FrameRemove() {
    owner = nullptr;
}

void MarginFrame::SetMargins(const Rect& r) {
    margins = r;
    if (owner) owner->RefreshLayout();
}

void MarginFrame::SetColor(Color c) {
    color = c;
    if (owner) owner->Refresh();
}

// BorderFrame implementations
void BorderFrame::FrameLayout(Rect& r) {
    r.left += 1;
    r.top += 1;
    r.right -= 1;
    r.bottom -= 1;
}

void BorderFrame::FramePaint(Draw& w, const Rect& r) {
    if (border) {
        w.DrawRect(r.left, r.top, r.Width(), 1, border->frame[0]);      // Top
        w.DrawRect(r.left, r.top, 1, r.Height(), border->frame[1]);    // Left
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), border->frame[2]); // Right
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, border->frame[3]); // Bottom
    }
}

void BorderFrame::FrameAddSize(Size& sz) {
    sz.cx += 2;
    sz.cy += 2;
}

// Predefined frame instances
class InsetFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 2;
        r.top += 2;
        r.right -= 2;
        r.bottom -= 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 4;
        sz.cy += 4;
    }
};

class OutsetFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 2;
        r.top += 2;
        r.right -= 2;
        r.bottom -= 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(255, 255, 255));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(128, 128, 128));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 4;
        sz.cy += 4;
    }
};

class ButtonFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 3;
        r.top += 3;
        r.right -= 3;
        r.bottom -= 3;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(255, 255, 255));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(0, 0, 0));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(0, 0, 0));
        w.DrawRect(r.left + 1, r.top + 1, r.Width() - 2, 1, Color(192, 192, 192));
        w.DrawRect(r.left + 1, r.top + 1, 1, r.Height() - 2, Color(192, 192, 192));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 6;
        sz.cy += 6;
    }
};

class ThinInsetFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 1;
        r.top += 1;
        r.right -= 1;
        r.bottom -= 1;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
        sz.cy += 2;
    }
};

class ThinOutsetFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 1;
        r.top += 1;
        r.right -= 1;
        r.bottom -= 1;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(255, 255, 255));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(128, 128, 128));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
        sz.cy += 2;
    }
};

class BlackFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 1;
        r.top += 1;
        r.right -= 1;
        r.bottom -= 1;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(0, 0, 0));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(0, 0, 0));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(0, 0, 0));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(0, 0, 0));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
        sz.cy += 2;
    }
};

class WhiteFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 1;
        r.top += 1;
        r.right -= 1;
        r.bottom -= 1;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(255, 255, 255));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
        sz.cy += 2;
    }
};

// Separator frames
class TopSeparatorFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.top += 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.top + 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cy += 2;
    }
};

class BottomSeparatorFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.bottom -= 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.bottom - 2, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cy += 2;
    }
};

// Static frame instances
static InsetFrameClass s_inset_frame;
static OutsetFrameClass s_outset_frame;
static ButtonFrameClass s_button_frame;
static ThinInsetFrameClass s_thin_inset_frame;
static ThinOutsetFrameClass s_thin_outset_frame;
static BlackFrameClass s_black_frame;
static WhiteFrameClass s_white_frame;
static TopSeparatorFrameClass s_top_separator_frame;
static BottomSeparatorFrameClass s_bottom_separator_frame;

// Global frame accessor functions
CtrlFrame& InsetFrame() { return s_inset_frame; }
CtrlFrame& OutsetFrame() { return s_outset_frame; }
CtrlFrame& ButtonFrame() { return s_button_frame; }
CtrlFrame& ThinInsetFrame() { return s_thin_inset_frame; }
CtrlFrame& ThinOutsetFrame() { return s_thin_outset_frame; }
CtrlFrame& BlackFrame() { return s_black_frame; }
CtrlFrame& WhiteFrame() { return s_white_frame; }
CtrlFrame& TopSeparatorFrame() { return s_top_separator_frame; }
CtrlFrame& BottomSeparatorFrame() { return s_bottom_separator_frame; }

// Layout utility functions
void LayoutFrameLeft(Rect& r, Ctrl *ctrl, int cx) {
    if (ctrl && ctrl->IsVisible()) {
        ctrl->SetRect(r.left, r.top, cx, r.Height());
        r.left += cx;
    }
}

void LayoutFrameRight(Rect& r, Ctrl *ctrl, int cx) {
    if (ctrl && ctrl->IsVisible()) {
        ctrl->SetRect(r.right - cx, r.top, cx, r.Height());
        r.right -= cx;
    }
}

void LayoutFrameTop(Rect& r, Ctrl *ctrl, int cy) {
    if (ctrl && ctrl->IsVisible()) {
        ctrl->SetRect(r.left, r.top, r.Width(), cy);
        r.top += cy;
    }
}

void LayoutFrameBottom(Rect& r, Ctrl *ctrl, int cy) {
    if (ctrl && ctrl->IsVisible()) {
        ctrl->SetRect(r.left, r.bottom - cy, r.Width(), cy);
        r.bottom -= cy;
    }
}

// Additional frame classes that need implementation
class FieldFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 2;
        r.top += 2;
        r.right -= 2;
        r.bottom -= 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 4;
        sz.cy += 4;
    }
};

class XPFieldFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 3;
        r.top += 3;
        r.right -= 3;
        r.bottom -= 3;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, r.Width(), 1, Color(128, 128, 128));
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color(255, 255, 255));
        w.DrawRect(r.left + 1, r.top + 1, r.Width() - 2, 1, Color(220, 220, 220));
        w.DrawRect(r.left + 1, r.top + 1, 1, r.Height() - 2, Color(220, 220, 220));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 6;
        sz.cy += 6;
    }
};

static FieldFrameClass s_field_frame;
static XPFieldFrameClass s_xp_field_frame;

CtrlFrame& FieldFrame() { return s_field_frame; }
CtrlFrame& XPFieldFrame() { return s_xp_field_frame; }

// Separator frame classes
class LeftSeparatorFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.left += 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.left, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.left + 1, r.top, 1, r.Height(), Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
    }
};

class RightSeparatorFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.right -= 2;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        w.DrawRect(r.right - 2, r.top, 1, r.Height(), Color(128, 128, 128));
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color(255, 255, 255));
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 2;
    }
};

static LeftSeparatorFrameClass s_left_separator_frame;
static RightSeparatorFrameClass s_right_separator_frame;

CtrlFrame& LeftSeparatorFrame() { return s_left_separator_frame; }
CtrlFrame& RightSeparatorFrame() { return s_right_separator_frame; }

// Gap frame
class RightGapFrameClass : public CtrlFrame {
public:
    virtual void FrameLayout(Rect& r) {
        r.right -= 5;
    }
    
    virtual void FramePaint(Draw& w, const Rect& r) {
        // Gap frame doesn't paint anything
    }
    
    virtual void FrameAddSize(Size& sz) {
        sz.cx += 5;
    }
};

static RightGapFrameClass s_right_gap_frame;

CtrlFrame& RightGapFrame() { return s_right_gap_frame; }

// Additional frame classes that might be missing from the header

// FrameCtrl template implementation
template <class T>
void FrameCtrl<T>::FrameAdd(Ctrl& parent) { 
    parent.Add(*this); 
}

template <class T>
void FrameCtrl<T>::FrameRemove() { 
    this->Ctrl::Remove(); 
}

// FrameLR template implementation
template <class T>
void FrameLR<T>::FrameAddSize(Size& sz) { 
    sz.cx += Nvl(this->cx, FrameButtonWidth()) * this->IsShown(); 
}

// FrameLeft template implementation
template <class T>
void FrameLeft<T>::FrameLayout(Rect& r) {
    LayoutFrameLeft(r, this, Nvl(this->cx, FrameButtonWidth()));
}

// FrameRight template implementation
template <class T>
void FrameRight<T>::FrameLayout(Rect& r) {
    LayoutFrameRight(r, this, Nvl(this->cx, FrameButtonWidth()));
}

// FrameTB template implementation
template <class T>
void FrameTB<T>::FrameAddSize(Size& sz) { 
    sz.cy += Nvl(this->cy, sz.cx) * this->IsShown(); 
}

// FrameTop template implementation
template <class T>
void FrameTop<T>::FrameLayout(Rect& r) {
    LayoutFrameTop(r, this, Nvl(this->cy, r.Width()));
}

// FrameBottom template implementation
template <class T>
void FrameBottom<T>::FrameLayout(Rect& r) {
    LayoutFrameBottom(r, this, Nvl(this->cy, r.Width()));
}

}