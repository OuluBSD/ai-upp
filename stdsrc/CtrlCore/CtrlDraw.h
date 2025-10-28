#pragma once
#ifndef _CtrlCore_CtrlDraw_h_
#define _CtrlCore_CtrlDraw_h_

#include "CtrlCore.h"
#include "Ctrl.h"
#include "Draw.h"
#include <functional>

// Control drawing and painting system
class CtrlDraw : public Draw {
private:
    Ctrl& ctrl;
    Rect clip_rect;
    Vector<Rect> clip_stack;
    std::function<void(Ctrl&, Draw&)> paint_callback;
    
public:
    explicit CtrlDraw(Ctrl& c);
    
    // Draw methods that respect control's bounds and clipping
    virtual void DrawRect(const Rect& r, Color color) override;
    virtual void DrawRect(int x, int y, int cx, int cy, Color color) override;
    virtual void DrawImage(const Rect& r, const Image& img) override;
    virtual void DrawImage(int x, int y, const Image& img) override;
    virtual void DrawImage(int x, int y, int cx, int cy, const Image& img) override;
    virtual void DrawLine(int x1, int y1, int x2, int y2, Color color) override;
    virtual void DrawPolyline(const Point *pt, int n, Color color) override;
    virtual void DrawPolygon(const Point *pt, int n, Color color, Color outline) override;
    virtual void DrawEllipse(const Rect& r, Color color) override;
    virtual void DrawEllipse(int x, int y, int cx, int cy, Color color) override;
    virtual void DrawText(int x, int y, const wchar *text, Font font, Color color) override;
    virtual void DrawText(int x, int y, const char *text, Font font, Color color) override;
    virtual void DrawText(int x, int y, const String& text, Font font, Color color) override;
    
    // Control-specific drawing methods
    void DrawCtrlFrame(const Rect& r, Color color, int width = 1);
    void DrawCtrlBackground(const Rect& r, Color color);
    void DrawCtrlText(const Rect& r, const String& text, Font font, Color color, int align = ALIGN_LEFT);
    void DrawCtrlImage(const Rect& r, const Image& img, int align = ALIGN_CENTER);
    
    // Set/get clipping region specific to this control
    virtual void Clip(const Rect& r) override;
    virtual void OffsetClip(int x, int y) override;
    virtual void NoClip() override;
    virtual bool IsPaintingRect(const Rect& r) const override;
    
    // Push/pop clipping stack
    void PushClip(const Rect& r);
    void PopClip();
    
    // Check if a point is within the current clipping region
    bool IsWithinClip(const Point& pt) const;
    
    // Get the effective clipping rectangle
    Rect GetClipRect() const;
    
    // Paint control and its children
    void PaintCtrl();
    void PaintCtrl(const Rect& update_rect);
    
    // Set/get the paint callback
    void SetPaintCallback(std::function<void(Ctrl&, Draw&)> callback);
    std::function<void(Ctrl&, Draw&)> GetPaintCallback() const;
    
    // Draw focus rectangle
    void DrawFocusRect(const Rect& r);
    
    // Draw selection rectangle
    void DrawSelectionRect(const Rect& r, Color color);
    
    // Draw gradient background
    void DrawGradientBackground(const Rect& r, Color start_color, Color end_color, bool vertical = true);
    
    // Draw themed control elements
    void DrawButtonFace(const Rect& r, bool pressed = false, bool focused = false, bool enabled = true);
    void DrawCheckBox(const Rect& r, bool checked = false, bool enabled = true);
    void DrawRadioButton(const Rect& r, bool checked = false, bool enabled = true);
    void DrawProgressBar(const Rect& r, int percent, Color bar_color = Color::Green(), bool vertical = false);
    
    // Draw scrolling elements
    void DrawScrollBar(const Rect& r, int pos, int page, int max_pos, bool horizontal = true);
    void DrawScrollUpButton(const Rect& r, bool pressed = false);
    void DrawScrollDownButton(const Rect& r, bool pressed = false);
    void DrawScrollLeftButton(const Rect& r, bool pressed = false);
    void DrawScrollRightButton(const Rect& r, bool pressed = false);
    
