#pragma once
#ifndef _Core_Ini_h_
#define _Core_Ini_h_

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "Core.h"

// INI file parser and writer for stdsrc
// Handles Windows-style .ini files with sections and key=value pairs

class IniFile {
private:
    struct Section {
        std::map<std::string, std::string> keys;
        std::vector<std::string> key_order; // Maintain insertion order
        
        void Set(const std::string& key, const std::string& value) {
            auto it = std::find(key_order.begin(), key_order.end(), key);
            if (it == key_order.end()) {
                key_order.push_back(key);
            }
            keys[key] = value;
        }
        
        bool Has(const std::string& key) const {
            return keys.find(key) != keys.end();
        }
        
        std::string Get(const std::string& key) const {
            auto it = keys.find(key);
            return it != keys.end() ? it->second : std::string();
        }
        
        void Remove(const std::string& key) {
            keys.erase(key);
            auto it = std::find(key_order.begin(), key_order.end(), key);
            if (it != key_order.end()) {
                key_order.erase(it);
            }
        }
        
        void Clear() {
            keys.clear();
            key_order.clear();
        }
        
        bool IsEmpty() const {
            return keys.empty();
        }
    };
    
    std::map<std::string, Section> sections;
    std::vector<std::string> section_order; // Maintain insertion order
    
    // Helper functions for string trimming
    static std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }
    
    // Helper function to check if a line is a comment
    static bool IsComment(const std::string& line) {
        return !line.empty() && (line[0] == ';' || line[0] == '#');
    }
    
    // Helper function to check if a line is a section header
    static bool IsSection(const std::string& line) {
        return !line.empty() && line[0] == '[' && line[line.length()-1] == ']';
    }
    
    // Helper function to parse a section header
    static std::string ParseSection(const std::string& line) {
        if (line.length() < 2) return "";
        return Trim(line.substr(1, line.length() - 2));
    }
    
    // Helper function to parse a key-value pair
    static bool ParseKeyValue(const std::string& line, std::string& key, std::string& value) {
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) return false;
        
        key = Trim(line.substr(0, eq_pos));
        value = Trim(line.substr(eq_pos + 1));
        return true;
    }
    
    // Helper function to escape special characters in values
    static std::string EscapeValue(const std::string& value) {
        std::string result;
        result.reserve(value.length() * 2);
        
        for (char c : value) {
            switch (c) {
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                case '\\': result += "\\\\"; break;
                case '"': result += "\\\""; break;
                default: result += c; break;
            }
        }
        
        return result;
    }
    
    // Helper function to unescape special characters in values
    static std::string UnescapeValue(const std::string& value) {
        std::string result;
        result.reserve(value.length());
        
        for (size_t i = 0; i < value.length(); ++i) {
            if (value[i] == '\\' && i + 1 < value.length()) {
                ++i;
                switch (value[i]) {
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += '\\'; result += value[i]; break;
                }
            } else {
                result += value[i];
            }
        }
        
        return result;
    }
    
    // Get or create section
    Section& GetOrCreateSection(const std::string& section_name) {
        auto it = std::find(section_order.begin(), section_order.end(), section_name);
        if (it == section_order.end()) {
            section_order.push_back(section_name);
        }
        return sections[section_name];
    }
    
    // Get existing section or return null
    Section* GetSection(const std::string& section_name) {
        auto it = sections.find(section_name);
        return it != sections.end() ? &it->second : nullptr;
    }
    
    const Section* GetSection(const std::string& section_name) const {
        auto it = sections.find(section_name);
        return it != sections.end() ? &it->second : nullptr;
    }
    
