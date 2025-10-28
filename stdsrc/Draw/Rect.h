#pragma once
// U++-compatible Rect wrapper implemented using direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

#include <algorithm>

class Rect {
public:
    int left, top, right, bottom;

    // Constructors
    Rect() : left(0), top(0), right(0), bottom(0) {}
    Rect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    Rect(const Point& topLeft, const Size& size) 
        : left(topLeft.x), top(topLeft.y), 
          right(topLeft.x + size.cx), bottom(topLeft.y + size.cy) {}
    Rect(const Point& pt1, const Point& pt2) 
        : left(std::min(pt1.x, pt2.x)), top(std::min(pt1.y, pt2.y)),
          right(std::max(pt1.x, pt2.x)), bottom(std::max(pt1.y, pt2.y)) {}

    // U++-style static constructors
    static Rect Make(int left, int top, int right, int bottom);
    static Rect MakeP(int x, int y, int width, int height);
    static Rect MakeSize(const Point& pt, const Size& sz);
    static Rect MakeWH(int x, int y, int width, int height);
    static Rect Zero();

    // Assignment operators
    Rect& operator=(const Rect& other);

    // Comparison operators
    bool operator==(const Rect& other) const;
    bool operator!=(const Rect& other) const;

    // U++-style access methods
    int GetLeft() const;
    int GetTop() const;
    int GetRight() const;
    int GetBottom() const;
    int GetWidth() const;
    int GetHeight() const;
    
    Point GetTopLeft() const;
    Point GetTopRight() const;
    Point GetBottomLeft() const;
    Point GetBottomRight() const;
    Point GetCenter() const;
    
    Size GetSize() const;

    void Set(int l, int t, int r, int b);
    void SetTopLeft(const Point& pt);
    void SetTopRight(const Point& pt);
    void SetBottomLeft(const Point& pt);
    void SetBottomRight(const Point& pt);
    void SetRect(const Point& pt, const Size& sz);

    // U++-style operations
    Rect& Move(int dx, int dy);
    Rect& Move(const Point& delta);
    Rect& MoveLeft(int d);
    Rect& MoveTop(int d);
    Rect& MoveRight(int d);
    Rect& MoveBottom(int d);
    Rect& MoveX(int d);
    Rect& MoveY(int d);

    // U++-style size operations
    Rect& MoveLeftTo(int x);
    Rect& MoveTopTo(int y);
    Rect& MoveRightTo(int x);
    Rect& MoveBottomTo(int y);
    Rect& MoveCenterTo(const Point& pt);

    // U++-style size operations
    Rect& SetWidth(int width);
    Rect& SetHeight(int height);
    Rect& SetSize(const Size& sz);

    // U++-style operations
    bool Is() const;
    bool IsNull() const;
    bool IsEmpty() const;
    bool IsPtInside(const Point& pt) const;
    bool IsRectInside(const Rect& r) const;

    // U++-style union/intersection
    Rect GetUnion(const Rect& r) const;

    Rect GetIntersection(const Rect& r) const;

    bool IsIntersects(const Rect& r) const;

    Rect& Clip(const Rect& r);

    // U++-style offset operations
    Rect& Offset(int dx, int dy);
    Rect& Offset(const Point& delta);

    // U++-style expansion
    Rect& Inflate(int dx, int dy);
    Rect& Inflate(const Point& delta);
    Rect& InflateRect(int l, int t, int r, int b);
};