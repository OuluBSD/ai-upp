#ifndef _Draw_DrawUtil_h_
#define _Draw_DrawUtil_h_

#include "Draw.h"
#include "Image.h"
#include "Font.h"
#include <algorithm>
#include <cmath>

// Drawing utilities and helper functions
class DrawUtil {
public:
    // Draw a rounded rectangle
    static void DrawRoundedRect(Draw& draw, const Rect& r, int radius, 
                               Color color, int pen_width = 1);
    
    // Fill a rounded rectangle
    static void FillRoundedRect(Draw& draw, const Rect& r, int radius, Color color);
    
    // Draw a circle
    static void DrawCircle(Draw& draw, int x, int y, int radius, 
                          Color color, int pen_width = 1);
    
    // Fill a circle
    static void FillCircle(Draw& draw, int x, int y, int radius, Color color);
    
    // Draw an ellipse
    static void DrawEllipse(Draw& draw, const Rect& r, Color color, int pen_width = 1);
    
    // Fill an ellipse
    static void FillEllipse(Draw& draw, const Rect& r, Color color);
    
    // Draw a line with anti-aliasing
    static void DrawLine(Draw& draw, int x1, int y1, int x2, int y2, 
                        Color color, int pen_width = 1);
    
    // Draw a polygon
    static void DrawPolygon(Draw& draw, const Vector<Point>& points, 
                           Color color, int pen_width = 1);
    
    // Fill a polygon
    static void FillPolygon(Draw& draw, const Vector<Point>& points, Color color);
    
    // Draw a triangle
    static void DrawTriangle(Draw& draw, Point p1, Point p2, Point p3, 
                            Color color, int pen_width = 1);
    
    // Fill a triangle
    static void FillTriangle(Draw& draw, Point p1, Point p2, Point p3, Color color);
    
    // Draw a dashed line
    static void DrawDashedLine(Draw& draw, int x1, int y1, int x2, int y2, 
                              Color color, const Vector<int>& dash_pattern = Vector<int>({4, 4}));
    
    // Draw a gradient rectangle
    static void DrawGradientRect(Draw& draw, const Rect& r, 
                                Color start_color, Color end_color, 
                                bool vertical = true);
    
    // Draw text with shadow
    static void DrawTextShadow(Draw& draw, int x, int y, const String& text, 
                              Font font, Color text_color, Color shadow_color, 
                              int offset_x = 1, int offset_y = 1);
    
    // Draw text with outline
    static void DrawTextOutline(Draw& draw, int x, int y, const String& text, 
                               Font font, Color text_color, Color outline_color, 
                               int outline_width = 1);
    
    // Draw text with glow effect
    static void DrawTextGlow(Draw& draw, int x, int y, const String& text, 
                            Font font, Color text_color, Color glow_color, 
                            int glow_radius = 3);
    
    // Draw a frame around a rectangle
    static void DrawFrame(Draw& draw, const Rect& r, Color color, int width = 1);
    
    // Draw a 3D frame (raised or sunken)
    static void Draw3DFrame(Draw& draw, const Rect& r, bool raised = true, 
                           int width = 1);
    
    // Draw a focus rectangle
    static void DrawFocusRect(Draw& draw, const Rect& r, 
                             Color color = Color::Blue());
    
    // Draw a checkered pattern (useful for transparent backgrounds)
    static void DrawCheckerboard(Draw& draw, const Rect& r, 
                                Color color1 = Color::White(), 
                                Color color2 = Color::LtGray(), 
                                int size = 8);
    
    // Calculate text size using the font
    static Size GetTextSize(Draw& draw, const String& text, Font font);
    
    // Draw text with alignment within a rectangle
    static void DrawAlignedText(Draw& draw, const Rect& r, const String& text, 
                               Font font, Color color, int align = ALIGN_LEFT);
    
    // Clip drawing to a circular region
    static void CircularClip(Draw& draw, int x, int y, int radius);
    
