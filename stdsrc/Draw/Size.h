#pragma once
// U++-compatible Size wrapper implemented using std::pair or direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

#include <algorithm>

class Size {
public:
    int cx, cy;

    // Constructors
    Size() : cx(0), cy(0) {}
    Size(int cx_, int cy_) : cx(cx_), cy(cy_) {}

    // U++-style static constructors
    static Size Make(int cx, int cy);
    static Size Zero();

    // Assignment operators
    Size& operator=(const Size& other);

    // Arithmetic operators
    Size operator+(const Size& other) const;
    Size& operator+=(const Size& other);
    Size operator-(const Size& other) const;
    Size& operator-=(const Size& other);
    Size operator*(int scale) const;
    Size& operator*=(int scale);
    Size operator/(int div) const;
    Size& operator/=(int div);

    // Comparison operators
    bool operator==(const Size& other) const;
    bool operator!=(const Size& other) const;

    // U++-style access methods
    int GetWidth() const;
    int GetHeight() const;
    void Set(int cx_, int cy_);
    void SetWidth(int cx_);
    void SetHeight(int cy_);

    // U++-style operations
    Size& Scale(int mul, int div);

    // U++-style methods
    bool Is() const; // Sizes always "exist"
    bool IsZero() const;
    bool IsEmpty() const;
    Size Inverse() const; // Swapped dimensions
    Size& Inverse();

    // Area calculation
    int GetArea() const;
};