#include "Core.h"
#include <string>
#include <vector>
#include <map>

NAMESPACE_UPP

// UTF-8 utility functions
int Utf8Length(const char* utf8, int max_bytes) {
    if (!utf8 || max_bytes <= 0) return 0;
    
    int len = 0;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(utf8);
    
    for (int i = 0; i < max_bytes && *p; ++i) {
        unsigned char c = *p;
        
        if ((c & 0x80) == 0) { // ASCII
            p++;
            len++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            if (i + 1 < max_bytes) {
                p += 2;
                len++;
                i++;
            } else {
                break; // Incomplete sequence
            }
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            if (i + 2 < max_bytes) {
                p += 3;
                len++;
                i += 2;
            } else {
                break; // Incomplete sequence
            }
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            if (i + 3 < max_bytes) {
                p += 4;
                len++;
                i += 3;
            } else {
                break; // Incomplete sequence
            }
        } else {
            p++; // Invalid starting byte
            len++;
        }
    }
    
    return len;
}

int Utf8Length(const std::string& utf8) {
    return Utf8Length(utf8.c_str(), static_cast<int>(utf8.length()));
}

std::string Utf8Substr(const std::string& utf8, int start, int length) {
    if (start < 0 || length <= 0 || start >= static_cast<int>(utf8.length())) {
        return "";
    }
    
    const char* p = utf8.c_str();
    const char* start_ptr = p;
    int char_count = 0;
    
    // Find start position
    for (int i = 0; i < static_cast<int>(utf8.length()) && char_count < start; ) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        
        if ((c & 0x80) == 0) { // ASCII
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            i += 4;
        } else {
            i++; // Invalid byte
        }
        
        char_count++;
        start_ptr = p + i;
    }
    
    if (char_count < start) return ""; // Start position beyond string length
    
    // Find end position
    const char* end_ptr = start_ptr;
    char_count = 0;
    
    for (int i = start_ptr - p; i < static_cast<int>(utf8.length()) && char_count < length; ) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        
        if ((c & 0x80) == 0) { // ASCII
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            i += 4;
        } else {
            i++; // Invalid byte
        }
        
        char_count++;
        end_ptr = p + i;
    }
    
    return std::string(start_ptr, end_ptr - start_ptr);
}

int Utf8Next(const char* utf8, int* byte_pos, int max_bytes) {
    if (!utf8 || byte_pos == nullptr || *byte_pos >= max_bytes) return -1;
    
    const unsigned char* p = reinterpret_cast<const unsigned char*>(utf8 + *byte_pos);
    unsigned char c = *p;
    
    if ((c & 0x80) == 0) { // ASCII
        (*byte_pos)++;
        return c;
    } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
        if (*byte_pos + 1 >= max_bytes) return -1;
        int code = ((c & 0x1F) << 6) | (p[1] & 0x3F);
        (*byte_pos) += 2;
        return code;
    } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
        if (*byte_pos + 2 >= max_bytes) return -1;
        int code = ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        (*byte_pos) += 3;
        return code;
    } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
        if (*byte_pos + 3 >= max_bytes) return -1;
        int code = ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        (*byte_pos) += 4;
        return code;
    } else {
        // Invalid UTF-8 sequence, advance by one byte
        (*byte_pos)++;
        return -1;
    }
}

bool IsAlpha(int codepoint) {
    return (codepoint >= 'A' && codepoint <= 'Z') || 
           (codepoint >= 'a' && codepoint <= 'z') ||
           (codepoint > 127); // Extended Unicode letters
}

bool IsDigit(int codepoint) {
    return (codepoint >= '0' && codepoint <= '9');
}

bool IsAlNum(int codepoint) {
    return IsAlpha(codepoint) || IsDigit(codepoint);
}

bool IsSpace(int codepoint) {
    return codepoint == ' ' || codepoint == '\t' || codepoint == '\n' || 
           codepoint == '\r' || codepoint == '\f' || codepoint == '\v';
}

// Unicode categories
enum class UnicodeCategory {
    Letter,
    Mark,
    Number,
    Punctuation,
    Symbol,
    Separator,
    Other
};

UnicodeCategory GetUnicodeCategory(int codepoint) {
    if (IsAlpha(codepoint)) return UnicodeCategory::Letter;
    if (IsDigit(codepoint)) return UnicodeCategory::Number;
    if (IsSpace(codepoint)) return UnicodeCategory::Separator;
    
    // For this simplified implementation, just return a general category
    if (codepoint < 128) {
        // ASCII range - do simple classification
        if ((codepoint >= 33 && codepoint <= 47) ||  // !"#$%&'()*+,-./
            (codepoint >= 58 && codepoint <= 64) ||  // :;<=>?@
            (codepoint >= 91 && codepoint <= 96) ||  // [\]^_` (excluding underscore)
            (codepoint >= 123 && codepoint <= 126)) { // {|}~
            return UnicodeCategory::Punctuation;
        }
    }
    
    // Default to other for this implementation
    return UnicodeCategory::Other;
}

END_UPP_NAMESPACE