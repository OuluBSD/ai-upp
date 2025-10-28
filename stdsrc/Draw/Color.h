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
    static Color White();
    static Color Black();
    static Color Red();
    static Color Green();
    static Color Blue();
    static Color Yellow();
    static Color Magenta();
    static Color Cyan();
    static Color Gray();
    static Color LtGray();
    static Color DkGray();
    static Color Null();

    // Assignment operators
    Color& operator=(const Color& other);

    // Comparison operators
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    // U++-style access methods
    uint8_t GetR() const;
    uint8_t GetG() const;
    uint8_t GetB() const;
    uint8_t GetA() const;

    void SetR(uint8_t value);
    void SetG(uint8_t value);
    void SetB(uint8_t value);
    void SetA(uint8_t value);

    // U++-style methods
    bool IsNull() const;
    bool Is() const;

    // Conversion to 32-bit ARGB value (as used in U++)
    uint32_t GetARGB() const;

    // U++-style color operations
    Color Blend(const Color& other, double factor) const;

    // Brightness adjustment
    Color AdjustBrightness(double factor) const;

    // Invert color
    Color Invert() const;

    // Grayscale conversion
    Color ToGray() const;

    // U++-style linear interpolation
    static Color Slerp(const Color& c1, const Color& c2, double factor);
};