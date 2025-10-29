#pragma once
#ifndef _Core_Ini_h_
#define _Core_Ini_h_

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include "Core.h"

// INI file parser and writer
class IniFile {
public:
    IniFile();
    
    // Load from string
    bool LoadFromString(const std::string& content);
    
    // Load from file
    bool LoadFromFile(const std::string& filename);
    
    // Save to string
    std::string SaveToString() const;
    
    // Save to file
    bool SaveToFile(const std::string& filename) const;
    
    // Get/set values
    std::string Get(const std::string& section, const std::string& key, const std::string& default_value = "") const;
    void Set(const std::string& section, const std::string& key, const std::string& value);
    
    // Get/set with type conversion
    int GetInt(const std::string& section, const std::string& key, int default_value = 0) const;
    bool GetBool(const std::string& section, const std::string& key, bool default_value = false) const;
    double GetDouble(const std::string& section, const std::string& key, double default_value = 0.0) const;
    
    void SetInt(const std::string& section, const std::string& key, int value);
    void SetBool(const std::string& section, const std::string& key, bool value);
    void SetDouble(const std::string& section, const std::string& key, double value);
    
    // Section operations
    std::vector<std::string> GetSectionNames() const;
    std::vector<std::string> GetKeys(const std::string& section) const;
    
    // Clear all data
    void Clear();
    
private:
    std::map<std::string, std::map<std::string, std::string>> data;
};

#endif