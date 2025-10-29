// STL-backed Draw utility implementation

#include "DrawUtil.h"
#include <cmath>
#include <algorithm>

namespace Upp {

// Draw a rounded rectangle
void DrawUtil::DrawRoundedRect(Draw& draw, const Rect& r, int radius, 
                              Color color, int pen_width) {
    if (r.IsEmpty() || radius <= 0) return;
    
    // Draw the four sides
    draw.DrawLine(r.left + radius, r.top, r.right - radius, r.top, pen_width, color); // Top
    draw.DrawLine(r.left, r.top + radius, r.left, r.bottom - radius, pen_width, color); // Left
    draw.DrawLine(r.right - 1, r.top + radius, r.right - 1, r.bottom - radius, pen_width, color); // Right
    draw.DrawLine(r.left + radius, r.bottom - 1, r.right - radius, r.bottom - 1, pen_width, color); // Bottom
    
    // Draw the four corners (approximated with arcs)
    // Top-left corner
    draw.DrawArc(Rect(r.left, r.top, r.left + 2 * radius, r.top + 2 * radius), 
                Point(r.left + radius, r.top), Point(r.left, r.top + radius), 
                pen_width, color);
    
    // Top-right corner
    draw.DrawArc(Rect(r.right - 2 * radius, r.top, r.right, r.top + 2 * radius), 
                Point(r.right - radius, r.top), Point(r.right - 1, r.top + radius), 
                pen_width, color);
    
    // Bottom-left corner
    draw.DrawArc(Rect(r.left, r.bottom - 2 * radius, r.left + 2 * radius, r.bottom), 
                Point(r.left, r.bottom - radius - 1), Point(r.left + radius, r.bottom - 1), 
                pen_width, color);
    
    // Bottom-right corner
    draw.DrawArc(Rect(r.right - 2 * radius, r.bottom - 2 * radius, r.right, r.bottom), 
                Point(r.right - 1, r.bottom - radius - 1), Point(r.right - radius, r.bottom - 1), 
                pen_width, color);
}

// Fill a rounded rectangle
void DrawUtil::FillRoundedRect(Draw& draw, const Rect& r, int radius, Color color) {
    if (r.IsEmpty() || radius <= 0) return;
    
    // Fill the main rectangle
    draw.DrawRect(r.left + radius, r.top, r.Width() - 2 * radius, r.Height(), color);
    
    // Fill the left and right rectangles
    draw.DrawRect(r.left, r.top + radius, radius, r.Height() - 2 * radius, color);
    draw.DrawRect(r.right - radius, r.top + radius, radius, r.Height() - 2 * radius, color);
    
    // Fill the four corners (approximated with filled arcs)
    // Top-left corner
    draw.DrawEllipse(Rect(r.left, r.top, r.left + 2 * radius, r.top + 2 * radius), color);
    
    // Top-right corner
    draw.DrawEllipse(Rect(r.right - 2 * radius, r.top, r.right, r.top + 2 * radius), color);
    
    // Bottom-left corner
    draw.DrawEllipse(Rect(r.left, r.bottom - 2 * radius, r.left + 2 * radius, r.bottom), color);
    
    // Bottom-right corner
    draw.DrawEllipse(Rect(r.right - 2 * radius, r.bottom - 2 * radius, r.right, r.bottom), color);
}

// Draw a circle
void DrawUtil::DrawCircle(Draw& draw, int x, int y, int radius, 
                         Color color, int pen_width) {
    if (radius <= 0) return;
    
    draw.DrawEllipse(Rect(x - radius, y - radius, x + radius, y + radius), 
                    color, pen_width, color);
}

// Fill a circle
void DrawUtil::FillCircle(Draw& draw, int x, int y, int radius, Color color) {
    if (radius <= 0) return;
    
    draw.DrawEllipse(Rect(x - radius, y - radius, x + radius, y + radius), color);
}

// Draw an ellipse
void DrawUtil::DrawEllipse(Draw& draw, const Rect& r, Color color, int pen_width) {
    if (r.IsEmpty()) return;
    
    draw.DrawEllipse(r, color, pen_width, color);
}

// Fill an ellipse
void DrawUtil::FillEllipse(Draw& draw, const Rect& r, Color color) {
    if (r.IsEmpty()) return;
    
    draw.DrawEllipse(r, color);
}

// Draw a line with anti-aliasing
void DrawUtil::DrawLine(Draw& draw, int x1, int y1, int x2, int y2, 
                       Color color, int pen_width) {
    draw.DrawLine(x1, y1, x2, y2, pen_width, color);
}

// Draw a polygon
void DrawUtil::DrawPolygon(Draw& draw, const Vector<Point>& points, 
                          Color color, int pen_width) {
    if (points.GetCount() < 2) return;
    
    draw.DrawPolyline(points.Begin(), points.GetCount(), pen_width, color);
}

// Fill a polygon
void DrawUtil::FillPolygon(Draw& draw, const Vector<Point>& points, Color color) {
    if (points.GetCount() < 3) return;
    
    // In a real implementation, this would fill the polygon
    // For now, just draw the outline
    draw.DrawPolygon(points.Begin(), points.GetCount(), color, color);
}

// Draw a triangle
void DrawUtil::DrawTriangle(Draw& draw, Point p1, Point p2, Point p3, 
                          Color color, int pen_width) {
    Point points[3] = {p1, p2, p3};
    draw.DrawPolyline(points, 3, pen_width, color);
}

// Fill a triangle
void DrawUtil::FillTriangle(Draw& draw, Point p1, Point p2, Point p3, Color color) {
    Point points[3] = {p1, p2, p3};
    draw.DrawPolygon(points, 3, color, color);
}

// Draw a dashed line
void DrawUtil::DrawDashedLine(Draw& draw, int x1, int y1, int x2, int y2, 
                             Color color, const Vector<int>& dash_pattern) {
    if (dash_pattern.IsEmpty()) {
        draw.DrawLine(x1, y1, x2, y2, 1, color);
        return;
    }
    
    // Calculate line length and direction
    int dx = x2 - x1;
    int dy = y2 - y1;
    double length = sqrt(dx * dx + dy * dy);
    if (length <= 0) return;
    
    double dir_x = dx / length;
    double dir_y = dy / length;
    
    // Draw dashed line
    double pos = 0;
    bool drawing = true;
    int pattern_index = 0;
    
    while (pos < length) {
        int segment_length = dash_pattern[pattern_index % dash_pattern.GetCount()];
        double end_pos = std::min(pos + segment_length, length);
        
        if (drawing) {
            int start_x = static_cast<int>(x1 + pos * dir_x);
            int start_y = static_cast<int>(y1 + pos * dir_y);
            int end_x = static_cast<int>(x1 + end_pos * dir_x);
            int end_y = static_cast<int>(y1 + end_pos * dir_y);
            draw.DrawLine(start_x, start_y, end_x, end_y, 1, color);
        }
        
        pos = end_pos;
        drawing = !drawing;
        pattern_index++;
    }
}

// Draw a gradient rectangle
void DrawUtil::DrawGradientRect(Draw& draw, const Rect& r, 
                               Color start_color, Color end_color, 
                               bool vertical) {
    if (r.IsEmpty()) return;
    
    int steps = vertical ? r.Height() : r.Width();
    if (steps <= 1) {
        draw.DrawRect(r, start_color);
        return;
    }
    
    for (int i = 0; i < steps; i++) {
        double ratio = static_cast<double>(i) / (steps - 1);
        int r_val = static_cast<int>(start_color.GetR() * (1 - ratio) + end_color.GetR() * ratio);
        int g_val = static_cast<int>(start_color.GetG() * (1 - ratio) + end_color.GetG() * ratio);
        int b_val = static_cast<int>(start_color.GetB() * (1 - ratio) + end_color.GetB() * ratio);
        int a_val = static_cast<int>(start_color.GetA() * (1 - ratio) + end_color.GetA() * ratio);
        
        Color blend_color(r_val, g_val, b_val, a_val);
        
        if (vertical) {
            draw.DrawLine(r.left, r.top + i, r.right, r.top + i, 1, blend_color);
        } else {
            draw.DrawLine(r.left + i, r.top, r.left + i, r.bottom, 1, blend_color);
        }
    }
}

// Draw text with shadow
void DrawUtil::DrawTextShadow(Draw& draw, int x, int y, const String& text, 
                             Font font, Color text_color, Color shadow_color, 
                             int offset_x, int offset_y) {
    // Draw shadow first
    draw.DrawText(x + offset_x, y + offset_y, text, font, shadow_color);
    // Draw text on top
    draw.DrawText(x, y, text, font, text_color);
}

// Draw text with outline
void DrawUtil::DrawTextOutline(Draw& draw, int x, int y, const String& text, 
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

// Draw text with glow effect
void DrawUtil::DrawTextGlow(Draw& draw, int x, int y, const String& text, 
                           Font font, Color text_color, Color glow_color, 
                           int glow_radius) {
    // Draw glow (simplified - in a real implementation, this would use blur or alpha blending)
    for (int dx = -glow_radius; dx <= glow_radius; dx++) {
        for (int dy = -glow_radius; dy <= glow_radius; dy++) {
            if (dx != 0 || dy != 0) {
                int alpha = static_cast<int>(255 * (glow_radius - std::max(abs(dx), abs(dy))) / glow_radius);
                Color faded_glow(glow_color.GetR(), glow_color.GetG(), glow_color.GetB(), alpha);
                draw.DrawText(x + dx, y + dy, text, font, faded_glow);
            }
        }
    }
    // Draw text on top
    draw.DrawText(x, y, text, font, text_color);
}

// Draw a frame around a rectangle
void DrawUtil::DrawFrame(Draw& draw, const Rect& r, Color color, int width) {
    if (width > 0) {
        draw.DrawRect(r.left, r.top, r.Width(), width, color); // Top
        draw.DrawRect(r.left, r.top, width, r.Height(), color); // Left
        draw.DrawRect(r.right - width, r.top, width, r.Height(), color); // Right
        draw.DrawRect(r.left, r.bottom - width, r.Width(), width, color); // Bottom
    }
}

// Draw a 3D frame (raised or sunken)
void DrawUtil::Draw3DFrame(Draw& draw, const Rect& r, bool raised, 
                          int width) {
    Color light = raised ? Color::White() : Color::Gray();
    Color dark = raised ? Color::Gray() : Color::Black();
    
    for (int i = 0; i < width; i++) {
        draw.DrawRect(r.left + i, r.top + i, r.Width() - 2*i, 1, light); // Top
        draw.DrawRect(r.left + i, r.top + i, 1, r.Height() - 2*i, light); // Left
        draw.DrawRect(r.right - 1 - i, r.top + i, 1, r.Height() - 2*i, dark); // Right
        draw.DrawRect(r.left + i, r.bottom - 1 - i, r.Width() - 2*i, 1, dark); // Bottom
    }
}

// Draw a focus rectangle
void DrawUtil::DrawFocusRect(Draw& draw, const Rect& r, 
                            Color color) {
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

// Draw a checkered pattern (useful for transparent backgrounds)
void DrawUtil::DrawCheckerboard(Draw& draw, const Rect& r, 
                               Color color1, Color color2, 
                               int size) {
    for (int y = r.top; y < r.bottom; y += size) {
        for (int x = r.left; x < r.right; x += size) {
            Color color = ((x / size + y / size) % 2 == 0) ? color1 : color2;
            draw.DrawRect(x, y, std::min(size, r.right - x), std::min(size, r.bottom - y), color);
        }
    }
}

// Calculate text size using the font
Size DrawUtil::GetTextSize(Draw& draw, const String& text, Font font) {
    // In a real implementation, this would use proper text measurement
    if (text.IsEmpty()) return Size(0, font.GetCy());
    
    // Simplified estimation
    int avg_char_width = font.GetCy() / 2; // Rough estimate
    return Size(text.GetLength() * avg_char_width, font.GetCy());
}

// Draw text with alignment within a rectangle
void DrawUtil::DrawAlignedText(Draw& draw, const Rect& r, const String& text, 
                              Font font, Color color, int align) {
    Size text_size = GetTextSize(draw, text, font);
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
    
    draw.DrawText(x, y, text, font, color);
}

// Clip drawing to a circular region
void DrawUtil::CircularClip(Draw& draw, int x, int y, int radius) {
    // In a real implementation, this would set a circular clipping region
    // For now, just use rectangular clipping as a placeholder
    draw.Clip(Rect(x - radius, y - radius, x + radius, y + radius));
}

// Draw an image with opacity
void DrawUtil::DrawImageOpacity(Draw& draw, int x, int y, const Image& img, 
                               int opacity) {
    if (img.IsEmpty() || opacity <= 0) return;
    
    if (opacity >= 255) {
        draw.DrawImage(x, y, img);
        return;
    }
    
    // In a real implementation, this would apply alpha blending
    // For now, just draw the image normally as a placeholder
    draw.DrawImage(x, y, img);
}

// Draw an image with scaling
void DrawUtil::DrawImageScaled(Draw& draw, const Rect& r, const Image& img) {
    if (img.IsEmpty() || r.IsEmpty()) return;
    
    draw.DrawImage(r.left, r.top, r.Width(), r.Height(), img);
}

// Draw an image with rotation
void DrawUtil::DrawImageRotated(Draw& draw, int x, int y, const Image& img, 
                               double angle) {
    if (img.IsEmpty()) return;
    
    // In a real implementation, this would apply rotation transform
    // For now, just draw the image normally as a placeholder
    draw.DrawImage(x, y, img);
}

// Path class implementation
Path::Path() : closed(false) {}

Path& Path::MoveTo(int x, int y) {
    points.Clear();
    points.Add(Point(x, y));
    return *this;
}

Path& Path::LineTo(int x, int y) {
    points.Add(Point(x, y));
    return *this;
}

Path& Path::LineTo(Point p) {
    points.Add(p);
    return *this;
}

Path& Path::AddArc(int x, int y, int radius, double start_angle, double sweep_angle) {
    // Simplified arc implementation - in a real implementation, this would calculate arc points
    int segments = static_cast<int>(abs(sweep_angle) * radius / 10); // Approximate number of segments
    segments = std::max(segments, 4);
    
    double angle_step = sweep_angle / segments;
    for (int i = 0; i <= segments; i++) {
        double angle = start_angle + i * angle_step;
        int px = static_cast<int>(x + radius * cos(angle));
        int py = static_cast<int>(y + radius * sin(angle));
        points.Add(Point(px, py));
    }
    return *this;
}

Path& Path::Close() {
    closed = true;
    return *this;
}

void Path::Draw(Draw& draw, Color color, int pen_width) {
    if (points.GetCount() < 2) return;
    
    draw.DrawPolyline(points.Begin(), points.GetCount(), pen_width, color);
    
    if (closed && points.GetCount() > 2) {
        // Connect last point to first point
        draw.DrawLine(points.Last(), points[0], pen_width, color);
    }
}

void Path::Fill(Draw& draw, Color color) {
    if (points.GetCount() < 3) return;
    
    draw.DrawPolygon(points.Begin(), points.GetCount(), color, color);
}

void Path::DrawFill(Draw& draw, Color outline_color, Color fill_color, int pen_width) {
    Fill(draw, fill_color);
    Draw(draw, outline_color, pen_width);
}

void Path::Clear() {
    points.Clear();
    closed = false;
}

// Gradient class implementation
Gradient::Gradient(Type t) : type(t), radius(0) {}

Gradient& Gradient::SetLinearPoints(Point start, Point end) {
    start_point = start;
    end_point = end;
    return *this;
}

Gradient& Gradient::SetRadialCenter(Point center, double rad) {
    center_point = center;
    radius = rad;
    return *this;
}

Gradient& Gradient::AddStop(double position, Color color) {
    stops.Add(Stop(position, color));
    // Keep stops sorted by position
    std::sort(stops.Begin(), stops.End(), [](const Stop& a, const Stop& b) {
        return a.position < b.position;
    });
    return *this;
}

void Gradient::ApplyToRect(Draw& draw, const Rect& r) {
    if (stops.IsEmpty()) return;
    
    if (type == LINEAR) {
        ApplyHorizontal(draw, r);
    } else {
        // Radial gradient implementation
        for (int y = r.top; y < r.bottom; y++) {
            for (int x = r.left; x < r.right; x++) {
                double dx = x - center_point.x;
                double dy = y - center_point.y;
                double distance = sqrt(dx * dx + dy * dy);
                double ratio = distance / radius;
                ratio = std::max(0.0, std::min(1.0, ratio));
                Color color = GetColorAt(ratio);
                draw.DrawRect(x, y, 1, 1, color);
            }
        }
    }
}

void Gradient::ApplyHorizontal(Draw& draw, const Rect& r) {
    if (stops.IsEmpty() || r.IsEmpty()) return;
    
    int steps = r.Width();
    for (int i = 0; i < steps; i++) {
        double ratio = static_cast<double>(i) / (steps - 1);
        Color color = GetColorAt(ratio);
        draw.DrawLine(r.left + i, r.top, r.left + i, r.bottom, 1, color);
    }
}

void Gradient::ApplyVertical(Draw& draw, const Rect& r) {
    if (stops.IsEmpty() || r.IsEmpty()) return;
    
    int steps = r.Height();
    for (int i = 0; i < steps; i++) {
        double ratio = static_cast<double>(i) / (steps - 1);
        Color color = GetColorAt(ratio);
        draw.DrawLine(r.left, r.top + i, r.right, r.top + i, 1, color);
    }
}

Color Gradient::GetColorAt(double position) const {
    if (stops.IsEmpty()) return Color::Black();
    
    if (position <= stops[0].position) return stops[0].color;
    if (position >= stops.Last().position) return stops.Last().color;
    
    // Find the two stops that bracket the position
    for (int i = 0; i < stops.GetCount() - 1; i++) {
        if (position >= stops[i].position && position <= stops[i + 1].position) {
            double ratio = (position - stops[i].position) / (stops[i + 1].position - stops[i].position);
            int r = static_cast<int>(stops[i].color.GetR() * (1 - ratio) + stops[i + 1].color.GetR() * ratio);
            int g = static_cast<int>(stops[i].color.GetG() * (1 - ratio) + stops[i + 1].color.GetG() * ratio);
            int b = static_cast<int>(stops[i].color.GetB() * (1 - ratio) + stops[i + 1].color.GetB() * ratio);
            int a = static_cast<int>(stops[i].color.GetA() * (1 - ratio) + stops[i + 1].color.GetA() * ratio);
            return Color(r, g, b, a);
        }
    }
    
    return stops[0].color; // Fallback
}

// GeometryUtil implementation
double GeometryUtil::Distance(Point p1, Point p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

double GeometryUtil::Angle(Point p1, Point p2) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    return atan2(dy, dx);
}

double GeometryUtil::LineAngle(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return atan2(dy, dx);
}

bool GeometryUtil::PointInRect(Point pt, const Rect& r) {
    return r.IsPtInside(pt);
}

bool GeometryUtil::PointInCircle(Point pt, int center_x, int center_y, int radius) {
    int dx = pt.x - center_x;
    int dy = pt.y - center_y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

Point GeometryUtil::RotatePoint(Point pt, Point center, double angle) {
    double cos_a = cos(angle);
    double sin_a = sin(angle);
    int dx = pt.x - center.x;
    int dy = pt.y - center.y;
    int rx = static_cast<int>(dx * cos_a - dy * sin_a + center.x);
    int ry = static_cast<int>(dx * sin_a + dy * cos_a + center.y);
    return Point(rx, ry);
}

Rect GeometryUtil::ScaleRectCenter(const Rect& r, double scale_x, double scale_y) {
    Point center = r.CenterPoint();
    int new_width = static_cast<int>(r.Width() * scale_x);
    int new_height = static_cast<int>(r.Height() * scale_y);
    return Rect(center.x - new_width / 2, center.y - new_height / 2, 
               center.x + new_width / 2, center.y + new_height / 2);
}

Rect GeometryUtil::GetBoundingRect(const Vector<Point>& points) {
    if (points.IsEmpty()) return Rect(0, 0, 0, 0);
    
    int min_x = points[0].x, max_x = points[0].x;
    int min_y = points[0].y, max_y = points[0].y;
    
    for (const auto& pt : points) {
        if (pt.x < min_x) min_x = pt.x;
        if (pt.x > max_x) max_x = pt.x;
        if (pt.y < min_y) min_y = pt.y;
        if (pt.y > max_y) max_y = pt.y;
    }
    
    return Rect(min_x, min_y, max_x, max_y);
}

Rect GeometryUtil::IntersectRect(const Rect& r1, const Rect& r2) {
    return r1 & r2;
}

Rect GeometryUtil::UnionRect(const Rect& r1, const Rect& r2) {
    return r1 | r2;
}

Point GeometryUtil::TransformPoint(Point pt, double matrix[6]) {
    // Apply affine transformation: [x'] = [a b c] [x]
    //                               [y']   [d e f] [y]
    //                                      [0 0 1] [1]
    double x = pt.x * matrix[0] + pt.y * matrix[1] + matrix[2];
    double y = pt.x * matrix[3] + pt.y * matrix[4] + matrix[5];
    return Point(static_cast<int>(x), static_cast<int>(y));
}

double GeometryUtil::PolygonArea(const Vector<Point>& points) {
    if (points.GetCount() < 3) return 0.0;
    
    double area = 0.0;
    int n = points.GetCount();
    
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += points[i].x * points[j].y;
        area -= points[j].x * points[i].y;
    }
    
    return abs(area) / 2.0;
}

bool GeometryUtil::IsConvexPolygon(const Vector<Point>& points) {
    if (points.GetCount() < 3) return false;
    
    bool sign = false;
    int n = points.GetCount();
    
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;
        
        int dx1 = points[j].x - points[i].x;
        int dy1 = points[j].y - points[i].y;
        int dx2 = points[k].x - points[j].x;
        int dy2 = points[k].y - points[j].y;
        
        int cross_product = dx1 * dy2 - dy1 * dx2;
        bool current_sign = cross_product > 0;
        
        if (i == 0) {
            sign = current_sign;
        } else if (sign != current_sign) {
            return false;
        }
    }
    
    return true;
}

}