#include "St.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

NAMESPACE_UPP

StringVector Split(const String& text, char delimiter) {
    StringVector result;
    std::istringstream stream(text);
    std::string segment;
    
    while (std::getline(stream, segment, delimiter)) {
        result.push_back(segment);
    }
    
    return result;
}

StringVector Split(const String& text, const String& delimiter) {
    StringVector result;
    
    if (delimiter.empty()) {
        result.push_back(text);
        return result;
    }
    
    size_t start = 0;
    size_t end = text.find(delimiter);
    
    while (end != std::string::npos) {
        result.push_back(text.substr(start, end - start));
        start = end + delimiter.length();
        end = text.find(delimiter, start);
    }
    
    result.push_back(text.substr(start));
    return result;
}

String Join(const StringVector& parts, const String& delimiter) {
    if (parts.empty()) return "";
    
    String result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += delimiter + parts[i];
    }
    
    return result;
}

String Trim(const String& text) {
    if (text.empty()) return text;
    
    size_t start = 0;
    size_t end = text.length() - 1;
    
    // Find first non-whitespace
    while (start <= end && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }
    
    // Find last non-whitespace
    while (end >= start && std::isspace(static_cast<unsigned char>(text[end]))) {
        --end;
    }
    
    return text.substr(start, end - start + 1);
}

String ToLower(const String& text) {
    String result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

String ToUpper(const String& text) {
    String result = text;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool StartsWith(const String& text, const String& prefix) {
    if (prefix.length() > text.length()) return false;
    return text.compare(0, prefix.length(), prefix) == 0;
}

bool EndsWith(const String& text, const String& suffix) {
    if (suffix.length() > text.length()) return false;
    return text.compare(text.length() - suffix.length(), suffix.length(), suffix) == 0;
}

String Replace(const String& text, const String& from, const String& to) {
    String result = text;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

int ToInt(const String& text, int default_value) {
    try {
        return std::stoi(text);
    } catch (...) {
        return default_value;
    }
}

double ToDouble(const String& text, double default_value) {
    try {
        return std::stod(text);
    } catch (...) {
        return default_value;
    }
}

String ToString(int value) {
    return std::to_string(value);
}

String ToString(double value) {
    return std::to_string(value);
}

END_UPP_NAMESPACE