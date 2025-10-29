// STL-backed CtrlCore control drawing functionality implementation

#include "CtrlDraw.h"
#include <cmath>
#include <algorithm>

namespace Upp {

// Constructor
CtrlDraw::CtrlDraw(Ctrl& c) : ctrl(c), clip_rect(0, 0, 0, 0) {
    // Initialize with control's rectangle
    clip_rect = c.GetRect();
}

// Draw methods that respect control's bounds and clipping
void CtrlDraw::DrawRect(const Rect& r, Color color) {
    // Clip rectangle to control bounds and current clipping region
    Rect clipped = r & clip_rect;
    if (!clipped.IsEmpty()) {
        Draw::DrawRect(clipped, color);
    }
}

void CtrlDraw::DrawRect(int x, int y, int cx, int cy, Color color) {
    DrawRect(Rect(x, y, x + cx, y + cy), color);
}

void CtrlDraw::DrawImage(const Rect& r, const Image& img) {
    Rect clipped = r & clip_rect;
    if (!clipped.IsEmpty() && !img.IsEmpty()) {
        Draw::DrawImage(clipped, img);
    }
}

void CtrlDraw::DrawImage(int x, int y, const Image& img) {
    if (!img.IsEmpty()) {
        DrawImage(Rect(x, y, x + img.GetWidth(), y + img.GetHeight()), img);
    }
}

void CtrlDraw::DrawImage(int x, int y, int cx, int cy, const Image& img) {
    DrawImage(Rect(x, y, x + cx, y + cy), img);
}

void CtrlDraw::DrawLine(int x1, int y1, int x2, int y2, Color color) {
    // Simple line clipping (in a real implementation, this would be more sophisticated)
    Point p1(x1, y1);
    Point p2(x2, y2);
    
    // Check if line intersects with clip rectangle
    if (clip_rect.IsPtInside(p1) || clip_rect.IsPtInside(p2) || 
        clip_rect.Intersects(Rect(min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)))) {
        Draw::DrawLine(x1, y1, x2, y2, color);
    }
}

void CtrlDraw::DrawPolyline(const Point *pt, int n, Color color) {
    if (n > 1 && pt) {
        Draw::DrawPolyline(pt, n, color);
    }
}

void CtrlDraw::DrawPolygon(const Point *pt, int n, Color color, Color outline) {
    if (n > 2 && pt) {
        Draw::DrawPolygon(pt, n, color, outline);
    }
}

void CtrlDraw::DrawEllipse(const Rect& r, Color color) {
    Rect clipped = r & clip_rect;
    if (!clipped.IsEmpty()) {
        Draw::DrawEllipse(clipped, color);
    }
}

void CtrlDraw::DrawEllipse(int x, int y, int cx, int cy, Color color) {
    DrawEllipse(Rect(x, y, x + cx, y + cy), color);
}

void CtrlDraw::DrawText(int x, int y, const wchar *text, Font font, Color color) {
    if (text) {
        Point pt(x, y);
        if (clip_rect.IsPtInside(pt)) {
            Draw::DrawText(x, y, text, font, color);
        }
    }
}

void CtrlDraw::DrawText(int x, int y, const char *text, Font font, Color color) {
    if (text) {
        Point pt(x, y);
        if (clip_rect.IsPtInside(pt)) {
            Draw::DrawText(x, y, text, font, color);
        }
    }
}

void CtrlDraw::DrawText(int x, int y, const String& text, Font font, Color color) {
    Point pt(x, y);
    if (clip_rect.IsPtInside(pt) && !text.IsEmpty()) {
        Draw::DrawText(x, y, text, font, color);
    }
}

// Control-specific drawing methods
void CtrlDraw::DrawCtrlFrame(const Rect& r, Color color, int width) {
    if (width > 0) {
        DrawRect(r.left, r.top, r.Width(), width, color); // Top
        DrawRect(r.left, r.top, width, r.Height(), color); // Left
        DrawRect(r.right - width, r.top, width, r.Height(), color); // Right
        DrawRect(r.left, r.bottom - width, r.Width(), width, color); // Bottom
    }
}

void CtrlDraw::DrawCtrlBackground(const Rect& r, Color color) {
    DrawRect(r, color);
}

