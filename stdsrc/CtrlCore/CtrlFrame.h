#pragma once
#ifndef _CtrlCore_CtrlFrame_h_
#define _CtrlCore_CtrlFrame_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"

// Base class for control frames
class Frame {
public:
    virtual ~Frame() = default;
    
    // Get required outer size for a given inner size
    virtual Size GetFrameSize() const = 0;
    virtual Size GetFrameSize(const Size& sz) const = 0;
    
    // Get inner rectangle for a given outer rectangle
    virtual Rect GetInner(const Rect& r) const = 0;
    
    // Get outer rectangle for a given inner rectangle
    virtual Rect GetOuter(const Rect& r) const = 0;
    
    // Paint the frame
    virtual void Paint(Draw& w, const Rect& r) const = 0;
    
    // Set frame properties
    virtual void SetFrame(int left, int top, int right, int bottom) = 0;
    virtual void SetFrame(int margin) = 0;
    
    // Check if frame is null (no frame)
    virtual bool IsNullFrame() const = 0;
    
    // Default implementations for common operations
    Size GetTotalSize(const Size& inner_size) const {
        Size frame_size = GetFrameSize();
        return Size(inner_size.cx + frame_size.cx, inner_size.cy + frame_size.cy);
    }
    
    Size GetInnerSize(const Size& outer_size) const {
        Size frame_size = GetFrameSize();
        return Size(outer_size.cx - frame_size.cx, outer_size.cy - frame_size.cy);
    }
};

// Null frame (no frame around control)
class NullFrame : public Frame {
public:
    virtual Size GetFrameSize() const override { return Size(0, 0); }
    virtual Size GetFrameSize(const Size& sz) const override { return Size(0, 0); }
    virtual Rect GetInner(const Rect& r) const override { return r; }
    virtual Rect GetOuter(const Rect& r) const override { return r; }
    virtual void Paint(Draw& w, const Rect& r) const override {}
    virtual void SetFrame(int left, int top, int right, int bottom) override {}
    virtual void SetFrame(int margin) override {}
    virtual bool IsNullFrame() const override { return true; }
};

// Static frame with fixed margins
class StaticFrame : public Frame {
protected:
    int left_margin, top_margin, right_margin, bottom_margin;
    
public:
    StaticFrame(int left = 0, int top = 0, int right = 0, int bottom = 0)
        : left_margin(left), top_margin(top), right_margin(right), bottom_margin(bottom) {}
    
    virtual Size GetFrameSize() const override { 
        return Size(left_margin + right_margin, top_margin + bottom_margin); 
    }
    
    virtual Size GetFrameSize(const Size& sz) const override { 
        return Size(left_margin + right_margin, top_margin + bottom_margin); 
    }
    
    virtual Rect GetInner(const Rect& r) const override { 
        return Rect(r.left + left_margin, r.top + top_margin,
                   r.right - right_margin, r.bottom - bottom_margin); 
    }
    
    virtual Rect GetOuter(const Rect& r) const override { 
        return Rect(r.left - left_margin, r.top - top_margin,
                   r.right + right_margin, r.bottom + bottom_margin); 
    }
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Paint the frame border if margins are positive
        if (left_margin > 0) w.DrawRect(r.left, r.top, left_margin, r.Height(), Color::Gray());
        if (top_margin > 0) w.DrawRect(r.left, r.top, r.Width(), top_margin, Color::Gray());
        if (right_margin > 0) w.DrawRect(r.right - right_margin, r.top, right_margin, r.Height(), Color::Gray());
        if (bottom_margin > 0) w.DrawRect(r.left, r.bottom - bottom_margin, r.Width(), bottom_margin, Color::Gray());
    }
    
    virtual void SetFrame(int left, int top, int right, int bottom) override {
        left_margin = left;
        top_margin = top;
        right_margin = right;
        bottom_margin = bottom;
    }
    
    virtual void SetFrame(int margin) override {
        left_margin = top_margin = right_margin = bottom_margin = margin;
    }
    
    virtual bool IsNullFrame() const override { return left_margin == 0 && top_margin == 0 && right_margin == 0 && bottom_margin == 0; }
    
    // Get/set individual margins
    int GetLeft() const { return left_margin; }
    int GetTop() const { return top_margin; }
    int GetRight() const { return right_margin; }
    int GetBottom() const { return bottom_margin; }
    
    void SetLeft(int margin) { left_margin = margin; }
    void SetTop(int margin) { top_margin = margin; }
    void SetRight(int margin) { right_margin = margin; }
    void SetBottom(int margin) { bottom_margin = margin; }
};

