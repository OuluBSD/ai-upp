// U++-compatible Rect wrapper implemented using direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

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
    static Rect Make(int left, int top, int right, int bottom) { return Rect(left, top, right, bottom); }
    static Rect MakeP(int x, int y, int width, int height) { return Rect(x, y, x + width, y + height); }
    static Rect MakeSize(const Point& pt, const Size& sz) { return Rect(pt, sz); }
    static Rect MakeWH(int x, int y, int width, int height) { return Rect(x, y, x + width, y + height); }
    static Rect Zero() { return Rect(0, 0, 0, 0); }

    // Assignment operators
    Rect& operator=(const Rect& other) {
        if (this != &other) {
            left = other.left; top = other.top; right = other.right; bottom = other.bottom;
        }
        return *this;
    }

    // Comparison operators
    bool operator==(const Rect& other) const { 
        return left == other.left && top == other.top && 
               right == other.right && bottom == other.bottom; 
    }
    bool operator!=(const Rect& other) const { return !(*this == other); }

    // U++-style access methods
    int GetLeft() const { return left; }
    int GetTop() const { return top; }
    int GetRight() const { return right; }
    int GetBottom() const { return bottom; }
    int GetWidth() const { return right - left; }
    int GetHeight() const { return bottom - top; }
    
    Point GetTopLeft() const { return Point(left, top); }
    Point GetTopRight() const { return Point(right, top); }
    Point GetBottomLeft() const { return Point(left, bottom); }
    Point GetBottomRight() const { return Point(right, bottom); }
    Point GetCenter() const { return Point((left + right) / 2, (top + bottom) / 2); }
    
    Size GetSize() const { return Size(right - left, bottom - top); }

    void Set(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    void SetTopLeft(const Point& pt) { left = pt.x; top = pt.y; }
    void SetTopRight(const Point& pt) { right = pt.x; top = pt.y; }
    void SetBottomLeft(const Point& pt) { left = pt.x; bottom = pt.y; }
    void SetBottomRight(const Point& pt) { right = pt.x; bottom = pt.y; }
    void SetSize(const Size& sz) { right = left + sz.cx; bottom = top + sz.cy; }
    void SetRect(const Point& pt, const Size& sz) { *this = Rect(pt, sz); }

    // U++-style operations
    Rect& Move(int dx, int dy) { left += dx; top += dy; right += dx; bottom += dy; return *this; }
    Rect& Move(const Point& delta) { return Move(delta.x, delta.y); }
    Rect& MoveLeft(int d) { left += d; right += d; return *this; }
    Rect& MoveTop(int d) { top += d; bottom += d; return *this; }
    Rect& MoveRight(int d) { right += d; return *this; }
    Rect& MoveBottom(int d) { bottom += d; return *this; }
    Rect& MoveX(int d) { left += d; right += d; return *this; }
    Rect& MoveY(int d) { top += d; bottom += d; return *this; }

    // U++-style size operations
    Rect& MoveLeftTo(int x) { int w = GetWidth(); left = x; right = x + w; return *this; }
    Rect& MoveTopTo(int y) { int h = GetHeight(); top = y; bottom = y + h; return *this; }
    Rect& MoveRightTo(int x) { int w = GetWidth(); right = x; left = x - w; return *this; }
    Rect& MoveBottomTo(int y) { int h = GetHeight(); bottom = y; top = y - h; return *this; }
    Rect& MoveCenterTo(const Point& pt) { 
        Point c = GetCenter();
        int dx = pt.x - c.x;
        int dy = pt.y - c.y;
        return Move(dx, dy);
    }

    // U++-style size operations
    Rect& SetWidth(int width) { right = left + width; return *this; }
    Rect& SetHeight(int height) { bottom = top + height; return *this; }
    Rect& SetSize(const Size& sz) { right = left + sz.cx; bottom = top + sz.cy; return *this; }

    // U++-style operations
    bool Is() const { return left < right && top < bottom; }
    bool IsNull() const { return left == 0 && top == 0 && right == 0 && bottom == 0; }
    bool IsEmpty() const { return left >= right || top >= bottom; }
    bool IsPtInside(const Point& pt) const { 
        return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom; 
    }
    bool IsRectInside(const Rect& r) const {
        return r.left >= left && r.right <= right && r.top >= top && r.bottom <= bottom;
    }

    // U++-style union/intersection
    Rect GetUnion(const Rect& r) const {
        if (IsEmpty()) return r;
        if (r.IsEmpty()) return *this;
        return Rect(
            std::min(left, r.left),
            std::min(top, r.top),
            std::max(right, r.right),
            std::max(bottom, r.bottom)
        );
    }

    Rect GetIntersection(const Rect& r) const {
        Rect result(
            std::max(left, r.left),
            std::max(top, r.top),
            std::min(right, r.right),
            std::min(bottom, r.bottom)
        );
        return result.IsEmpty() ? Rect() : result;
    }

    bool IsIntersects(const Rect& r) const {
        return !GetIntersection(r).IsEmpty();
    }

    Rect& Clip(const Rect& r) {
        *this = GetIntersection(r);
        return *this;
    }

    // U++-style offset operations
    Rect& Offset(int dx, int dy) { return Move(dx, dy); }
    Rect& Offset(const Point& delta) { return Move(delta.x, delta.y); }

    // U++-style expansion
    Rect& Inflate(int dx, int dy) { 
        left -= dx; top -= dy; right += dx; bottom += dy; 
        return *this;
    }
    Rect& Inflate(const Point& delta) { return Inflate(delta.x, delta.y); }
    Rect& InflateRect(int l, int t, int r, int b) {
        left -= l; top -= t; right += r; bottom += b;
        return *this;
    }
};