void CtrlDraw::DrawCtrlText(const Rect& r, const String& text, Font font, Color color, int align) {
    // Calculate text position based on alignment
    Size text_size = GetTextSize(text, font);
    int x = r.left;
    int y = r.top;
    
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - text_size.cx) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - text_size.cx;
            break;
        default:
            break;
    }
    
    // Vertical alignment
    y = r.top + (r.Height() - text_size.cy) / 2;
    
    DrawText(x, y, text, font, color);
}

void CtrlDraw::DrawCtrlImage(const Rect& r, const Image& img, int align) {
    if (img.IsEmpty()) return;
    
    int x = r.left;
    int y = r.top;
    
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - img.GetWidth()) / 2;
            y = r.top + (r.Height() - img.GetHeight()) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - img.GetWidth();
            break;
        default:
            break;
    }
    
    DrawImage(x, y, img);
}

// Clipping methods
void CtrlDraw::Clip(const Rect& r) {
    clip_rect = r;
    Draw::Clip(r);
}

void CtrlDraw::OffsetClip(int x, int y) {
    clip_rect.Offset(x, y);
    Draw::OffsetClip(x, y);
}

void CtrlDraw::NoClip() {
    clip_rect = ctrl.GetRect(); // Reset to control's full rectangle
    Draw::NoClip();
}

bool CtrlDraw::IsPaintingRect(const Rect& r) const {
    return Draw::IsPaintingRect(r) && clip_rect.Intersects(r);
}

// Push/pop clipping stack
void CtrlDraw::PushClip(const Rect& r) {
    clip_stack.Add(clip_rect);
    clip_rect = r;
    Draw::Clip(r);
}

void CtrlDraw::PopClip() {
    if (!clip_stack.IsEmpty()) {
        clip_rect = clip_stack.Pop();
        Draw::Clip(clip_rect);
    } else {
        NoClip();
    }
}

// Check if a point is within the current clipping region
bool CtrlDraw::IsWithinClip(const Point& pt) const {
    return clip_rect.IsPtInside(pt);
}

// Get the effective clipping rectangle
Rect CtrlDraw::GetClipRect() const {
    return clip_rect;
}

// Paint control and its children
void CtrlDraw::PaintCtrl() {
    PaintCtrl(ctrl.GetRect());
}

void CtrlDraw::PaintCtrl(const Rect& update_rect) {
    // In a real implementation, this would paint the control and its children
    // For now, just refresh the control
    ctrl.Refresh(update_rect);
}

// Set/get the paint callback
void CtrlDraw::SetPaintCallback(std::function<void(Ctrl&, Draw&)> callback) {
    paint_callback = callback;
}

std::function<void(Ctrl&, Draw&)> CtrlDraw::GetPaintCallback() const {
    return paint_callback;
}

// Draw focus rectangle
void CtrlDraw::DrawFocusRect(const Rect& r) {
    // Draw dotted rectangle for focus indication
    DrawLine(r.left, r.top, r.right, r.top, Color::Black()); // Top
    DrawLine(r.left, r.bottom, r.right, r.bottom, Color::Black()); // Bottom
    DrawLine(r.left, r.top, r.left, r.bottom, Color::Black()); // Left
    DrawLine(r.right, r.top, r.right, r.bottom, Color::Black()); // Right
}

// Draw selection rectangle
void CtrlDraw::DrawSelectionRect(const Rect& r, Color color) {
    DrawRect(r, Blend(color, Color::White(), 128)); // Semi-transparent selection
    DrawCtrlFrame(r, color, 1); // Border
}

// Draw gradient background
void CtrlDraw::DrawGradientBackground(const Rect& r, Color start_color, Color end_color, bool vertical) {
    if (vertical) {
        // Vertical gradient
        int height = r.Height();
        for (int y = 0; y < height; y++) {
            double ratio = (double)y / (height - 1);
            Color color = Blend(start_color, end_color, (int)(ratio * 255));
            DrawLine(r.left, r.top + y, r.right, r.top + y, color);
        }
    } else {
        // Horizontal gradient
        int width = r.Width();
        for (int x = 0; x < width; x++) {
            double ratio = (double)x / (width - 1);
            Color color = Blend(start_color, end_color, (int)(ratio * 255));
            DrawLine(r.left + x, r.top, r.left + x, r.bottom, color);
        }
    }
}

