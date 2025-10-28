// STL-backed Draw API implementation

#include "Color.h"
#include <algorithm>

namespace Upp {

// U++-style static constructors for common colors
Color Color::White() { return Color(255, 255, 255); }
Color Color::Black() { return Color(0, 0, 0); }
Color Color::Red() { return Color(255, 0, 0); }
Color Color::Green() { return Color(0, 255, 0); }
Color Color::Blue() { return Color(0, 0, 255); }
Color Color::Yellow() { return Color(255, 255, 0); }
Color Color::Magenta() { return Color(255, 0, 255); }
Color Color::Cyan() { return Color(0, 255, 255); }
Color Color::Gray() { return Color(128, 128, 128); }
Color Color::LtGray() { return Color(192, 192, 192); }
Color Color::DkGray() { return Color(64, 64, 64); }
Color Color::Null() { return Color(0, 0, 0, 0); }

// Assignment operators
Color& Color::operator=(const Color& other) {
    if (this != &other) {
        r = other.r; g = other.g; b = other.b; a = other.a;
    }
    return *this;
}

// Comparison operators
bool Color::operator==(const Color& other) const { 
    return r == other.r && g == other.g && b == other.b && a == other.a; 
}
bool Color::operator!=(const Color& other) const { 
    return !(*this == other); 
}

// U++-style access methods
uint8_t Color::GetR() const { 
    return r; 
}
uint8_t Color::GetG() const { 
    return g; 
}
uint8_t Color::GetB() const { 
    return b; 
}
uint8_t Color::GetA() const { 
    return a; 
}

void Color::SetR(uint8_t value) { 
    r = value; 
}
void Color::SetG(uint8_t value) { 
    g = value; 
}
void Color::SetB(uint8_t value) { 
    b = value; 
}
void Color::SetA(uint8_t value) { 
    a = value; 
}

// U++-style methods
bool Color::IsNull() const { 
    return a == 0; 
}
bool Color::Is() const { 
    return a != 0; 
}

// Conversion to 32-bit ARGB value (as used in U++)
uint32_t Color::GetARGB() const {
    return (static_cast<uint32_t>(a) << 24) | 
           (static_cast<uint32_t>(r) << 16) | 
           (static_cast<uint32_t>(g) << 8) | 
           static_cast<uint32_t>(b);
}

// U++-style color operations
Color Color::Blend(const Color& other, double factor) const {
    double clamped_factor = std::max(0.0, std::min(1.0, factor)); // Clamp to [0, 1]
    double inv_factor = 1.0 - clamped_factor;
    return Color(
        static_cast<uint8_t>(r * inv_factor + other.r * clamped_factor),
        static_cast<uint8_t>(g * inv_factor + other.g * clamped_factor),
        static_cast<uint8_t>(b * inv_factor + other.b * clamped_factor),
        static_cast<uint8_t>(a * inv_factor + other.a * clamped_factor)
    );
}

// Brightness adjustment
Color Color::AdjustBrightness(double factor) const {
    factor = std::max(0.0, factor); // Don't go below 0
    return Color(
        std::min(255, static_cast<int>(r * factor)),
        std::min(255, static_cast<int>(g * factor)),
        std::min(255, static_cast<int>(b * factor)),
        a
    );
}

// Invert color
Color Color::Invert() const {
    return Color(255 - r, 255 - g, 255 - b, a);
}

// Grayscale conversion
Color Color::ToGray() const {
    uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
    return Color(gray, gray, gray, a);
}

// U++-style linear interpolation
Color Color::Slerp(const Color& c1, const Color& c2, double factor) {
    return c1.Blend(c2, factor);
}

}