// U++-compatible Font wrapper implemented for cross-platform compatibility
// This header is aggregated and wrapped into namespace Upp by Draw.h

class Font {
public:
    std::string face;      // Font face name
    int height;            // Font height in pixels
    bool bold;             // Bold flag
    bool italic;           // Italic flag
    bool underline;        // Underline flag
    bool strikeout;        // Strikeout flag
    int angle;             // Text angle in 1/10 degrees (0 = 0°, 3600 = 360°)

private:
    // Platform-specific font handle (could be HFONT on Windows, PangoFontDescription on Linux, etc.)
    void* platform_font_handle;

public:
    // Constructors
    Font() : height(12), bold(false), italic(false), underline(false), strikeout(false), angle(0), platform_font_handle(nullptr) {
        face = "Arial"; // Default font
    }
    
    Font(const std::string& face_name, int height_) 
        : face(face_name), height(height_), bold(false), italic(false), 
          underline(false), strikeout(false), angle(0), platform_font_handle(nullptr) {}

    // Copy constructor
    Font(const Font& other) = default;
    
    // Assignment operator
    Font& operator=(const Font& other) = default;

    // U++-style static constructors
    static Font Arial(int height) { return Font("Arial", height); }
    static Font Sans(int height) { return Font("sans-serif", height); }
    static Font System() { return Font("System", 12); }
    static Font ArialBold(int height) { 
        Font f("Arial", height);
        f.bold = true;
        return f;
    }
    static Font Std() { return Font("Arial", 12); }  // Standard font
    static Font DejaVuSans(int height) { return Font("DejaVu Sans", height); }

    // U++-style methods to modify font properties
    Font& Arial(int height) { 
        face = "Arial"; 
        this->height = height;
        return *this;
    }
    
    Font& Face(const std::string& face_name) { 
        face = face_name; 
        return *this;
    }
    
    Font& Height(int h) { 
        height = h; 
        return *this;
    }
    
    Font& Bold(bool b = true) { 
        bold = b; 
        return *this;
    }
    
    Font& Italic(bool i = true) { 
        italic = i; 
        return *this;
    }
    
    Font& Underline(bool u = true) { 
        underline = u; 
        return *this;
    }
    
    Font& Strikeout(bool s = true) { 
        strikeout = s; 
        return *this;
    }
    
    Font& Angle(int a) { 
        angle = a; 
        return *this;
    }

    // U++-style property accessors
    std::string GetFace() const { return face; }
    int GetHeight() const { return height; }
    bool IsBold() const { return bold; }
    bool IsItalic() const { return italic; }
    bool IsUnderline() const { return underline; }
    bool IsStrikeout() const { return strikeout; }
    int GetAngle() const { return angle; }

    // U++-style methods
    bool Is() const { return !face.empty(); }  // Font is valid if it has a face name
    bool IsNull() const { return face.empty(); }

    // U++-style comparison operators
    bool operator==(const Font& other) const {
        return face == other.face && height == other.height && 
               bold == other.bold && italic == other.italic &&
               underline == other.underline && strikeout == other.strikeout &&
               angle == other.angle;
    }
    
    bool operator!=(const Font& other) const {
        return !(*this == other);
    }

    // Platform-specific font handle management
    void* GetPlatformFontHandle() const { return platform_font_handle; }
    void SetPlatformFontHandle(void* handle) { platform_font_handle = handle; }

    // U++-style font metrics (these would typically be implemented using platform-specific APIs)
    int GetAscent() const { 
        // This would be calculated based on the platform font metrics
        // For now, return a simple approximation
        return static_cast<int>(height * 0.8); 
    }
    
    int GetDescent() const { 
        // This would be calculated based on the platform font metrics
        return static_cast<int>(height * 0.2); 
    }
    
    int GetInternalLeading() const { 
        // Internal leading is typically a small amount
        return 2; 
    }
    
    int GetExternalLeading() const { 
        // External leading is typically a small amount
        return 2; 
    }

    // Text size calculation (these would typically use platform-specific APIs)
    Size GetTextSize(const std::string& text) const {
        // This is a very rough approximation
        // In a real implementation, this would use platform-specific text measurement
        if (text.empty()) {
            return Size(0, height);
        }
        
        // Very rough approximation: assume average character width is half the height
        int avgCharWidth = height / 2;
        int textWidth = static_cast<int>(text.length()) * avgCharWidth;
        return Size(textWidth, height);
    }
    
    // U++-style font operations
    int GetCy() const { return height; }  // Character height
    
    // U++-style font comparison with "other" fonts
    bool IsEqual(const Font& other) const {
        return *this == other;
    }
};