// Draw themed control elements
void CtrlDraw::DrawButtonFace(const Rect& r, bool pressed, bool focused, bool enabled) {
    Color face_color = enabled ? (pressed ? Color::Gray() : Color::LtGray()) : Color::LtGray();
    Color border_color = pressed ? Color::Gray() : Color::White();
    
    DrawCtrlBackground(r, face_color);
    DrawCtrlFrame(r, border_color, 1);
    
    if (focused) {
        DrawFocusRect(Rect(r.left + 2, r.top + 2, r.right - 2, r.bottom - 2));
    }
}

void CtrlDraw::DrawCheckBox(const Rect& r, bool checked, bool enabled) {
    Color box_color = enabled ? Color::White() : Color::LtGray();
    Color check_color = enabled ? Color::Black() : Color::Gray();
    
    // Draw checkbox background
    DrawRect(r.left + 2, r.top + 2, 12, 12, box_color);
    DrawCtrlFrame(Rect(r.left + 2, r.top + 2, r.left + 14, r.top + 14), Color::Black(), 1);
    
    // Draw checkmark if checked
    if (checked) {
        Point points[3] = {
            Point(r.left + 4, r.top + 7),
            Point(r.left + 7, r.top + 10),
            Point(r.left + 10, r.top + 4)
        };
        DrawPolyline(points, 3, check_color);
    }
}

void CtrlDraw::DrawRadioButton(const Rect& r, bool checked, bool enabled) {
    Color circle_color = enabled ? Color::White() : Color::LtGray();
    Color check_color = enabled ? Color::Black() : Color::Gray();
    
    // Draw radio button circle
    DrawEllipse(r.left + 2, r.top + 2, 12, 12, circle_color);
    DrawEllipse(r.left + 2, r.top + 2, 12, 12, Color::Black());
    
    // Draw filled circle if checked
    if (checked) {
        DrawEllipse(r.left + 5, r.top + 5, 6, 6, check_color);
    }
}

void CtrlDraw::DrawProgressBar(const Rect& r, int percent, Color bar_color, bool vertical) {
    // Draw background
    DrawRect(r, Color::LtGray());
    DrawCtrlFrame(r, Color::Black(), 1);
    
    // Calculate filled area
    if (vertical) {
        int fill_height = (r.Height() * percent) / 100;
        DrawRect(r.left + 1, r.bottom - fill_height - 1, r.Width() - 2, fill_height, bar_color);
    } else {
        int fill_width = (r.Width() * percent) / 100;
        DrawRect(r.left + 1, r.top + 1, fill_width - 2, r.Height() - 2, bar_color);
    }
}

// Draw scrolling elements
void CtrlDraw::DrawScrollBar(const Rect& r, int pos, int page, int max_pos, bool horizontal) {
    // Draw scrollbar background
    DrawRect(r, Color::LtGray());
    
    // Draw scroll buttons
    if (horizontal) {
        DrawScrollLeftButton(Rect(r.left, r.top, r.left + 16, r.bottom), false);
        DrawScrollRightButton(Rect(r.right - 16, r.top, r.right, r.bottom), false);
    } else {
        DrawScrollUpButton(Rect(r.left, r.top, r.right, r.top + 16), false);
        DrawScrollDownButton(Rect(r.left, r.bottom - 16, r.right, r.bottom), false);
    }
}

void CtrlDraw::DrawScrollUpButton(const Rect& r, bool pressed) {
    DrawButtonFace(r, pressed, false, true);
    // Draw up arrow (simplified)
    DrawLine(r.left + r.Width()/2, r.top + 4, r.left + r.Width()/2, r.bottom - 4, Color::Black());
}

void CtrlDraw::DrawScrollDownButton(const Rect& r, bool pressed) {
    DrawButtonFace(r, pressed, false, true);
    // Draw down arrow (simplified)
    DrawLine(r.left + r.Width()/2, r.top + 4, r.left + r.Width()/2, r.bottom - 4, Color::Black());
}

void CtrlDraw::DrawScrollLeftButton(const Rect& r, bool pressed) {
    DrawButtonFace(r, pressed, false, true);
    // Draw left arrow (simplified)
    DrawLine(r.left + 4, r.top + r.Height()/2, r.right - 4, r.top + r.Height()/2, Color::Black());
}

