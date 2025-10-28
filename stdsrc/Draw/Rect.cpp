// STL-backed Draw API implementation

#include "Rect.h"
#include "Point.h"
#include "Size.h"
#include <algorithm>

namespace Upp {

// U++-style static constructors
Rect Rect::Make(int left, int top, int right, int bottom) { 
    return Rect(left, top, right, bottom); 
}
Rect Rect::MakeP(int x, int y, int width, int height) { 
    return Rect(x, y, x + width, y + height); 
}
Rect Rect::MakeSize(const Point& pt, const Size& sz) { 
    return Rect(pt, sz); 
}
Rect Rect::MakeWH(int x, int y, int width, int height) { 
    return Rect(x, y, x + width, y + height); 
}
Rect Rect::Zero() { 
    return Rect(0, 0, 0, 0); 
}

// Assignment operators
Rect& Rect::operator=(const Rect& other) {
    if (this != &other) {
        left = other.left; top = other.top; right = other.right; bottom = other.bottom;
    }
    return *this;
}

// Comparison operators
bool Rect::operator==(const Rect& other) const { 
    return left == other.left && top == other.top && 
           right == other.right && bottom == other.bottom; 
}
bool Rect::operator!=(const Rect& other) const { 
    return !(*this == other); 
}

// U++-style access methods
int Rect::GetLeft() const { 
    return left; 
}
int Rect::GetTop() const { 
    return top; 
}
int Rect::GetRight() const { 
    return right; 
}
int Rect::GetBottom() const { 
    return bottom; 
}
int Rect::GetWidth() const { 
    return right - left; 
}
int Rect::GetHeight() const { 
    return bottom - top; 
}

Point Rect::GetTopLeft() const { 
    return Point(left, top); 
}
Point Rect::GetTopRight() const { 
    return Point(right, top); 
}
Point Rect::GetBottomLeft() const { 
    return Point(left, bottom); 
}
Point Rect::GetBottomRight() const { 
    return Point(right, bottom); 
}
Point Rect::GetCenter() const { 
    return Point((left + right) / 2, (top + bottom) / 2); 
}

Size Rect::GetSize() const { 
    return Size(right - left, bottom - top); 
}

void Rect::Set(int l, int t, int r, int b) { 
    left = l; top = t; right = r; bottom = b; 
}
void Rect::SetTopLeft(const Point& pt) { 
    left = pt.x; top = pt.y; 
}
void Rect::SetTopRight(const Point& pt) { 
    right = pt.x; top = pt.y; 
}
void Rect::SetBottomLeft(const Point& pt) { 
    left = pt.x; bottom = pt.y; 
}
void Rect::SetBottomRight(const Point& pt) { 
    right = pt.x; bottom = pt.y; 
}
void Rect::SetSize(const Size& sz) { 
    right = left + sz.cx; bottom = top + sz.cy; 
}
void Rect::SetRect(const Point& pt, const Size& sz) { 
    *this = Rect(pt, sz); 
}

// U++-style operations
Rect& Rect::Move(int dx, int dy) { 
    left += dx; top += dy; right += dx; bottom += dy; 
    return *this; 
}
Rect& Rect::Move(const Point& delta) { 
    return Move(delta.x, delta.y); 
}
Rect& Rect::MoveLeft(int d) { 
    left += d; right += d; 
    return *this; 
}
Rect& Rect::MoveTop(int d) { 
    top += d; bottom += d; 
    return *this; 
}
Rect& Rect::MoveRight(int d) { 
    right += d; 
    return *this; 
}
Rect& Rect::MoveBottom(int d) { 
    bottom += d; 
    return *this; 
}
Rect& Rect::MoveX(int d) { 
    left += d; right += d; 
    return *this; 
}
Rect& Rect::MoveY(int d) { 
    top += d; bottom += d; 
    return *this; 
}

// U++-style size operations
Rect& Rect::MoveLeftTo(int x) { 
    int w = GetWidth(); 
    left = x; right = x + w; 
    return *this; 
}
Rect& Rect::MoveTopTo(int y) { 
    int h = GetHeight(); 
    top = y; bottom = y + h; 
    return *this; 
}
Rect& Rect::MoveRightTo(int x) { 
    int w = GetWidth(); 
    right = x; left = x - w; 
    return *this; 
}
Rect& Rect::MoveBottomTo(int y) { 
    int h = GetHeight(); 
    bottom = y; top = y - h; 
    return *this; 
}
Rect& Rect::MoveCenterTo(const Point& pt) { 
    Point c = GetCenter();
    int dx = pt.x - c.x;
    int dy = pt.y - c.y;
    return Move(dx, dy);
}

// U++-style size operations
Rect& Rect::SetWidth(int width) { 
    right = left + width; 
    return *this; 
}
Rect& Rect::SetHeight(int height) { 
    bottom = top + height; 
    return *this; 
}
Rect& Rect::SetSize(const Size& sz) { 
    right = left + sz.cx; bottom = top + sz.cy; 
    return *this; 
}

// U++-style operations
bool Rect::Is() const { 
    return left < right && top < bottom; 
}
bool Rect::IsNull() const { 
    return left == 0 && top == 0 && right == 0 && bottom == 0; 
}
bool Rect::IsEmpty() const { 
    return left >= right || top >= bottom; 
}
bool Rect::IsPtInside(const Point& pt) const { 
    return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom; 
}
bool Rect::IsRectInside(const Rect& r) const {
    return r.left >= left && r.right <= right && r.top >= top && r.bottom <= bottom;
}

// U++-style union/intersection
Rect Rect::GetUnion(const Rect& r) const {
    if (IsEmpty()) return r;
    if (r.IsEmpty()) return *this;
    return Rect(
        std::min(left, r.left),
        std::min(top, r.top),
        std::max(right, r.right),
        std::max(bottom, r.bottom)
    );
}

Rect Rect::GetIntersection(const Rect& r) const {
    Rect result(
        std::max(left, r.left),
        std::max(top, r.top),
        std::min(right, r.right),
        std::min(bottom, r.bottom)
    );
    return result.IsEmpty() ? Rect() : result;
}

bool Rect::IsIntersects(const Rect& r) const {
    return !GetIntersection(r).IsEmpty();
}

Rect& Rect::Clip(const Rect& r) {
    *this = GetIntersection(r);
    return *this;
}

// U++-style offset operations
Rect& Rect::Offset(int dx, int dy) { 
    return Move(dx, dy); 
}
Rect& Rect::Offset(const Point& delta) { 
    return Move(delta.x, delta.y); 
}

// U++-style expansion
Rect& Rect::Inflate(int dx, int dy) { 
    left -= dx; top -= dy; right += dx; bottom += dy; 
    return *this;
}
Rect& Rect::Inflate(const Point& delta) { 
    return Inflate(delta.x, delta.y); 
}
Rect& Rect::InflateRect(int l, int t, int r, int b) {
    left -= l; top -= t; right += r; bottom += b;
    return *this;
}

}