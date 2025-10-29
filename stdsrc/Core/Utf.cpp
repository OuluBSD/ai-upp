#include "Core.h"
#include <string>
#include <vector>
#include <iostream>

NAMESPACE_UPP

// UTF-8 utility functions

std::string ToUtf8(int codepoint) {
    std::string result;
    
    if (codepoint < 0x80) {
        result.push_back(static_cast<char>(codepoint));
    } else if (codepoint < 0x800) {
        result.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint < 0x10000) {
        result.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint < 0x110000) {
        result.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        // Invalid codepoint, return replacement character
        result = "\xEF\xBF\xBD"; // U+FFFD (replacement character)
    }
    
    return result;
}

std::vector<int> FromUtf8(const std::string& utf8) {
    std::vector<int> result;
    size_t i = 0;
    
    while (i < utf8.length()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        
        if ((c & 0x80) == 0) { // ASCII
            result.push_back(c);
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            if (i + 1 >= utf8.length()) break; // Incomplete sequence
            int codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(utf8[i+1]) & 0x3F);
            result.push_back(codepoint);
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            if (i + 2 >= utf8.length()) break; // Incomplete sequence
            int codepoint = ((c & 0x0F) << 12) | 
                           ((static_cast<unsigned char>(utf8[i+1]) & 0x3F) << 6) | 
                           (static_cast<unsigned char>(utf8[i+2]) & 0x3F);
            result.push_back(codepoint);
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            if (i + 3 >= utf8.length()) break; // Incomplete sequence
            int codepoint = ((c & 0x07) << 18) | 
                           ((static_cast<unsigned char>(utf8[i+1]) & 0x3F) << 12) | 
                           ((static_cast<unsigned char>(utf8[i+2]) & 0x3F) << 6) | 
                           (static_cast<unsigned char>(utf8[i+3]) & 0x3F);
            result.push_back(codepoint);
            i += 4;
        } else {
            // Invalid UTF-8 sequence
            result.push_back(0xFFFD); // Replacement character
            i++;
        }
    }
    
    return result;
}

std::string ToUtf8(const std::vector<int>& codepoints) {
    std::string result;
    
    for (int codepoint : codepoints) {
        result += ToUtf8(codepoint);
    }
    
    return result;
}

// Helper to get UTF-8 length in Unicode characters
int GetUtf8Length(const std::string& utf8) {
    int length = 0;
    size_t i = 0;
    
    while (i < utf8.length()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        
        if ((c & 0x80) == 0) { // ASCII
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            i += 4;
        } else {
            i++; // Invalid byte, treat as single character
        }
        
        length++;
    }
    
    return length;
}

// Validate UTF-8 sequence
bool IsValidUtf8(const std::string& utf8) {
    size_t i = 0;
    
    while (i < utf8.length()) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        
        if ((c & 0x80) == 0) { // ASCII
            i++;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence
            if (i + 1 >= utf8.length() || 
                (static_cast<unsigned char>(utf8[i+1]) & 0xC0) != 0x80) {
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence
            if (i + 2 >= utf8.length() || 
                (static_cast<unsigned char>(utf8[i+1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(utf8[i+2]) & 0xC0) != 0x80) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence
            if (i + 3 >= utf8.length() || 
                (static_cast<unsigned char>(utf8[i+1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(utf8[i+2]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(utf8[i+3]) & 0xC0) != 0x80) {
                return false;
            }
            i += 4;
        } else {
            return false; // Invalid start byte
        }
    }
    
    return true;
}

END_UPP_NAMESPACE