    // Draw an image with opacity
    static void DrawImageOpacity(Draw& draw, int x, int y, const Image& img, 
                                int opacity = 255); // 0 = transparent, 255 = opaque
    
    // Draw an image with scaling
    static void DrawImageScaled(Draw& draw, const Rect& r, const Image& img);
    
    // Draw an image with rotation
    static void DrawImageRotated(Draw& draw, int x, int y, const Image& img, 
                                double angle); // angle in radians
};

// Utility class for path drawing
class Path {
private:
    Vector<Point> points;
    bool closed;
    
public:
    Path();
    
    Path& MoveTo(int x, int y);
    Path& LineTo(int x, int y);
    Path& LineTo(Point p);
    Path& AddArc(int x, int y, int radius, double start_angle, double sweep_angle);
    Path& Close();
    
    void Draw(Draw& draw, Color color, int pen_width = 1);
    void Fill(Draw& draw, Color color);
    void DrawFill(Draw& draw, Color outline_color, Color fill_color, int pen_width = 1);
    
    const Vector<Point>& GetPoints() const { return points; }
    bool IsClosed() const { return closed; }
    void Clear();
};

// Utility class for gradients
class Gradient {
public:
    enum Type {
        LINEAR,
        RADIAL
    };
    
    struct Stop {
        double position; // 0.0 to 1.0
        Color color;
        
        Stop(double pos, Color c) : position(pos), color(c) {}
    };
    
private:
    Type type;
    Vector<Stop> stops;
    Point start_point, end_point;
    Point center_point;
    double radius;
    
public:
    Gradient(Type t = LINEAR);
    
    // For linear gradients
    Gradient& SetLinearPoints(Point start, Point end);
    
    // For radial gradients
    Gradient& SetRadialCenter(Point center, double rad);
    
    // Add color stops
    Gradient& AddStop(double position, Color color);
    
    // Apply gradient to a rectangle
    void ApplyToRect(Draw& draw, const Rect& r);
    
    // Apply gradient horizontally across a rectangle
    void ApplyHorizontal(Draw& draw, const Rect& r);
    
    // Apply gradient vertically across a rectangle
    void ApplyVertical(Draw& draw, const Rect& r);
    
    // Get color at a specific position in the gradient
    Color GetColorAt(double position) const;
    
    Type GetType() const { return type; }
    const Vector<Stop>& GetStops() const { return stops; }
};

// Utility functions for geometry calculations
class GeometryUtil {
public:
    // Calculate distance between two points
    static double Distance(Point p1, Point p2);
    
    // Calculate angle between two points (in radians)
    static double Angle(Point p1, Point p2);
    
    // Calculate angle of a line (in radians)
    static double LineAngle(int x1, int y1, int x2, int y2);
    
    // Check if point is inside rectangle
    static bool PointInRect(Point pt, const Rect& r);
    
    // Check if point is inside circle
    static bool PointInCircle(Point pt, int center_x, int center_y, int radius);
    
    // Rotate a point around a center
    static Point RotatePoint(Point pt, Point center, double angle); // angle in radians
    
    // Scale a rectangle around its center
    static Rect ScaleRectCenter(const Rect& r, double scale_x, double scale_y);
    
    // Get bounding rectangle for a set of points
    static Rect GetBoundingRect(const Vector<Point>& points);
    
    // Calculate intersection of two rectangles
    static Rect IntersectRect(const Rect& r1, const Rect& r2);
    
    // Calculate union of two rectangles
    static Rect UnionRect(const Rect& r1, const Rect& r2);
    
    // Transform a point using a transformation matrix
    static Point TransformPoint(Point pt, double matrix[6]); // Affine transformation matrix
    
    // Calculate area of a polygon using the shoelace formula
    static double PolygonArea(const Vector<Point>& points);
    
    // Check if a polygon is convex
    static bool IsConvexPolygon(const Vector<Point>& points);
};

#endif