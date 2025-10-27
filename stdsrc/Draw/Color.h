// U++-compatible Color wrapper implemented using direct implementation
// This header is aggregated and wrapped into namespace Upp by Draw.h

class Color {
public:
    uint8_t r, g, b, a;

    // Constructors
    Color() : r(0), g(0), b(0), a(255) {}  // Black by default
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
    
    // Create from 32-bit value (ARGB format as used in U++)
    explicit Color(uint32_t argb) 
        : r((argb >> 16) & 0xFF), g((argb >> 8) & 0xFF), b(argb & 0xFF), a((argb >> 24) & 0xFF) {}

    // U++-style static constructors for common colors
    static Color White() { return Color(255, 255, 255); }
    static Color Black() { return Color(0, 0, 0); }
    static Color Red() { return Color(255, 0, 0); }
    static Color Green() { return Color(0, 255, 0); }
    static Color Blue() { return Color(0, 0, 255); }
    static Color Yellow() { return Color(255, 255, 0); }
    static Color Magenta() { return Color(255, 0, 255); }
    static Color Cyan() { return Color(0, 255, 255); }
    static Color Gray() { return Color(128, 128, 128); }
    static Color LtGray() { return Color(192, 192, 192); }
    static Color DkGray() { return Color(64, 64, 64); }
    static Color Null() { return Color(0, 0, 0, 0); }

    // Assignment operators
    Color& operator=(const Color& other) {
        if (this != &other) {
            r = other.r; g = other.g; b = other.b; a = other.a;
        }
        return *this;
    }

    // Comparison operators
    bool operator==(const Color& other) const { 
        return r == other.r && g == other.g && b == other.b && a == other.a; 
    }
    bool operator!=(const Color& other) const { return !(*this == other); }

    // U++-style access methods
    uint8_t GetR() const { return r; }
    uint8_t GetG() const { return g; }
    uint8_t GetB() const { return b; }
    uint8_t GetA() const { return a; }

    void SetR(uint8_t value) { r = value; }
    void SetG(uint8_t value) { g = value; }
    void SetB(uint8_t value) { b = value; }
    void SetA(uint8_t value) { a = value; }

    // U++-style methods
    bool IsNull() const { return a == 0; }
    bool Is() const { return a != 0; }

    // Conversion to 32-bit ARGB value (as used in U++)
    uint32_t GetARGB() const {
        return (static_cast<uint32_t>(a) << 24) | 
               (static_cast<uint32_t>(r) << 16) | 
               (static_cast<uint32_t>(g) << 8) | 
               static_cast<uint32_t>(b);
    }

    // U++-style color operations
    Color Blend(const Color& other, double factor) const {
        factor = std::max(0.0, std::min(1.0, factor)); // Clamp to [0, 1]
        double inv_factor = 1.0 - factor;
        return Color(
            static_cast<uint8_t>(r * inv_factor + other.r * factor),
            static_cast<uint8_t>(g * inv_factor + other.g * factor),
            static_cast<uint8_t>(b * inv_factor + other.b * factor),
            static_cast<uint8_t>(a * inv_factor + other.a * factor)
        );
    }

    // Brightness adjustment
    Color AdjustBrightness(double factor) const {
        factor = std::max(0.0, factor); // Don't go below 0
        return Color(
            std::min(255, static_cast<int>(r * factor)),
            std::min(255, static_cast<int>(g * factor)),
            std::min(255, static_cast<int>(b * factor)),
            a
        );
    }

    // Invert color
    Color Invert() const {
        return Color(255 - r, 255 - g, 255 - b, a);
    }

    // Grayscale conversion
    Color ToGray() const {
        uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
        return Color(gray, gray, gray, a);
    }

    // U++-style linear interpolation
    static Color Slerp(const Color& c1, const Color& c2, double factor) {
        return c1.Blend(c2, factor);
    }
};