void CtrlDraw::DrawScrollRightButton(const Rect& r, bool pressed) {
    DrawButtonFace(r, pressed, false, true);
    // Draw right arrow (simplified)
    DrawLine(r.left + 4, r.top + r.Height()/2, r.right - 4, r.top + r.Height()/2, Color::Black());
}

// Draw menu elements
void CtrlDraw::DrawMenuBarBackground(const Rect& r, Color color) {
    DrawCtrlBackground(r, color);
    DrawCtrlFrame(r, Color::Black(), 1);
}

void CtrlDraw::DrawMenuItem(const Rect& r, const String& text, Font font, Color text_color, 
                           Color bg_color, bool selected, bool enabled) {
    if (selected) {
        DrawCtrlBackground(r, Blend(bg_color, Color::Blue(), 128));
    } else {
        DrawCtrlBackground(r, bg_color);
    }
    
    if (enabled) {
        DrawText(r.left + 4, r.top + (r.Height() - GetTextSize(text, font).cy) / 2, text, font, text_color);
    } else {
        DrawText(r.left + 4, r.top + (r.Height() - GetTextSize(text, font).cy) / 2, text, font, Color::Gray());
    }
}

// Draw text with various alignments
void CtrlDraw::DrawTextAlign(const Rect& r, const String& text, Font font, Color color, int align, int valign) {
    Size text_size = GetTextSize(text, font);
    int x = r.left;
    int y = r.top;
    
    // Horizontal alignment
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - text_size.cx) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - text_size.cx;
            break;
        default:
            x = r.left;
            break;
    }
    
    // Vertical alignment
    switch (valign) {
        case ALIGN_MIDDLE:
            y = r.top + (r.Height() - text_size.cy) / 2;
            break;
        case ALIGN_BOTTOM:
            y = r.bottom - text_size.cy;
            break;
        default:
            y = r.top;
            break;
    }
    
    DrawText(x, y, text, font, color);
}

// Draw image with various alignments
void CtrlDraw::DrawImageAlign(const Rect& r, const Image& img, int align, int valign) {
    if (img.IsEmpty()) return;
    
    int x = r.left;
    int y = r.top;
    
    // Horizontal alignment
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - img.GetWidth()) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - img.GetWidth();
            break;
        default:
            x = r.left;
            break;
    }
    
    // Vertical alignment
    switch (valign) {
        case ALIGN_MIDDLE:
            y = r.top + (r.Height() - img.GetHeight()) / 2;
            break;
        case ALIGN_BOTTOM:
            y = r.bottom - img.GetHeight();
            break;
        default:
            y = r.top;
            break;
    }
    
    DrawImage(x, y, img);
}

// Draw text with word wrapping
void CtrlDraw::DrawWrappedText(const Rect& r, const String& text, Font font, Color color, int align) {
    // Simplified word wrapping implementation
    Size text_size = GetTextSize(text, font);
    if (text_size.cx <= r.Width()) {
        // Text fits in one line
        DrawTextAlign(r, text, font, color, align, ALIGN_TOP);
    } else {
        // Simple line breaking (in a real implementation, this would be more sophisticated)
        int line_height = font.GetCy();
        int y = r.top;
        int max_lines = r.Height() / line_height;
        
        // For now, just truncate with ellipsis if too much text
        String display_text = text;
        if (text_size.cx > r.Width() * max_lines) {
            display_text = text.Mid(0, min(text.GetLength(), r.Width() / 8)) + "...";
        }
        
        DrawText(r.left, y, display_text, font, color);
    }
}

// Get text size with control's font
Size CtrlDraw::GetTextSize(const String& text, Font font) {
    // In a real implementation, this would use proper text measurement
    if (text.IsEmpty()) return Size(0, font.GetCy());
    
    // Simplified estimation
    int avg_char_width = font.GetCy() / 2; // Rough estimate
    return Size(text.GetLength() * avg_char_width, font.GetCy());
}

