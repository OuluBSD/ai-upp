#ifndef _Qwen_registry_h_
#define _Qwen_registry_h_

#include <string>
#include <map>

// Simple registry class for storing key-value pairs
class Registry {
private:
    std::map<std::string, std::string> registry_map;

public:
    Registry() = default;
    
    // Get value for a key, return empty string if not found
    std::string getValue(const std::string& key) const {
        auto it = registry_map.find(key);
        if (it != registry_map.end()) {
            return it->second;
        }
        return std::string();
    }
    
    // Set a key-value pair
    void setValue(const std::string& key, const std::string& value) {
        registry_map[key] = value;
    }
    
    // Check if a key exists
    bool hasKey(const std::string& key) const {
        return registry_map.find(key) != registry_map.end();
    }
    
    // Remove a key
    void removeKey(const std::string& key) {
        registry_map.erase(key);
    }
};

#endif // _Qwen_registry_h_