    // Draw menu elements
    void DrawMenuBarBackground(const Rect& r, Color color = Color::LtGray());
    void DrawMenuItem(const Rect& r, const String& text, Font font, Color text_color, 
                     Color bg_color, bool selected = false, bool enabled = true);
    
    // Draw text with various alignments
    void DrawTextAlign(const Rect& r, const String& text, Font font, Color color, 
                      int align = ALIGN_LEFT, int valign = ALIGN_TOP);
    
    // Draw image with various alignments
    void DrawImageAlign(const Rect& r, const Image& img, 
                       int align = ALIGN_CENTER, int valign = ALIGN_CENTER);
    
    // Draw text with word wrapping
    void DrawWrappedText(const Rect& r, const String& text, Font font, Color color, 
                        int align = ALIGN_LEFT);
    
    // Get text size with control's font
    Size GetTextSize(const String& text, Font font = StdFont());
    
    // Measure wrapped text size
    Size GetWrappedTextSize(const String& text, Font font, int width);
    
    // Draw border with rounded corners
    void DrawRoundedFrame(const Rect& r, int radius, Color color, int width = 1);
    
    // Draw control with shadow
    void DrawShadowRect(const Rect& r, Color shadow_color = Color::LtGray(), int offset = 2);
    
    // Draw disabled control
    void DrawDisabledCtrl(const Rect& r, Color bg_color = Color::LtGray());
    
    // Apply alpha blending for semi-transparent drawing
    void SetAlpha(byte alpha);
    byte GetAlpha() const;
    
    // Begin/end group drawing operations
    void BeginGroup();
    void EndGroup();
    
    // Draw with transformation (scale, rotate)
    void SetTransform(double scale_x, double scale_y, double angle = 0.0, Point center = Point(0, 0));
    void ResetTransform();
    
    // Get control's drawing bounds
    Rect GetDrawBounds() const;
    
    // Check if drawing should occur (control is visible, within clip, etc.)
    bool ShouldDraw() const;
    
    // Set drawing quality (anti-aliasing, etc.)
    void SetDrawQuality(int quality); // 0 = low, 1 = normal, 2 = high
    int GetDrawQuality() const;
    
    // Draw with dithering for gradient effects
    void DrawDitheredRect(const Rect& r, Color color1, Color color2, int pattern = 0);
};

// Helper class for painting control content
class PaintCtx {
private:
    Ctrl& ctrl;
    Draw& draw;
    Rect paint_rect;
    bool is_valid;
    
public:
    PaintCtx(Ctrl& c, Draw& d, const Rect& rect) 
        : ctrl(c), draw(d), paint_rect(rect), is_valid(true) {}
    
    // Check if paint context is valid
    bool IsValid() const { return is_valid; }
    
    // Get drawing context
    Draw& GetDraw() { return draw; }
    const Draw& GetDraw() const { return draw; }
    
    // Get paint rectangle
    const Rect& GetPaintRect() const { return paint_rect; }
    
    // Check if point is in paint rectangle
    bool IsInPaintRect(const Point& pt) const { return paint_rect.IsPtInside(pt); }
    
    // Check if rectangle intersects paint rectangle
    bool IntersectsPaintRect(const Rect& r) const { return paint_rect.Intersects(r); }
    
    // Get clipped rectangle for painting
    Rect GetClipRect() const { return draw.IsPaintingRect(paint_rect) ? paint_rect : Rect(); }
    
    // Paint background
    void PaintBackground(Color color = Color::White()) {
        draw.DrawRect(paint_rect, color);
    }
    
    // Paint with a specific drawing function
    template<typename Func>
    void PaintWith(Func&& func) {
        if (is_valid) {
            func(draw, paint_rect);
        }
    }
    
    // Paint child controls
    void PaintChildren();
};

