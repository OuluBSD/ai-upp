// U++-compatible Size wrapper implemented using std::pair or direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

class Size {
public:
    int cx, cy;

    // Constructors
    Size() : cx(0), cy(0) {}
    Size(int cx_, int cy_) : cx(cx_), cy(cy_) {}

    // U++-style static constructors
    static Size Make(int cx, int cy) { return Size(cx, cy); }
    static Size Zero() { return Size(0, 0); }

    // Assignment operators
    Size& operator=(const Size& other) {
        if (this != &other) {
            cx = other.cx;
            cy = other.cy;
        }
        return *this;
    }

    // Arithmetic operators
    Size operator+(const Size& other) const { return Size(cx + other.cx, cy + other.cy); }
    Size& operator+=(const Size& other) { cx += other.cx; cy += other.cy; return *this; }
    Size operator-(const Size& other) const { return Size(cx - other.cx, cy - other.cy); }
    Size& operator-=(const Size& other) { cx -= other.cx; cy -= other.cy; return *this; }
    Size operator*(int scale) const { return Size(cx * scale, cy * scale); }
    Size& operator*=(int scale) { cx *= scale; cy *= scale; return *this; }
    Size operator/(int div) const { return Size(cx / div, cy / div); }
    Size& operator/=(int div) { cx /= div; cy /= div; return *this; }

    // Comparison operators
    bool operator==(const Size& other) const { return cx == other.cx && cy == other.cy; }
    bool operator!=(const Size& other) const { return !(*this == other); }

    // U++-style access methods
    int GetWidth() const { return cx; }
    int GetHeight() const { return cy; }
    void Set(int cx_, int cy_) { cx = cx_; cy = cy_; }
    void SetWidth(int cx_) { cx = cx_; }
    void SetHeight(int cy_) { cy = cy_; }

    // U++-style operations
    Size& Scale(int mul, int div) { 
        if (div != 0) {
            cx = (cx * mul) / div;
            cy = (cy * mul) / div;
        } else {
            cx = 0; cy = 0; // Handle division by zero
        }
        return *this;
    }

    // U++-style methods
    bool Is() const { return true; } // Sizes always "exist"
    bool IsZero() const { return cx == 0 && cy == 0; }
    bool IsEmpty() const { return cx <= 0 || cy <= 0; }
    Size Inverse() const { return Size(cy, cx); } // Swapped dimensions
    Size& Inverse() { std::swap(cx, cy); return *this; }

    // Area calculation
    int GetArea() const { return cx * cy; }
};