// STL-backed SystemDraw implementation for stdsrc/CtrlCore

#include "SystemDraw.h"

namespace Upp {

// BackDraw implementation
dword BackDraw::GetInfo() const {
    return painting ? painting->GetInfo() : 0;
}

void BackDraw::BeginOp() {
    if (painting) painting->BeginOp();
}

void BackDraw::EndOp() {
    if (painting) painting->EndOp();
}

void BackDraw::OffsetOp(Point p) {
    if (painting) painting->OffsetOp(p + painting_offset);
}

bool BackDraw::ClipOp(const Rect& r) {
    if (painting) return painting->ClipOp(r + painting_offset);
    return true;
}

bool BackDraw::ClipoffOp(const Rect& r) {
    if (painting) return painting->ClipoffOp(r + painting_offset);
    return true;
}

bool BackDraw::ExcludeClipOp(const Rect& r) {
    if (painting) return painting->ExcludeClipOp(r + painting_offset);
    return true;
}

bool BackDraw::IntersectClipOp(const Rect& r) {
    if (painting) return painting->IntersectClipOp(r + painting_offset);
    return true;
}

bool BackDraw::IsPaintingOp(const Rect& r) const {
    if (painting) return painting->IsPaintingOp(r + painting_offset);
    return true;
}

void BackDraw::DrawRectOp(int x, int y, int cx, int cy, Color color) {
    if (painting) painting->DrawRectOp(x + painting_offset.x, y + painting_offset.y, cx, cy, color);
}

void BackDraw::DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) {
    if (painting) painting->DrawImageOp(x + painting_offset.x, y + painting_offset.y, cx, cy, img, src, color);
}

void BackDraw::DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color) {
    if (painting) painting->DrawLineOp(x1 + painting_offset.x, y1 + painting_offset.y, 
                                       x2 + painting_offset.x, y2 + painting_offset.y, width, color);
}

void BackDraw::DrawPolyPolylineOp(const Point *vertices, int vertex_count,
                                 const int *counts, int count_count,
                                 int width, Color color, Color doxor) {
    if (painting) {
        // Transform vertices
        Buffer<Point> transformed(vertex_count);
        for (int i = 0; i < vertex_count; i++) {
            transformed[i] = vertices[i] + painting_offset;
        }
        painting->DrawPolyPolylineOp(transformed, vertex_count, counts, count_count, width, color, doxor);
    }
}

void BackDraw::DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
                                    const int *subpolygon_counts, int subpolygon_count_count,
                                    const int *disjunct_polygon_counts, int disjunct_polygon_count_count,
                                    Color color, int width, Color outline, uint64 pattern, Color doxor) {
    if (painting) {
        // Transform vertices
        Buffer<Point> transformed(vertex_count);
        for (int i = 0; i < vertex_count; i++) {
            transformed[i] = vertices[i] + painting_offset;
        }
        painting->DrawPolyPolyPolygonOp(transformed, vertex_count, 
                                       subpolygon_counts, subpolygon_count_count,
                                       disjunct_polygon_counts, disjunct_polygon_count_count,
                                       color, width, outline, pattern, doxor);
    }
}

void BackDraw::DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor) {
    if (painting) {
        Rect offset_r = r + painting_offset;
        painting->DrawEllipseOp(offset_r, color, pen, pencolor);
    }
}

void BackDraw::DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color) {
    if (painting) {
        Rect offset_rc = rc + painting_offset;
        Point offset_start = start + painting_offset;
        Point offset_end = end + painting_offset;
        painting->DrawArcOp(offset_rc, offset_start, offset_end, width, color);
    }
}

void BackDraw::DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
                          int n, const int *dx) {
    if (painting) {
        painting->DrawTextOp(x + painting_offset.x, y + painting_offset.y, 
                            angle, text, font, ink, n, dx);
    }
}

void BackDraw::DrawDrawingOp(const Rect& target, const Drawing& w) {
    if (painting) {
        Rect offset_target = target + painting_offset;
        painting->DrawDrawingOp(offset_target, w);
    }
}

void BackDraw::DrawPaintingOp(const Rect& target, const Painting& w) {
    if (painting) {
        Rect offset_target = target + painting_offset;
        painting->DrawPaintingOp(offset_target, w);
    }
}

void BackDraw::DrawDataOp(int x, int y, int cx, int cy, const String& data, const char *id) {
    if (painting) {
        painting->DrawDataOp(x + painting_offset.x, y + painting_offset.y, cx, cy, data, id);
    }
}

void BackDraw::Escape(const String& data) {
    if (painting) painting->Escape(data);
}

void BackDraw::Set(SystemDraw& w, Point offset) {
    painting = &w;
    painting_offset = offset;
    size = w.GetSize();
}

void BackDraw::Set(SystemDraw *w, Point offset) {
    painting = w;
    painting_offset = offset;
    size = w ? w->GetSize() : Size(0, 0);
}

void BackDraw::Destroy() {
    painting = NULL;
    painting_offset = Point(0, 0);
    size = Size(0, 0);
}

BackDraw::BackDraw() {
    painting = NULL;
    painting_offset = Point(0, 0);
    size = Size(0, 0);
}

BackDraw::~BackDraw() {
    // Nothing to destroy in this basic implementation
}

// ScreenInfo implementation
static SystemDraw s_screen_info;

SystemDraw& ScreenInfo() {
    return s_screen_info;
}

// SetSurface implementation
void SetSurface(Draw& w, const Rect& dest, const RGBA *pixels, Size srcsz, Point poff) {
    // In a real implementation, this would set surface data directly to the drawing context
    // For now, this is a placeholder that could be implemented with platform-specific code
}

void SetSurface(Draw& w, int x, int y, int cx, int cy, const RGBA *pixels) {
    SetSurface(w, Rect(x, y, x + cx, y + cy), pixels, Size(cx, cy), Point(0, 0));
}

}