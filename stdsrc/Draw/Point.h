// U++-compatible Point wrapper implemented using std::pair or direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

class Point {
public:
    int x, y;

    // Constructors
    Point() : x(0), y(0) {}
    Point(int x_, int y_) : x(x_), y(y_) {}

    // U++-style static constructors
    static Point Make(int x, int y) { return Point(x, y); }
    static Point Zero() { return Point(0, 0); }

    // Assignment operators
    Point& operator=(const Point& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
        }
        return *this;
    }

    // Arithmetic operators
    Point operator+(const Point& other) const { return Point(x + other.x, y + other.y); }
    Point& operator+=(const Point& other) { x += other.x; y += other.y; return *this; }
    Point operator-(const Point& other) const { return Point(x - other.x, y - other.y); }
    Point& operator-=(const Point& other) { x -= other.x; y -= other.y; return *this; }
    Point operator*(int scale) const { return Point(x * scale, y * scale); }
    Point& operator*=(int scale) { x *= scale; y *= scale; return *this; }
    Point operator/(int div) const { return Point(x / div, y / div); }
    Point& operator/=(int div) { x /= div; y /= div; return *this; }

    // Comparison operators
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }

    // U++-style access methods
    int GetX() const { return x; }
    int GetY() const { return y; }
    void Set(int x_, int y_) { x = x_; y = y_; }
    void SetX(int x_) { x = x_; }
    void SetY(int y_) { y = y_; }

    // U++-style operations
    Point& Offset(int dx, int dy) { x += dx; y += dy; return *this; }
    Point& Offset(const Point& delta) { x += delta.x; y += delta.y; return *this; }
    Point& MulDiv(int mul, int div) { 
        if (div != 0) {
            x = (x * mul) / div;
            y = (y * mul) / div;
        } else {
            x = 0; y = 0; // Handle division by zero
        }
        return *this;
    }

    // U++-style methods
    bool Is() const { return true; } // Points always "exist"
    bool IsZero() const { return x == 0 && y == 0; }
    Point Neg() const { return Point(-x, -y); }
    int GetDistance(const Point& other) const { 
        int dx = x - other.x;
        int dy = y - other.y;
        return static_cast<int>(std::sqrt(dx*dx + dy*dy));
    }
};