// Measure wrapped text size
Size CtrlDraw::GetWrappedTextSize(const String& text, Font font, int width) {
    // Simplified implementation
    Size char_size = GetTextSize("M", font); // Estimate with a typical character
    int chars_per_line = width / char_size.cx;
    int lines = (text.GetLength() + chars_per_line - 1) / max(chars_per_line, 1);
    return Size(width, lines * char_size.cy);
}

// Draw border with rounded corners
void CtrlDraw::DrawRoundedFrame(const Rect& r, int radius, Color color, int width) {
    if (radius <= 0) {
        DrawCtrlFrame(r, color, width);
        return;
    }
    
    // Simplified rounded rectangle (in a real implementation, this would be more accurate)
    // For now, just draw regular frame
    DrawCtrlFrame(r, color, width);
}

// Draw control with shadow
void CtrlDraw::DrawShadowRect(const Rect& r, Color shadow_color, int offset) {
    // Draw shadow
    DrawRect(r.left + offset, r.top + offset, r.Width(), r.Height(), shadow_color);
    // Draw main rectangle
    DrawRect(r, Color::White());
    DrawCtrlFrame(r, Color::Black(), 1);
}

// Draw disabled control
void CtrlDraw::DrawDisabledCtrl(const Rect& r, Color bg_color) {
    DrawCtrlBackground(r, bg_color);
    // Could add visual indication of disabled state (grayed out, etc.)
}

// Alpha blending helper
Color CtrlDraw::Blend(Color c1, Color c2, int alpha) {
    if (alpha <= 0) return c1;
    if (alpha >= 255) return c2;
    
    int r = (c1.GetR() * (255 - alpha) + c2.GetR() * alpha) / 255;
    int g = (c1.GetG() * (255 - alpha) + c2.GetG() * alpha) / 255;
    int b = (c1.GetB() * (255 - alpha) + c2.GetB() * alpha) / 255;
    
    return Color(min(255, max(0, r)), min(255, max(0, g)), min(255, max(0, b)));
}

// Apply alpha blending for semi-transparent drawing
void CtrlDraw::SetAlpha(byte alpha) {
    // In a real implementation, this would set the alpha value for subsequent drawing operations
}

byte CtrlDraw::GetAlpha() const {
    // In a real implementation, this would return the current alpha value
    return 255;
}

// Begin/end group drawing operations
void CtrlDraw::BeginGroup() {
    // In a real implementation, this would begin a group of drawing operations
}

void CtrlDraw::EndGroup() {
    // In a real implementation, this would end a group of drawing operations
}

// Draw with transformation (scale, rotate)
void CtrlDraw::SetTransform(double scale_x, double scale_y, double angle, Point center) {
    // In a real implementation, this would set up coordinate transformation
}

void CtrlDraw::ResetTransform() {
    // In a real implementation, this would reset coordinate transformation
}

// Get control's drawing bounds
Rect CtrlDraw::GetDrawBounds() const {
    return ctrl.GetRect();
}

// Check if drawing should occur (control is visible, within clip, etc.)
bool CtrlDraw::ShouldDraw() const {
    return ctrl.IsVisible() && !clip_rect.IsEmpty();
}

// Set drawing quality (anti-aliasing, etc.)
void CtrlDraw::SetDrawQuality(int quality) {
    // In a real implementation, this would set the drawing quality
}

int CtrlDraw::GetDrawQuality() const {
    // In a real implementation, this would return the current drawing quality
    return 1; // Normal quality
}

// Draw with dithering for gradient effects
void CtrlDraw::DrawDitheredRect(const Rect& r, Color color1, Color color2, int pattern) {
    // Simple alternating pattern (in a real implementation, this would be more sophisticated)
    for (int y = r.top; y < r.bottom; y++) {
        for (int x = r.left; x < r.right; x++) {
            Color color = ((x + y + pattern) % 2 == 0) ? color1 : color2;
            DrawRect(x, y, 1, 1, color);
        }
    }
}

// PaintCtx implementation
void PaintCtx::PaintChildren() {
    // In a real implementation, this would paint the control's children
}