public:
    IniFile() = default;
    
    // Load INI file from path
    bool Load(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        Clear();
        
        std::string current_section;
        std::string line;
        
        while (std::getline(file, line)) {
            // Trim whitespace
            line = Trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || IsComment(line)) {
                continue;
            }
            
            // Check for section header
            if (IsSection(line)) {
                current_section = ParseSection(line);
                continue;
            }
            
            // Parse key-value pair
            std::string key, value;
            if (ParseKeyValue(line, key, value)) {
                Set(current_section, key, value);
            }
        }
        
        return true;
    }
    
    // Save INI file to path
    bool Save(const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        // Write sections in order
        for (const std::string& section_name : section_order) {
            auto section_it = sections.find(section_name);
            if (section_it == sections.end()) continue;
            
            const Section& section = section_it->second;
            
            // Write section header
            if (!section_name.empty()) {
                file << "[" << section_name << "]\n";
            }
            
            // Write key-value pairs in order
            for (const std::string& key : section.key_order) {
                auto key_it = section.keys.find(key);
                if (key_it != section.keys.end()) {
                    file << key << "=" << EscapeValue(key_it->second) << "\n";
                }
            }
            
            // Add blank line after section
            file << "\n";
        }
        
        return true;
    }
    
    // Load INI data from string
    bool LoadFromString(const std::string& data) {
        std::istringstream stream(data);
        Clear();
        
        std::string current_section;
        std::string line;
        
        while (std::getline(stream, line)) {
            // Trim whitespace
            line = Trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || IsComment(line)) {
                continue;
            }
            
            // Check for section header
            if (IsSection(line)) {
                current_section = ParseSection(line);
                continue;
            }
            
            // Parse key-value pair
            std::string key, value;
            if (ParseKeyValue(line, key, value)) {
                Set(current_section, key, value);
            }
        }
        
        return true;
    }
    
    // Save INI data to string
    std::string SaveToString() const {
        std::ostringstream stream;
        
        // Write sections in order
        for (const std::string& section_name : section_order) {
            auto section_it = sections.find(section_name);
            if (section_it == sections.end()) continue;
            
            const Section& section = section_it->second;
            
            // Write section header
            if (!section_name.empty()) {
                stream << "[" << section_name << "]\n";
            }
            
            // Write key-value pairs in order
            for (const std::string& key : section.key_order) {
                auto key_it = section.keys.find(key);
                if (key_it != section.keys.end()) {
                    stream << key << "=" << EscapeValue(key_it->second) << "\n";
                }
            }
            
            // Add blank line after section
            stream << "\n";
        }
        
        return stream.str();
    }
    
    // Set value
    void Set(const std::string& section, const std::string& key, const std::string& value) {
        GetOrCreateSection(section).Set(key, value);
    }
    
    void Set(const std::string& section, const std::string& key, const char* value) {
        Set(section, key, std::string(value ? value : ""));
    }
    
    void Set(const std::string& section, const std::string& key, int value) {
        Set(section, key, std::to_string(value));
    }
    
    void Set(const std::string& section, const std::string& key, double value) {
        Set(section, key, std::to_string(value));
    }
    
    void Set(const std::string& section, const std::string& key, bool value) {
        Set(section, key, value ? "1" : "0");
    }
    
    // Get value with default
    std::string Get(const std::string& section, const std::string& key, const std::string& default_value = "") const {
        const Section* sec = GetSection(section);
        if (!sec) return default_value;
        return sec->Get(key);
    }
    
    int GetInt(const std::string& section, const std::string& key, int default_value = 0) const {
        std::string value = Get(section, key);
        if (value.empty()) return default_value;
        
        try {
            return std::stoi(value);
        } catch (...) {
            return default_value;
        }
    }
    
    double GetDouble(const std::string& section, const std::string& key, double default_value = 0.0) const {
        std::string value = Get(section, key);
        if (value.empty()) return default_value;
        
        try {
            return std::stod(value);
        } catch (...) {
            return default_value;
        }
    }
    
    bool GetBool(const std::string& section, const std::string& key, bool default_value = false) const {
        std::string value = Get(section, key);
        if (value.empty()) return default_value;
        
        // Convert to lowercase for comparison
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        
        return (value == "1" || value == "true" || value == "yes" || value == "on");
    }
    
    // Check if key exists
    bool Has(const std::string& section, const std::string& key) const {
        const Section* sec = GetSection(section);
        return sec && sec->Has(key);
    }
    
    // Remove key
    void Remove(const std::string& section, const std::string& key) {
        Section* sec = GetSection(section);
        if (sec) {
            sec->Remove(key);
        }
    }
    
    // Check if section exists
    bool HasSection(const std::string& section) const {
        return sections.find(section) != sections.end();
    }
    
    // Remove section
    void RemoveSection(const std::string& section) {
        sections.erase(section);
        auto it = std::find(section_order.begin(), section_order.end(), section);
        if (it != section_order.end()) {
            section_order.erase(it);
        }
    }
    
    // Get all sections
    std::vector<std::string> GetSections() const {
        return section_order;
    }
    
    // Get all keys in a section
    std::vector<std::string> GetKeys(const std::string& section) const {
        const Section* sec = GetSection(section);
        if (!sec) return {};
        return sec->key_order;
    }
    
    // Clear all data
    void Clear() {
        sections.clear();
        section_order.clear();
    }
    
    // Check if empty
    bool IsEmpty() const {
        return sections.empty();
    }
    
    // Get total number of sections
    size_t GetSectionCount() const {
        return sections.size();
    }
    
    // Get total number of keys across all sections
    size_t GetKeyCount() const {
        size_t count = 0;
        for (const auto& pair : sections) {
            count += pair.second.keys.size();
        }
        return count;
    }
    
    // Merge with another INI file
    void Merge(const IniFile& other) {
        for (const std::string& section_name : other.section_order) {
            const Section* other_section = other.GetSection(section_name);
            if (!other_section) continue;
            
            Section& section = GetOrCreateSection(section_name);
            for (const std::string& key : other_section->key_order) {
                auto key_it = other_section->keys.find(key);
                if (key_it != other_section->keys.end()) {
                    section.Set(key, key_it->second);
                }
            }
        }
    }
    
    // Copy constructor
    IniFile(const IniFile& other) = default;
    
    // Assignment operator
    IniFile& operator=(const IniFile& other) = default;
    
    // Move constructor
    IniFile(IniFile&& other) noexcept = default;
    
    // Move assignment operator
    IniFile& operator=(IniFile&& other) noexcept = default;
    
    // Destructor
    ~IniFile() = default;
};

// Global convenience functions
inline IniFile LoadIniFile(const std::string& filepath) {
    IniFile ini;
    ini.Load(filepath);
    return ini;
}

inline bool SaveIniFile(const IniFile& ini, const std::string& filepath) {
    return ini.Save(filepath);
}

// Streaming operators
template<typename Stream>
void operator%(Stream& s, IniFile& ini) {
    if (s.IsStoring()) {
        std::string data = ini.SaveToString();
        s % data;
    } else {
        std::string data;
        s % data;
        ini.LoadFromString(data);
    }
}

// String conversion
inline std::string AsString(const IniFile& ini) {
    return ini.SaveToString();
}

#endif