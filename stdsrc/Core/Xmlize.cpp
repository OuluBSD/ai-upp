#include "Core.h"
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

NAMESPACE_UPP

// Basic XML utilities
std::string XmlEscape(const std::string& input) {
    std::string result;
    result.reserve(input.length() * 2); // Reserve space for worst case
    
    for (char c : input) {
        switch (c) {
            case '&':  result += "&amp;";  break;
            case '\"': result += "&quot;"; break;
            case '\'': result += "&apos;"; break;
            case '<':  result += "&lt;";   break;
            case '>':  result += "&gt;";   break;
            default:   result += c;        break;
        }
    }
    
    return result;
}

std::string XmlUnescape(const std::string& input) {
    std::string result;
    result.reserve(input.length()); // Usually gets smaller
    
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '&' && i + 1 < input.length()) {
            if (input[i+1] == 'a' && i + 5 <= input.length() && 
                input.substr(i, 6) == "&quot;") {
                result += '"';
                i += 5;
            } else if (input[i+1] == 'a' && i + 5 <= input.length() && 
                       input.substr(i, 6) == "&apos;") {
                result += '\'';
                i += 5;
            } else if (input[i+1] == 'l' && i + 3 <= input.length() && 
                       input.substr(i, 4) == "&lt;") {
                result += '<';
                i += 3;
            } else if (input[i+1] == 'g' && i + 3 <= input.length() && 
                       input.substr(i, 4) == "&gt;") {
                result += '>';
                i += 3;
            } else if (input[i+1] == 'a' && i + 4 <= input.length() && 
                       input.substr(i, 5) == "&amp;") {
                result += '&';
                i += 4;
            } else {
                result += input[i];
            }
        } else {
            result += input[i];
        }
    }
    
    return result;
}

// Basic XML element representation
struct XmlElement {
    std::string tag;
    std::map<std::string, std::string> attributes;
    std::string content;
    std::vector<XmlElement> children;
    
    XmlElement(const std::string& t = "") : tag(t) {}
    
    void AddAttribute(const std::string& name, const std::string& value) {
        attributes[name] = value;
    }
    
    XmlElement& AddChild(const std::string& tag) {
        children.emplace_back(tag);
        return children.back();
    }
    
    std::string ToXmlString(int depth = 0) const {
        std::string indent(depth * 2, ' ');
        std::string result = indent + "<" + tag;
        
        for (const auto& attr : attributes) {
            result += " " + attr.first + "=\"" + XmlEscape(attr.second) + "\"";
        }
        
        if (children.empty() && content.empty()) {
            result += "/>";
        } else {
            result += ">";
            
            if (!content.empty()) {
                result += XmlEscape(content);
            }
            
            for (const auto& child : children) {
                result += "\n" + child.ToXmlString(depth + 1);
            }
            
            if (!children.empty()) {
                result += "\n" + indent;
            }
            
            result += "</" + tag + ">";
        }
        
        return result;
    }
};

std::string ToXml(const XmlElement& element) {
    return element.ToXmlString();
}

// XML serialization for basic types
std::string ToXmlAttribute(const std::string& value) {
    return XmlEscape(value);
}

std::string ToXmlAttribute(int value) {
    return std::to_string(value);
}

std::string ToXmlAttribute(double value) {
    return std::to_string(value);
}

std::string ToXmlAttribute(bool value) {
    return value ? "true" : "false";
}

// Basic XML parsing (simplified)
XmlElement ParseXmlElement(const std::string& xml) {
    // This is a very simplified XML parser just for demonstration
    // A full implementation would be much more complex
    XmlElement result;
    
    // For now, return an empty element
    return result;
}

// Template-based Xmlizer for custom types
template<typename T>
void Xmlize(const T& obj, XmlElement& xml) {
    // Default implementation does nothing
    // Specializations should be provided for custom types
}

template<typename T>
void Xmlize(T& obj, const XmlElement& xml) {
    // Default implementation does nothing
    // Specializations should be provided for custom types
}

END_UPP_NAMESPACE