// CtrlDrawUtil implementation
Point CtrlDrawUtil::AlignTextPos(const Rect& r, const String& text, Font font, int align, int valign) {
    Size text_size(0, 0); // In a real implementation, this would measure the text
    
    int x = r.left;
    int y = r.top;
    
    // Horizontal alignment
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - text_size.cx) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - text_size.cx;
            break;
        default:
            break;
    }
    
    // Vertical alignment
    switch (valign) {
        case ALIGN_MIDDLE:
            y = r.top + (r.Height() - text_size.cy) / 2;
            break;
        case ALIGN_BOTTOM:
            y = r.bottom - text_size.cy;
            break;
        default:
            break;
    }
    
    return Point(x, y);
}

Point CtrlDrawUtil::AlignImagePos(const Rect& r, const Image& img, int align, int valign) {
    if (img.IsEmpty()) return Point(r.left, r.top);
    
    int x = r.left;
    int y = r.top;
    
    // Horizontal alignment
    switch (align) {
        case ALIGN_CENTER:
            x = r.left + (r.Width() - img.GetWidth()) / 2;
            break;
        case ALIGN_RIGHT:
            x = r.right - img.GetWidth();
            break;
        default:
            break;
    }
    
    // Vertical alignment
    switch (valign) {
        case ALIGN_MIDDLE:
            y = r.top + (r.Height() - img.GetHeight()) / 2;
            break;
        case ALIGN_BOTTOM:
            y = r.bottom - img.GetHeight();
            break;
        default:
            break;
    }
    
    return Point(x, y);
}

void CtrlDrawUtil::DrawFittedText(Draw& draw, const Rect& r, const String& text, 
                                 Font font, Color color, int align) {
    // In a real implementation, this would draw text that fits within the rectangle
    // with ellipsis if needed
    draw.DrawText(r.left, r.top, text, font, color);
}

void CtrlDrawUtil::DrawFittedImage(Draw& draw, const Rect& r, const Image& img, 
                                   int align, int valign) {
    if (img.IsEmpty()) return;
    
    Point pos = AlignImagePos(r, img, align, valign);
    draw.DrawImage(pos.x, pos.y, img);
}

void CtrlDrawUtil::DrawFilledImage(Draw& draw, const Rect& r, const Image& img, 
                                  int align, int valign) {
    if (img.IsEmpty()) return;
    
    // Draw image that fills the rectangle (preserve aspect ratio, with cropping)
    Point pos = AlignImagePos(r, img, align, valign);
    draw.DrawImage(pos.x, pos.y, img.GetWidth(), img.GetHeight(), img);
}

void CtrlDrawUtil::DrawTextWithShadow(Draw& draw, int x, int y, const String& text, 
                                     Font font, Color text_color, Color shadow_color,
                                     int offset_x, int offset_y) {
    // Draw shadow first
    draw.DrawText(x + offset_x, y + offset_y, text, font, shadow_color);
    // Draw text on top
    draw.DrawText(x, y, text, font, text_color);
}

void CtrlDrawUtil::DrawTextWithOutline(Draw& draw, int x, int y, const String& text, 
                                      Font font, Color text_color, Color outline_color,
                                      int outline_width) {
    // Draw outline (simplified - in a real implementation, this would be more sophisticated)
    for (int dx = -outline_width; dx <= outline_width; dx++) {
        for (int dy = -outline_width; dy <= outline_width; dy++) {
            if (dx != 0 || dy != 0) {
                draw.DrawText(x + dx, y + dy, text, font, outline_color);
            }
        }
    }
    // Draw text on top
    draw.DrawText(x, y, text, font, text_color);
}

void CtrlDrawUtil::DrawCtrlState(Draw& draw, const Rect& r, bool pressed, 
                                bool focused, bool enabled, 
                                Color normal_color, Color pressed_color) {
    Color bg_color = pressed ? pressed_color : normal_color;
    if (!enabled) {
        bg_color = Color::LtGray();
    }
    
    draw.DrawRect(r, bg_color);
    draw.DrawRect(r.left, r.top, r.Width(), 1, Color::White());
    draw.DrawRect(r.left, r.top, 1, r.Height(), Color::White());
    draw.DrawRect(r.right - 1, r.top, 1, r.Height(), Color::Gray());
    draw.DrawRect(r.left, r.bottom - 1, r.Width(), 1, Color::Gray());
    
    if (focused) {
        CtrlDrawUtil::DrawStdFocusRect(draw, Rect(r.left + 2, r.top + 2, r.right - 2, r.bottom - 2));
    }
}

