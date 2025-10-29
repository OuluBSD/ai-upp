// STL-backed RTF (Rich Text Format) encoding/decoding implementation for stdsrc/CtrlCore

#include "EncodeRTF.h"
#include <sstream>
#include <iomanip>
#include <map>

namespace Upp {

// Helper function to convert RGB color to RTF color table index
static String ColorToRTF(const Color& color) {
    std::ostringstream oss;
    oss << "\\red" << color.GetR() << "\\green" << color.GetG() << "\\blue" << color.GetB() << ";";
    return oss.str();
}

// Helper function to escape RTF special characters
static String EscapeRTF(const String& text) {
    String result;
    result.Reserve(text.GetLength() * 2); // Reserve space to avoid frequent reallocations
    
    for (int i = 0; i < text.GetLength(); i++) {
        char c = text[i];
        switch (c) {
            case '{':  result += "\\{"; break;
            case '}':  result += "\\}"; break;
            case '\\': result += "\\\\"; break;
            default:
                if (c >= 32 && c <= 126) {
                    result += c;
                } else {
                    // For non-printable characters, use Unicode escape sequences
                    result += "\\u" + AsString(static_cast<int>(c)) + "?";
                }
                break;
        }
    }
    
    return result;
}

// Helper function to escape wide string to RTF
static String EscapeRTF(const WString& text) {
    String result;
    result.Reserve(text.GetLength() * 4); // Reserve more space for Unicode
    
    for (int i = 0; i < text.GetLength(); i++) {
        wchar_t c = text[i];
        if (c < 128) {
            char ch = static_cast<char>(c);
            switch (ch) {
                case '{':  result += "\\{"; break;
                case '}':  result += "\\}"; break;
                case '\\': result += "\\\\"; break;
                default:
                    if (ch >= 32 && ch <= 126) {
                        result += ch;
                    } else {
                        result += "\\u" + AsString(static_cast<int>(c)) + "?";
                    }
                    break;
            }
        } else {
            // Unicode character
            result += "\\u" + AsString(static_cast<int>(c)) + "?";
        }
    }
    
    return result;
}

// Helper function to convert font to RTF font table index
static String FontToRTF(const Font& font) {
    // In a real implementation, this would map to font table entries
    // For now, we'll just return a generic font family
    String family = font.GetFace();
    if (family.IsEmpty()) {
        family = "Times New Roman";
    }
    return family;
}

// Helper function to convert font style to RTF formatting codes
static String FontStyleToRTF(const Font& font) {
    String result;
    
    if (font.IsBold()) {
        result += "\\b";
    }
    if (font.IsItalic()) {
        result += "\\i";
    }
    if (font.IsUnderline()) {
        result += "\\ul";
    }
    if (font.IsStrikeout()) {
        result += "\\strike";
    }
    
    // Font size (half-points in RTF)
    int fontSize = font.GetHeight() * 2;
    result += "\\fs" + AsString(fontSize);
    
    // Font family
    String fontFamily = FontToRTF(font);
    // In a real implementation, this would reference font table index
    // For now, we'll just use the font name directly
    
    return result;
}

// Encode text content to RTF format
String EncodeRTF::Encode(const WString& text) {
    return EscapeRTF(text);
}

String EncodeRTF::Encode(const String& text) {
    return EscapeRTF(text);
}

// Encode text with formatting
String EncodeRTF::Encode(const WString& text, Font font, Color color) {
    std::ostringstream oss;
    
    // Start with formatting codes
    oss << "{" << FontStyleToRTF(font);
    
    // Add color information (in a real implementation, this would reference color table)
    // For now, we'll just note the color in comments
    oss << "\\cf" << color.GetR() << "," << color.GetG() << "," << color.GetB() << " ";
    
    // Add escaped text
    oss << EscapeRTF(text) << "}";
    
    return oss.str();
}

String EncodeRTF::Encode(const String& text, Font font, Color color) {
    return Encode(ToWString(text), font, color);
}

// Encode formatted text block with various attributes
String EncodeRTF::EncodeBlock(const WString& text, 
                             Font font, 
                             Color color, 
                             bool isBold, 
                             bool isItalic, 
                             bool isUnderline) {
    std::ostringstream oss;
    
    // Start with formatting codes
    oss << "{";
    
    // Apply font style
    if (isBold) {
        oss << "\\b";
    }
    if (isItalic) {
        oss << "\\i";
    }
    if (isUnderline) {
        oss << "\\ul";
    }
    
    // Font size (half-points in RTF)
    int fontSize = font.GetHeight() * 2;
    oss << "\\fs" + AsString(fontSize);
    
    // Add color information
    oss << "\\cf" << color.GetR() << "," << color.GetG() << "," << color.GetB() << " ";
    
    // Add escaped text
    oss << EscapeRTF(text) << "}";
    
    return oss.str();
}

// Create RTF document header and footer
String EncodeRTF::CreateHeader(int fontSize, const String& fontFamily) {
    std::ostringstream oss;
    
    // RTF header
    oss << "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033";
    
    // Font table
    oss << "{\\fonttbl{\\f0\\fnil\\fcharset0 " << fontFamily << ";}}";
    
    // Color table (empty for now)
    oss << "{\\colortbl ;}";
    
    // Document properties
    oss << "\\viewkind4\\uc1\\pard\\f0\\fs" << (fontSize * 2) << " "; // Font size in half-points
    
    return oss.str();
}

String EncodeRTF::CreateFooter() {
    return "}";
}

// Encode complete RTF document
String EncodeRTF::EncodeDocument(const WString& content, 
                                Font defaultFont,
                                Color defaultColor) {
    std::ostringstream oss;
    
    // Create header
    oss << CreateHeader(defaultFont.GetHeight(), defaultFont.GetFace());
    
    // Add content
    oss << Encode(content, defaultFont, defaultColor);
    
    // Add footer
    oss << CreateFooter();
    
    return oss.str();
}

// Escape RTF special characters
String EncodeRTF::Escape(const String& text) {
    return EscapeRTF(text);
}

String EncodeRTF::Escape(const WString& text) {
    return EscapeRTF(text);
}

// Color table operations
String EncodeRTF::ColorTable(const Vector<Color>& colors) {
    std::ostringstream oss;
    oss << "{\\colortbl ;"; // Empty entry for index 0
    
    for (const auto& color : colors) {
        oss << ColorToRTF(color);
    }
    
    oss << "}";
    return oss.str();
}

// Font table operations
String EncodeRTF::FontTable(const Vector<String>& fonts) {
    std::ostringstream oss;
    oss << "{\\fonttbl";
    
    for (int i = 0; i < fonts.GetCount(); i++) {
        oss << "{\\f" << i << "\\fnil\\fcharset0 " << fonts[i] << ";}";
    }
    
    oss << "}";
    return oss.str();
}

// RTF parsing utilities implementation
WString ParseRTF::ParseText(const String& rtf) {
    // In a real implementation, this would parse RTF and extract plain text
    // For now, return a simplified version that removes RTF control words
    
    String result = rtf;
    
    // Remove RTF header and footer
    int rtfStart = result.Find("{\\rtf1");
    if (rtfStart >= 0) {
        int rtfEnd = result.ReverseFind('}');
        if (rtfEnd > rtfStart) {
            result = result.Mid(rtfStart + 6, rtfEnd - rtfStart - 6);
        }
    }
    
    // Remove control words (simplified approach)
    // In a real implementation, this would be a proper parser
    while (result.Find('\\') >= 0) {
        int escapePos = result.Find('\\');
        if (escapePos >= 0) {
            // Remove the control word
            int endPos = escapePos + 1;
            while (endPos < result.GetLength() && 
                   (IsAlNum(result[endPos]) || result[endPos] == '-' || result[endPos] == '+')) {
                endPos++;
            }
            result.Remove(escapePos, endPos - escapePos);
        }
    }
    
    // Remove braces
    result.Replace("{", "");
    result.Replace("}", "");
    
    return ToWString(result);
}

// Parse RTF with formatting information
WString ParseRTF::ParseFormattedText(const String& rtf) {
    // In a real implementation, this would parse RTF with formatting
    // For now, just return plain text
    return ParseText(rtf);
}

// Parse RTF document and return structured content
Value ParseRTF::ParseDocument(const String& rtf) {
    // In a real implementation, this would parse the complete RTF document
    // and return structured content with formatting information
    // For now, just return the plain text content
    
    WString text = ParseText(rtf);
    return Value(text);
}

// Extract specific properties from RTF
int ParseRTF::GetFontSize(const String& rtf) {
    // In a real implementation, this would extract font size from RTF
    // For now, return a default value
    return 12;
}

Color ParseRTF::GetColor(const String& rtf) {
    // In a real implementation, this would extract color information from RTF
    // For now, return black
    return Color::Black();
}

Font ParseRTF::GetFont(const String& rtf) {
    // In a real implementation, this would extract font information from RTF
    // For now, return a default font
    return Font::Arial(12);
}

}