// Sunken frame (recessed appearance)
class SunkenFrame : public StaticFrame {
public:
    SunkenFrame(int margin = 1) : StaticFrame(margin, margin, margin, margin) {}
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Draw a sunken 3D border
        w.DrawRect(r.left, r.top, r.Width(), 1, Color::LtGray());
        w.DrawRect(r.left, r.top, 1, r.Height(), Color::LtGray());
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color::DkGray());
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color::DkGray());
    }
    
    virtual bool IsNullFrame() const override { return false; }
};

// Raised frame (protruding appearance)
class RaisedFrame : public StaticFrame {
public:
    RaisedFrame(int margin = 1) : StaticFrame(margin, margin, margin, margin) {}
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Draw a raised 3D border
        w.DrawRect(r.left, r.top, r.Width(), 1, Color::DkGray());
        w.DrawRect(r.left, r.top, 1, r.Height(), Color::DkGray());
        w.DrawRect(r.right - 1, r.top, 1, r.Height(), Color::LtGray());
        w.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color::LtGray());
    }
    
    virtual bool IsNullFrame() const override { return false; }
};

// Border frame with custom color and width
class BorderFrame : public StaticFrame {
private:
    Color border_color;
    int border_width;
    
public:
    BorderFrame(Color color = Color::Black(), int width = 1) 
        : StaticFrame(width, width, width, width), border_color(color), border_width(width) {}
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Draw a solid border
        for (int i = 0; i < border_width; i++) {
            w.DrawRect(r.left + i, r.top + i, r.Width() - 2 * i, 1, border_color);
            w.DrawRect(r.left + i, r.top + i, 1, r.Height() - 2 * i, border_color);
            w.DrawRect(r.right - 1 - i, r.top + i, 1, r.Height() - 2 * i, border_color);
            w.DrawRect(r.left + i, r.bottom - 1 - i, r.Width() - 2 * i, 1, border_color);
        }
    }
    
    void SetBorderColor(Color color) { border_color = color; }
    Color GetBorderColor() const { return border_color; }
    
    void SetBorderWidth(int width) { 
        border_width = width;
        SetFrame(width, width, width, width);
    }
    int GetBorderWidth() const { return border_width; }
};

// Padding frame (just adds padding around content)
class PaddingFrame : public StaticFrame {
public:
    PaddingFrame(int margin = 4) : StaticFrame(margin, margin, margin, margin) {}
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Padding frames don't paint anything by default
    }
    
    virtual bool IsNullFrame() const override { return false; }
};

// Frame with background color
class ColorFrame : public StaticFrame {
private:
    Color background_color;
    
public:
    ColorFrame(Color bg_color = Color::White(), int margin = 0) 
        : StaticFrame(margin, margin, margin, margin), background_color(bg_color) {}
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        // Paint background
        w.DrawRect(r, background_color);
        // Then draw base frame
        StaticFrame::Paint(w, r);
    }
    
    void SetBackgroundColor(Color color) { background_color = color; }
    Color GetBackgroundColor() const { return background_color; }
};

// Frame that contains another frame (decorator pattern)
class DecoratorFrame : public Frame {
protected:
    std::unique_ptr<Frame> frame;
    
public:
    explicit DecoratorFrame(std::unique_ptr<Frame> f) : frame(std::move(f)) {}
    
    virtual Size GetFrameSize() const override { 
        return frame ? frame->GetFrameSize() : Size(0, 0); 
    }
    
    virtual Size GetFrameSize(const Size& sz) const override { 
        return frame ? frame->GetFrameSize(sz) : Size(0, 0); 
    }
    
    virtual Rect GetInner(const Rect& r) const override { 
        return frame ? frame->GetInner(r) : r; 
    }
    
    virtual Rect GetOuter(const Rect& r) const override { 
        return frame ? frame->GetOuter(r) : r; 
    }
    
    virtual void Paint(Draw& w, const Rect& r) const override {
        if (frame) {
            frame->Paint(w, r);
        }
    }
    
