// STL-backed Draw API implementation

#include "Size.h"
#include <algorithm>

namespace Upp {

// U++-style static constructors
Size Size::Make(int cx, int cy) { 
    return Size(cx, cy); 
}
Size Size::Zero() { 
    return Size(0, 0); 
}

// Assignment operators
Size& Size::operator=(const Size& other) {
    if (this != &other) {
        cx = other.cx;
        cy = other.cy;
    }
    return *this;
}

// Arithmetic operators
Size Size::operator+(const Size& other) const { 
    return Size(cx + other.cx, cy + other.cy); 
}
Size& Size::operator+=(const Size& other) { 
    cx += other.cx; cy += other.cy; 
    return *this; 
}
Size Size::operator-(const Size& other) const { 
    return Size(cx - other.cx, cy - other.cy); 
}
Size& Size::operator-=(const Size& other) { 
    cx -= other.cx; cy -= other.cy; 
    return *this; 
}
Size Size::operator*(int scale) const { 
    return Size(cx * scale, cy * scale); 
}
Size& Size::operator*=(int scale) { 
    cx *= scale; cy *= scale; 
    return *this; 
}
Size Size::operator/(int div) const { 
    return Size(cx / div, cy / div); 
}
Size& Size::operator/=(int div) { 
    cx /= div; cy /= div; 
    return *this; 
}

// Comparison operators
bool Size::operator==(const Size& other) const { 
    return cx == other.cx && cy == other.cy; 
}
bool Size::operator!=(const Size& other) const { 
    return !(*this == other); 
}

// U++-style access methods
int Size::GetWidth() const { 
    return cx; 
}
int Size::GetHeight() const { 
    return cy; 
}
void Size::Set(int cx_, int cy_) { 
    cx = cx_; cy = cy_; 
}
void Size::SetWidth(int cx_) { 
    cx = cx_; 
}
void Size::SetHeight(int cy_) { 
    cy = cy_; 
}

// U++-style operations
Size& Size::Scale(int mul, int div) { 
    if (div != 0) {
        cx = (cx * mul) / div;
        cy = (cy * mul) / div;
    } else {
        cx = 0; cy = 0; // Handle division by zero
    }
    return *this;
}

// U++-style methods
bool Size::Is() const { 
    return true; // Sizes always "exist"
}
bool Size::IsZero() const { 
    return cx == 0 && cy == 0; 
}
bool Size::IsEmpty() const { 
    return cx <= 0 || cy <= 0; 
}
Size Size::Inverse() const { 
    return Size(cy, cx); // Swapped dimensions
}
Size& Size::Inverse() { 
    std::swap(cx, cy); 
    return *this; 
}

// Area calculation
int Size::GetArea() const { 
    return cx * cy; 
}

}