#pragma once
// U++-compatible Draw wrapper for graphics drawing operations
// This header is aggregated and wrapped into namespace Upp by Draw.h

#include <cstdint>  // For uint8_t, uint32_t
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

// Include the actual definitions instead of forward declarations
#include "Point.h"
#include "Size.h"
#include "Rect.h"
#include "Color.h"
#include "Image.h"
#include "Font.h"

class Draw {
public:
    // Virtual destructor for proper inheritance
    virtual ~Draw() = default;

    // U++-style drawing methods that can be implemented using various backends
    virtual void DrawPoint(const Point& p, const Color& color) = 0;
    virtual void DrawPoint(int x, int y, const Color& color) { DrawPoint(Point(x, y), color); }

    virtual void DrawLine(const Point& p1, const Point& p2, const Color& color, int width = 1) = 0;
    virtual void DrawLine(int x1, int y1, int x2, int y2, const Color& color, int width = 1) { 
        DrawLine(Point(x1, y1), Point(x2, y2), color, width); 
    }

    virtual void DrawRect(const Rect& r, const Color& color) = 0;
    virtual void DrawRect(int x, int y, int cx, int cy, const Color& color) { 
        DrawRect(Rect(x, y, x + cx, y + cy), color); 
    }

    virtual void DrawRect(const Rect& r, const Color& outline, const Color& fill) = 0;
    virtual void DrawRect(int x, int y, int cx, int cy, const Color& outline, const Color& fill) { 
        DrawRect(Rect(x, y, x + cx, y + cy), outline, fill); 
    }

    virtual void DrawEllipse(const Rect& r, const Color& color) = 0;
    virtual void DrawEllipse(int x, int y, int cx, int cy, const Color& color) { 
        DrawEllipse(Rect(x, y, x + cx, y + cy), color); 
    }

    virtual void DrawEllipse(const Rect& r, const Color& outline, const Color& fill) = 0;
    virtual void DrawEllipse(int x, int y, int cx, int cy, const Color& outline, const Color& fill) { 
        DrawEllipse(Rect(x, y, x + cx, y + cy), outline, fill); 
    }

    virtual void DrawPolygon(const Point* vertices, int count, const Color& color) = 0;
    virtual void DrawPolygon(const Point* vertices, int count, const Color& outline, const Color& fill) = 0;

    virtual void DrawPolyline(const Point* vertices, int count, const Color& color, int width = 1) = 0;

    virtual void DrawImage(const Point& p, const Image& img) = 0;
    virtual void DrawImage(int x, int y, const Image& img) { DrawImage(Point(x, y), img); }
    virtual void DrawImage(const Point& p, const Image& img, const Rect& src) = 0;
    virtual void DrawImage(int x, int y, const Image& img, const Rect& src) { 
        DrawImage(Point(x, y), img, src); 
    }
    virtual void DrawImage(const Rect& r, const Image& img) = 0;
    virtual void DrawImage(int x, int y, int cx, int cy, const Image& img) { 
        DrawImage(Rect(x, y, x + cx, y + cy), img); 
    }

    // U++-style text drawing (requires font support)
    virtual void DrawText(int x, int y, const std::string& text, const Font& font, const Color& color) = 0;
    virtual void DrawText(const Point& p, const std::string& text, const Font& font, const Color& color) { 
        DrawText(p.x, p.y, text, font, color); 
    }

    // U++-style drawing state operations
    virtual void Clip(const Rect& r) = 0;
    virtual void OffsetClip(const Point& offset) = 0;
    virtual void ResetClip() = 0;
    virtual Rect GetClip() const = 0;

    virtual void OffsetOp(const Point& offset) = 0;
    virtual Point GetOffset() const = 0;

    // U++-style surface operations
    virtual Size GetSize() const = 0;
    virtual Rect GetPaintRect() const = 0;

    // U++-style Begin/End operations for drawing contexts
    virtual void Begin() {}
    virtual void End() {}

    // U++-style drawing capabilities inquiry
    virtual bool IsPainting() const { return true; }
    virtual bool IsRectObscured(const Rect& r) const { return false; }
    virtual bool IsRectFullyPainted(const Rect& r) const { return true; }

    // U++-style Draw implementation identification
    virtual const char* GetInfo() const { return "stdsrc::Draw"; }
};

// Concrete implementation using native drawing APIs could be provided
// This is a placeholder implementation showing the interface
class StdDraw : public Draw {
private:
    Rect paintRect;
    Point offset;
    
public:
    explicit StdDraw(const Size& sz) : paintRect(0, 0, sz.cx, sz.cy), offset(0, 0) {}
    explicit StdDraw(int width, int height) : StdDraw(Size(width, height)) {}

    // Implementation of Draw interface methods
    void DrawPoint(const Point& p, const Color& color) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawLine(const Point& p1, const Point& p2, const Color& color, int width = 1) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawRect(const Rect& r, const Color& color) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawRect(const Rect& r, const Color& outline, const Color& fill) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawEllipse(const Rect& r, const Color& color) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawEllipse(const Rect& r, const Color& outline, const Color& fill) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawPolygon(const Point* vertices, int count, const Color& color) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawPolygon(const Point* vertices, int count, const Color& outline, const Color& fill) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawPolyline(const Point* vertices, int count, const Color& color, int width = 1) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawImage(const Point& p, const Image& img) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawImage(const Point& p, const Image& img, const Rect& src) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawImage(const Rect& r, const Image& img) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void DrawText(int x, int y, const std::string& text, const Font& font, const Color& color) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void Clip(const Rect& r) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void OffsetClip(const Point& offset) override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    void ResetClip() override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
    }

    Rect GetClip() const override {
        // Implementation would use native drawing system
        // Placeholder for actual drawing implementation
        return Rect();
    }

    void OffsetOp(const Point& offset) override {
        this->offset = this->offset + offset;
    }

    Point GetOffset() const override {
        return offset;
    }

    Size GetSize() const override {
        return paintRect.GetSize();
    }

    Rect GetPaintRect() const override {
        return paintRect;
    }
};