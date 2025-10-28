// STL-backed Draw API implementation

#include "Point.h"
#include <cmath>

namespace Upp {

// U++-style static constructors
Point Point::Make(int x, int y) { 
    return Point(x, y); 
}
Point Point::Zero() { 
    return Point(0, 0); 
}

// Assignment operators
Point& Point::operator=(const Point& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}

// Arithmetic operators
Point Point::operator+(const Point& other) const { 
    return Point(x + other.x, y + other.y); 
}
Point& Point::operator+=(const Point& other) { 
    x += other.x; y += other.y; 
    return *this; 
}
Point Point::operator-(const Point& other) const { 
    return Point(x - other.x, y - other.y); 
}
Point& Point::operator-=(const Point& other) { 
    x -= other.x; y -= other.y; 
    return *this; 
}
Point Point::operator*(int scale) const { 
    return Point(x * scale, y * scale); 
}
Point& Point::operator*=(int scale) { 
    x *= scale; y *= scale; 
    return *this; 
}
Point Point::operator/(int div) const { 
    return Point(x / div, y / div); 
}
Point& Point::operator/=(int div) { 
    x /= div; y /= div; 
    return *this; 
}

// Comparison operators
bool Point::operator==(const Point& other) const { 
    return x == other.x && y == other.y; 
}
bool Point::operator!=(const Point& other) const { 
    return !(*this == other); 
}

// U++-style access methods
int Point::GetX() const { 
    return x; 
}
int Point::GetY() const { 
    return y; 
}
void Point::Set(int x_, int y_) { 
    x = x_; y = y_; 
}
void Point::SetX(int x_) { 
    x = x_; 
}
void Point::SetY(int y_) { 
    y = y_; 
}

// U++-style operations
Point& Point::Offset(int dx, int dy) { 
    x += dx; y += dy; 
    return *this; 
}
Point& Point::Offset(const Point& delta) { 
    x += delta.x; y += delta.y; 
    return *this; 
}
Point& Point::MulDiv(int mul, int div) { 
    if (div != 0) {
        x = (x * mul) / div;
        y = (y * mul) / div;
    } else {
        x = 0; y = 0; // Handle division by zero
    }
    return *this;
}

// U++-style methods
bool Point::Is() const { 
    return true; // Points always "exist"
}
bool Point::IsZero() const { 
    return x == 0 && y == 0; 
}
Point Point::Neg() const { 
    return Point(-x, -y); 
}
int Point::GetDistance(const Point& other) const { 
    int dx = x - other.x;
    int dy = y - other.y;
    return static_cast<int>(std::sqrt(dx*dx + dy*dy));
}

}