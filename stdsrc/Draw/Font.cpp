// STL-backed Draw API implementation

#include "Font.h"

namespace Upp {

// U++-style static constructors
Font Font::Arial(int height) { 
    return Font("Arial", height); 
}

Font Font::Sans(int height) { 
    return Font("sans-serif", height); 
}

Font Font::System() { 
    return Font("System", 12); 
}

Font Font::ArialBold(int height) { 
    Font f("Arial", height);
    f.bold = true;
    return f;
}

Font Font::Std() { 
    return Font("Arial", 12); 
}

Font Font::DejaVuSans(int height) { 
    return Font("DejaVu Sans", height); 
}

// U++-style methods to modify font properties
Font& Font::SetArial(int height) { 
    face = "Arial"; 
    this->height = height;
    return *this;
}

Font& Font::Face(const std::string& face_name) { 
    face = face_name; 
    return *this;
}

Font& Font::Height(int h) { 
    height = h; 
    return *this;
}

Font& Font::Bold(bool b) { 
    bold = b; 
    return *this;
}

Font& Font::Italic(bool i) { 
    italic = i; 
    return *this;
}

Font& Font::Underline(bool u) { 
    underline = u; 
    return *this;
}

Font& Font::Strikeout(bool s) { 
    strikeout = s; 
    return *this;
}

Font& Font::Angle(int a) { 
    angle = a; 
    return *this;
}

// U++-style property accessors
std::string Font::GetFace() const { 
    return face; 
}

int Font::GetHeight() const { 
    return height; 
}

bool Font::IsBold() const { 
    return bold; 
}

bool Font::IsItalic() const { 
    return italic; 
}

bool Font::IsUnderline() const { 
    return underline; 
}

bool Font::IsStrikeout() const { 
    return strikeout; 
}

int Font::GetAngle() const { 
    return angle; 
}

// U++-style methods
bool Font::Is() const { 
    return !face.empty(); 
}

bool Font::IsNull() const { 
    return face.empty(); 
}

// U++-style comparison operators
bool Font::operator==(const Font& other) const {
    return face == other.face && height == other.height && 
           bold == other.bold && italic == other.italic &&
           underline == other.underline && strikeout == other.strikeout &&
           angle == other.angle;
}

bool Font::operator!=(const Font& other) const {
    return !(*this == other);
}

// Platform-specific font handle management
void* Font::GetPlatformFontHandle() const { 
    return platform_font_handle; 
}

void Font::SetPlatformFontHandle(void* handle) { 
    platform_font_handle = handle; 
}

// U++-style font metrics
int Font::GetAscent() const { 
    return static_cast<int>(height * 0.8); 
}

int Font::GetDescent() const { 
    return static_cast<int>(height * 0.2); 
}

int Font::GetInternalLeading() const { 
    return 2; 
}

int Font::GetExternalLeading() const { 
    return 2; 
}

// Text size calculation
Size Font::GetTextSize(const std::string& text) const {
    if (text.empty()) {
        return Size(0, height);
    }
    
    // Very rough approximation: assume average character width is half the height
    int avgCharWidth = height / 2;
    int textWidth = static_cast<int>(text.length()) * avgCharWidth;
    return Size(textWidth, height);
}

// U++-style font operations
int Font::GetCy() const { 
    return height; 
}

// U++-style font comparison with "other" fonts
bool Font::IsEqual(const Font& other) const {
    return *this == other;
}

}