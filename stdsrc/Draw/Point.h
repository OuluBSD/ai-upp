#pragma once
// U++-compatible Point wrapper implemented using std::pair or direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

#include <cstdint>  // For uint8_t, uint32_t
#include <algorithm>
#include <cmath>

class Point {
public:
    int x, y;

    // Constructors
    Point() : x(0), y(0) {}
    Point(int x_, int y_) : x(x_), y(y_) {}

    // U++-style static constructors
    static Point Make(int x, int y);
    static Point Zero();

    // Assignment operators
    Point& operator=(const Point& other);

    // Arithmetic operators
    Point operator+(const Point& other) const;
    Point& operator+=(const Point& other);
    Point operator-(const Point& other) const;
    Point& operator-=(const Point& other);
    Point operator*(int scale) const;
    Point& operator*=(int scale);
    Point operator/(int div) const;
    Point& operator/=(int div);

    // Comparison operators
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const;

    // U++-style access methods
    int GetX() const;
    int GetY() const;
    void Set(int x_, int y_);
    void SetX(int x_);
    void SetY(int y_);

    // U++-style operations
    Point& Offset(int dx, int dy);
    Point& Offset(const Point& delta);
    Point& MulDiv(int mul, int div);

    // U++-style methods
    bool Is() const; // Points always "exist"
    bool IsZero() const;
    Point Neg() const;
    int GetDistance(const Point& other) const;
};