    virtual void SetFrame(int left, int top, int right, int bottom) override {
        if (frame) {
            frame->SetFrame(left, top, right, bottom);
        }
    }
    
    virtual void SetFrame(int margin) override {
        if (frame) {
            frame->SetFrame(margin);
        }
    }
    
    virtual bool IsNullFrame() const override { 
        return !frame || frame->IsNullFrame(); 
    }
    
    Frame* GetFrame() const { return frame.get(); }
    void SetFrameOwner(std::unique_ptr<Frame> f) { frame = std::move(f); }
};

// Utility class for frame operations
class FrameOps {
public:
    // Combine two frames (outer frame contains inner frame)
    static std::unique_ptr<Frame> Combine(std::unique_ptr<Frame> outer, std::unique_ptr<Frame> inner) {
        class CombinedFrame : public Frame {
        private:
            std::unique_ptr<Frame> outer_frame;
            std::unique_ptr<Frame> inner_frame;
            
        public:
            CombinedFrame(std::unique_ptr<Frame> o, std::unique_ptr<Frame> i) 
                : outer_frame(std::move(o)), inner_frame(std::move(i)) {}
            
            virtual Size GetFrameSize() const override {
                Size outer_size = outer_frame ? outer_frame->GetFrameSize() : Size(0, 0);
                Size inner_size = inner_frame ? inner_frame->GetFrameSize() : Size(0, 0);
                return Size(outer_size.cx + inner_size.cx, outer_size.cy + inner_size.cy);
            }
            
            virtual Size GetFrameSize(const Size& sz) const override {
                Size outer_size = outer_frame ? outer_frame->GetFrameSize(sz) : Size(0, 0);
                Size inner_size = inner_frame ? inner_frame->GetFrameSize(sz) : Size(0, 0);
                return Size(outer_size.cx + inner_size.cx, outer_size.cy + inner_size.cy);
            }
            
            virtual Rect GetInner(const Rect& r) const override {
                Rect outer_inner = outer_frame ? outer_frame->GetInner(r) : r;
                return inner_frame ? inner_frame->GetInner(outer_inner) : outer_inner;
            }
            
            virtual Rect GetOuter(const Rect& r) const override {
                Rect inner_outer = inner_frame ? inner_frame->GetOuter(r) : r;
                return outer_frame ? outer_frame->GetOuter(inner_outer) : inner_outer;
            }
            
            virtual void Paint(Draw& w, const Rect& r) const override {
                if (outer_frame) outer_frame->Paint(w, r);
                if (inner_frame) {
                    Rect inner_rect = outer_frame ? outer_frame->GetInner(r) : r;
                    inner_frame->Paint(w, inner_rect);
                }
            }
            
            virtual void SetFrame(int left, int top, int right, int bottom) override {
                if (outer_frame) outer_frame->SetFrame(left, top, right, bottom);
                if (inner_frame) inner_frame->SetFrame(left, top, right, bottom);
            }
            
            virtual void SetFrame(int margin) override {
                if (outer_frame) outer_frame->SetFrame(margin);
                if (inner_frame) inner_frame->SetFrame(margin);
            }
            
            virtual bool IsNullFrame() const override {
                return (!outer_frame || outer_frame->IsNullFrame()) && 
                       (!inner_frame || inner_frame->IsNullFrame());
            }
        };
        
        return std::make_unique<CombinedFrame>(std::move(outer), std::move(inner));
    }
};

// Helper functions to create common frames
inline std::unique_ptr<Frame> InsetFrame(int margin = 1) {
    return std::make_unique<SunkenFrame>(margin);
}

inline std::unique_ptr<Frame> OutsetFrame(int margin = 1) {
    return std::make_unique<RaisedFrame>(margin);
}

inline std::unique_ptr<Frame> Padding(int margin) {
    return std::make_unique<PaddingFrame>(margin);
}

inline std::unique_ptr<Frame> Border(Color color = Color::Black(), int width = 1) {
    return std::make_unique<BorderFrame>(color, width);
}

inline std::unique_ptr<Frame> FrameRect(Color bg_color = Color::White(), int margin = 0) {
    return std::make_unique<ColorFrame>(bg_color, margin);
}

#endif