Color CtrlDrawUtil::GetCtrlColor(bool pressed, bool focused, bool enabled) {
    if (!enabled) return Color::LtGray();
    if (pressed) return Color::Gray();
    return Color::LtGray();
}

void CtrlDrawUtil::DrawStdFocusRect(Draw& draw, const Rect& r, Color color) {
    // Draw dotted rectangle for focus indication
    for (int x = r.left; x < r.right; x += 2) {
        draw.DrawRect(x, r.top, 1, 1, color);
        draw.DrawRect(x, r.bottom - 1, 1, 1, color);
    }
    for (int y = r.top; y < r.bottom; y += 2) {
        draw.DrawRect(r.left, y, 1, 1, color);
        draw.DrawRect(r.right - 1, y, 1, 1, color);
    }
}

void CtrlDrawUtil::DrawStdSelectionRect(Draw& draw, const Rect& r, Color color) {
    draw.DrawRect(r, Blend(color, Color::White(), 128)); // Semi-transparent
    draw.DrawRect(r.left, r.top, r.Width(), 1, color);
    draw.DrawRect(r.left, r.top, 1, r.Height(), color);
    draw.DrawRect(r.right - 1, r.top, 1, r.Height(), color);
    draw.DrawRect(r.left, r.bottom - 1, r.Width(), 1, color);
}

void CtrlDrawUtil::DrawStdFrame(Draw& draw, const Rect& r, Color color, int width) {
    if (width > 0) {
        draw.DrawRect(r.left, r.top, r.Width(), width, color); // Top
        draw.DrawRect(r.left, r.top, width, r.Height(), color); // Left
        draw.DrawRect(r.right - width, r.top, width, r.Height(), color); // Right
        draw.DrawRect(r.left, r.bottom - width, r.Width(), width, color); // Bottom
    }
}

void CtrlDrawUtil::Draw3DFrame(Draw& draw, const Rect& r, bool raised, int width) {
    Color light = raised ? Color::White() : Color::Gray();
    Color dark = raised ? Color::Gray() : Color::Black();
    
    for (int i = 0; i < width; i++) {
        draw.DrawRect(r.left + i, r.top + i, r.Width() - 2*i, 1, light); // Top
        draw.DrawRect(r.left + i, r.top + i, 1, r.Height() - 2*i, light); // Left
        draw.DrawRect(r.right - 1 - i, r.top + i, 1, r.Height() - 2*i, dark); // Right
        draw.DrawRect(r.left + i, r.bottom - 1 - i, r.Width() - 2*i, 1, dark); // Bottom
    }
}

void CtrlDrawUtil::DrawThemedScrollbar(Draw& draw, const Rect& r, int pos, int page, int max_pos, 
                                       bool horizontal, bool enabled) {
    Color bg_color = enabled ? Color::LtGray() : Color::Gray();
    Color thumb_color = enabled ? Color::Gray() : Color::LtGray();
    
    // Draw background
    draw.DrawRect(r, bg_color);
    
    if (max_pos > 0) {
        // Calculate thumb position and size
        int range = max_pos + page;
        int thumb_size = horizontal ? (r.Width() * page) / range : (r.Height() * page) / range;
        int thumb_pos = horizontal ? (r.Width() * pos) / range : (r.Height() * pos) / range;
        
        Rect thumb_rect;
        if (horizontal) {
            thumb_rect = Rect(r.left + thumb_pos, r.top, r.left + thumb_pos + thumb_size, r.bottom);
        } else {
            thumb_rect = Rect(r.left, r.top + thumb_pos, r.right, r.top + thumb_pos + thumb_size);
        }
        
        draw.DrawRect(thumb_rect, thumb_color);
        Draw3DFrame(draw, thumb_rect, true, 1);
    }
}

void CtrlDrawUtil::DrawThemedButton(Draw& draw, const Rect& r, const String& text, Font font,
                                   bool pressed, bool focused, bool enabled) {
    DrawCtrlState(draw, r, pressed, focused, enabled);
    if (!text.IsEmpty()) {
        Color text_color = enabled ? Color::Black() : Color::Gray();
        DrawFittedText(draw, r, text, font, text_color, ALIGN_CENTER);
    }
}

}