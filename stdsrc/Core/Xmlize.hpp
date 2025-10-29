#pragma once
#ifndef _Core_Xmlize_hpp_
#define _Core_Xmlize_hpp_

#include <string>
#include <vector>
#include <map>
#include "Core.h"

NAMESPACE_UPP

// Basic XML utilities
std::string XmlEscape(const std::string& input);
std::string XmlUnescape(const std::string& input);

// Basic XML element representation
struct XmlElement {
    std::string tag;
    std::map<std::string, std::string> attributes;
    std::string content;
    std::vector<XmlElement> children;
    
    XmlElement(const std::string& t = "");
    
    void AddAttribute(const std::string& name, const std::string& value);
    XmlElement& AddChild(const std::string& tag);
    std::string ToXmlString(int depth = 0) const;
};

std::string ToXml(const XmlElement& element);
std::string ToXmlAttribute(const std::string& value);
std::string ToXmlAttribute(int value);
std::string ToXmlAttribute(double value);
std::string ToXmlAttribute(bool value);

XmlElement ParseXmlElement(const std::string& xml);

// Template-based Xmlizer for custom types
template<typename T>
void Xmlize(const T& obj, XmlElement& xml);

template<typename T>
void Xmlize(T& obj, const XmlElement& xml);

END_UPP_NAMESPACE

#endif