// Paint scope utility for safe painting
class PaintScope {
private:
    Ctrl& ctrl;
    Draw& draw;
    Rect paint_rect;
    
public:
    PaintScope(Ctrl& c, Draw& d, const Rect& r) : ctrl(c), draw(d), paint_rect(r) {
        // Set up clipping if needed
        ctrl.PushClip(paint_rect);
    }
    
    ~PaintScope() {
        // Restore clipping
        ctrl.PopClip();
    }
    
    PaintCtx GetCtx() { return PaintCtx(ctrl, draw, paint_rect); }
};

// Drawing utilities for controls
class CtrlDrawUtil {
public:
    // Calculate text position within a rectangle based on alignment
    static Point AlignTextPos(const Rect& r, const String& text, Font font, int align = ALIGN_LEFT, int valign = ALIGN_TOP);
    
    // Calculate image position within a rectangle based on alignment
    static Point AlignImagePos(const Rect& r, const Image& img, int align = ALIGN_CENTER, int valign = ALIGN_CENTER);
    
    // Draw text that fits within a rectangle (with ellipsis if needed)
    static void DrawFittedText(Draw& draw, const Rect& r, const String& text, 
                              Font font, Color color, int align = ALIGN_LEFT);
    
    // Draw image that fits within a rectangle (preserving aspect ratio)
    static void DrawFittedImage(Draw& draw, const Rect& r, const Image& img, 
                               int align = ALIGN_CENTER, int valign = ALIGN_CENTER);
    
    // Draw image that fills a rectangle (preserving aspect ratio, with cropping)
    static void DrawFilledImage(Draw& draw, const Rect& r, const Image& img, 
                               int align = ALIGN_CENTER, int valign = ALIGN_CENTER);
    
    // Draw text with optional shadow
    static void DrawTextWithShadow(Draw& draw, int x, int y, const String& text, 
                                  Font font, Color text_color, Color shadow_color = Color::Black(),
                                  int offset_x = 1, int offset_y = 1);
    
    // Draw text with outline
    static void DrawTextWithOutline(Draw& draw, int x, int y, const String& text, 
                                   Font font, Color text_color, Color outline_color = Color::Black(),
                                   int outline_width = 1);
    
    // Draw a control with visual state (pressed, focused, etc.)
    static void DrawCtrlState(Draw& draw, const Rect& r, bool pressed = false, 
                             bool focused = false, bool enabled = true, 
                             Color normal_color = Color::LtGray(),
                             Color pressed_color = Color::Gray());
    
    // Get standard control colors based on visual state
    static Color GetCtrlColor(bool pressed = false, bool focused = false, bool enabled = true);
    
    // Draw focus rectangle with standard appearance
    static void DrawStdFocusRect(Draw& draw, const Rect& r, Color color = Color::Blue());
    
    // Draw selection rectangle with standard appearance
    static void DrawStdSelectionRect(Draw& draw, const Rect& r, Color color = Color::Blue());
    
    // Draw a frame with standard appearance
    static void DrawStdFrame(Draw& draw, const Rect& r, Color color = Color::Gray(), int width = 1);
    
    // Draw a raised/sunken frame (3D effect)
    static void Draw3DFrame(Draw& draw, const Rect& r, bool raised = true, int width = 1);
    
    // Draw a themed scrollbar
    static void DrawThemedScrollbar(Draw& draw, const Rect& r, int pos, int page, int max_pos, 
                                   bool horizontal = true, bool enabled = true);
    
    // Draw a themed button
    static void DrawThemedButton(Draw& draw, const Rect& r, const String& text, Font font,
                                bool pressed = false, bool focused = false, bool enabled = true);
};

// Macro for convenient painting
#define PAINT_CTRL(ctrl, draw, rect) PaintScope _paint_scope_(ctrl, draw, rect); \
                                     PaintCtx ctx = _paint_scope_.GetCtx(); \
                                     if (ctx.IsValid())

#endif