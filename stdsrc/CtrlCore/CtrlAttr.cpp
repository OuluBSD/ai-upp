// STL-backed CtrlCore attribute functionality implementation

#include "CtrlAttr.h"
#include <sstream>
#include <iomanip>

namespace Upp {

// Helper function to convert dword to string
static String DwordToString(dword value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Helper function to convert string to dword
static dword StringToDword(const String& str) {
    try {
        return static_cast<dword>(std::stoull(str));
    } catch (...) {
        return 0;
    }
}

// Check if attribute exists
bool CtrlAttr::HasAttr(const String& name) const {
    return attributes.find(name) != attributes.end();
}

// Remove attribute
CtrlAttr& CtrlAttr::RemoveAttr(const String& name) {
    attributes.erase(name);
    return *this;
}

// Clear all attributes
CtrlAttr& CtrlAttr::ClearAttrs() {
    attributes.clear();
    return *this;
}

// Set control's style using attributes
CtrlAttr& CtrlAttr::SetStyle(dword style) {
    return Attr<dword>("style", style);
}

dword CtrlAttr::GetStyle() const {
    return GetAttr<dword>("style", 0);
}

// Set control's data
CtrlAttr& CtrlAttr::SetData(const Value& data) {
    return Attr<Value>("data", data);
}

Value CtrlAttr::GetData() const {
    return GetAttr<Value>("data", Value());
}

// Set control's tag
CtrlAttr& CtrlAttr::SetTag(const Value& tag) {
    return Attr<Value>("tag", tag);
}

Value CtrlAttr::GetTag() const {
    return GetAttr<Value>("tag", Value());
}

// Set control's display
CtrlAttr& CtrlAttr::SetDisplay(const Display& display) {
    return Attr<const Display*>("display", &display);
}

const Display* CtrlAttr::GetDisplay() const {
    return GetAttr<const Display*>("display", nullptr);
}

// Template specializations for common types
template<>
CtrlAttr& CtrlAttr::Attr<int>(const String& name, const int& value) {
    attributes[name] = Value(value);
    return *this;
}

template<>
CtrlAttr& CtrlAttr::Attr<String>(const String& name, const String& value) {
    attributes[name] = Value(value);
    return *this;
}

template<>
CtrlAttr& CtrlAttr::Attr<Color>(const String& name, const Color& value) {
    attributes[name] = Value(value.GetR() << 16 | value.GetG() << 8 | value.GetB());
    return *this;
}

template<>
CtrlAttr& CtrlAttr::Attr<bool>(const String& name, const bool& value) {
    attributes[name] = Value(value ? 1 : 0);
    return *this;
}

template<>
int CtrlAttr::GetAttr<int>(const String& name, const int& default_value) const {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
        return it->second.GetInt();
    }
    return default_value;
}

template<>
String CtrlAttr::GetAttr<String>(const String& name, const String& default_value) const {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
        return it->second.GetString();
    }
    return default_value;
}

template<>
Color CtrlAttr::GetAttr<Color>(const String& name, const Color& default_value) const {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
        int rgb = it->second.GetInt();
        return Color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    return default_value;
}

template<>
bool CtrlAttr::GetAttr<bool>(const String& name, const bool& default_value) const {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
        return it->second.GetInt() != 0;
    }
    return default_value;
}

// Enhanced attribute system with type safety
class TypedAttr {
private:
    enum Type {
        TYPE_INT,
        TYPE_STRING,
        TYPE_COLOR,
        TYPE_BOOL,
        TYPE_DOUBLE,
        TYPE_POINTER
    };
    
    Type type;
    union {
        int int_val;
        double double_val;
        bool bool_val;
        void* ptr_val;
    };
    String string_val;
    
public:
    TypedAttr() : type(TYPE_INT), int_val(0) {}
    TypedAttr(int val) : type(TYPE_INT), int_val(val) {}
    TypedAttr(const String& val) : type(TYPE_STRING), string_val(val) {}
    TypedAttr(double val) : type(TYPE_DOUBLE), double_val(val) {}
    TypedAttr(bool val) : type(TYPE_BOOL), bool_val(val) {}
    TypedAttr(const Color& val) : type(TYPE_COLOR), int_val(val.GetR() << 16 | val.GetG() << 8 | val.GetB()) {}
    TypedAttr(void* val) : type(TYPE_POINTER), ptr_val(val) {}
    
    Type GetType() const { return type; }
    
    template<typename T>
    T As() const {
        // This would be implemented with proper type conversion
        return T{};
    }
    
    int GetInt() const { return type == TYPE_INT ? int_val : 0; }
    String GetString() const { return type == TYPE_STRING ? string_val : String(); }
    double GetDouble() const { return type == TYPE_DOUBLE ? double_val : 0.0; }
    bool GetBool() const { return type == TYPE_BOOL ? bool_val : false; }
    Color GetColor() const { 
        if (type == TYPE_COLOR) {
            return Color((int_val >> 16) & 0xFF, (int_val >> 8) & 0xFF, int_val & 0xFF);
        }
        return Color::Black();
    }
    void* GetPointer() const { return type == TYPE_POINTER ? ptr_val : nullptr; }
};

// Enhanced attribute manager with validation
class AttrValidator {
public:
    enum ValidationRule {
        RULE_REQUIRED,
        RULE_RANGE,
        RULE_PATTERN,
        RULE_CUSTOM
    };
    
    struct Rule {
        ValidationRule rule;
        Value min_val, max_val;
        String pattern;
        std::function<bool(const Value&)> custom_validator;
        
        Rule(ValidationRule r) : rule(r) {}
        Rule(ValidationRule r, const Value& min, const Value& max) : rule(r), min_val(min), max_val(max) {}
        Rule(ValidationRule r, const String& pat) : rule(r), pattern(pat) {}
        Rule(ValidationRule r, std::function<bool(const Value&)> validator) : rule(r), custom_validator(validator) {}
    };
    
private:
    std::map<String, std::vector<Rule>> rules;
    
public:
    AttrValidator& AddRule(const String& attr_name, const Rule& rule) {
        rules[attr_name].push_back(rule);
        return *this;
    }
    
    bool Validate(const String& attr_name, const Value& value) const {
        auto it = rules.find(attr_name);
        if (it == rules.end()) return true; // No rules means valid
        
        for (const auto& rule : it->second) {
            switch (rule.rule) {
                case RULE_REQUIRED:
                    if (value.IsNull()) return false;
                    break;
                case RULE_RANGE:
                    {
                        int val = value.GetInt();
                        int min = rule.min_val.GetInt();
                        int max = rule.max_val.GetInt();
                        if (val < min || val > max) return false;
                    }
                    break;
                case RULE_PATTERN:
                    // Pattern matching would be implemented here
                    break;
                case RULE_CUSTOM:
                    if (rule.custom_validator && !rule.custom_validator(value)) return false;
                    break;
            }
        }
        return true;
    }
    
    std::vector<String> GetAttributeRules(const String& attr_name) const {
        std::vector<String> result;
        auto it = rules.find(attr_name);
        if (it != rules.end()) {
            for (const auto& rule : it->second) {
                switch (rule.rule) {
                    case RULE_REQUIRED: result.push_back("required"); break;
                    case RULE_RANGE: result.push_back("range"); break;
                    case RULE_PATTERN: result.push_back("pattern"); break;
                    case RULE_CUSTOM: result.push_back("custom"); break;
                }
            }
        }
        return result;
    }
};

// Attribute group management for organizing related attributes
class AttrGroup {
private:
    String group_name;
    std::map<String, Value> attributes;
    std::shared_ptr<AttrValidator> validator;
    
public:
    explicit AttrGroup(const String& name) : group_name(name) {}
    
    AttrGroup& Set(const String& attr_name, const Value& value) {
        if (!validator || validator->Validate(attr_name, value)) {
            attributes[attr_name] = value;
        }
        return *this;
    }
    
    Value Get(const String& attr_name, const Value& default_value = Value()) const {
        auto it = attributes.find(attr_name);
        return it != attributes.end() ? it->second : default_value;
    }
    
    bool Has(const String& attr_name) const {
        return attributes.find(attr_name) != attributes.end();
    }
    
    AttrGroup& Remove(const String& attr_name) {
        attributes.erase(attr_name);
        return *this;
    }
    
    AttrGroup& Clear() {
        attributes.clear();
        return *this;
    }
    
    const std::map<String, Value>& GetAttributes() const {
        return attributes;
    }
    
    String GetName() const {
        return group_name;
    }
    
    AttrGroup& SetValidator(std::shared_ptr<AttrValidator> val) {
        validator = val;
        return *this;
    }
    
    std::shared_ptr<AttrValidator> GetValidator() const {
        return validator;
    }
};

// Global attribute registry for shared attribute definitions
class GlobalAttrRegistry {
private:
    static GlobalAttrRegistry* instance;
    std::map<String, std::map<String, Value>> registered_attributes;
    
    GlobalAttrRegistry() = default;
    
public:
    static GlobalAttrRegistry& GetInstance() {
        if (!instance) {
            instance = new GlobalAttrRegistry();
        }
        return *instance;
    }
    
    void Register(const String& category, const String& attr_name, const Value& default_value) {
        registered_attributes[category][attr_name] = default_value;
    }
    
    Value GetRegistered(const String& category, const String& attr_name, const Value& default_value = Value()) const {
        auto cat_it = registered_attributes.find(category);
        if (cat_it != registered_attributes.end()) {
            auto attr_it = cat_it->second.find(attr_name);
            if (attr_it != cat_it->second.end()) {
                return attr_it->second;
            }
        }
        return default_value;
    }
    
    std::vector<String> GetCategories() const {
        std::vector<String> categories;
        for (const auto& pair : registered_attributes) {
            categories.push_back(pair.first);
        }
        return categories;
    }
    
    std::vector<String> GetAttributes(const String& category) const {
        std::vector<String> attributes;
        auto it = registered_attributes.find(category);
        if (it != registered_attributes.end()) {
            for (const auto& pair : it->second) {
                attributes.push_back(pair.first);
            }
        }
        return attributes;
    }
};

GlobalAttrRegistry* GlobalAttrRegistry::instance = nullptr;

// Attribute serialization support
class AttrSerializer {
public:
    static String Serialize(const std::map<String, Value>& attributes) {
        String result = "{";
        bool first = true;
        for (const auto& pair : attributes) {
            if (!first) result += ",";
            result += "\"" + pair.first + "\":" + pair.second.ToString();
            first = false;
        }
        result += "}";
        return result;
    }
    
    static std::map<String, Value> Deserialize(const String& serialized) {
        std::map<String, Value> attributes;
        // In a real implementation, this would parse the JSON-like string
        // This is a simplified placeholder
        return attributes;
    }
    
    static void SaveToFile(const std::map<String, Value>& attributes, const String& filename) {
        // In a real implementation, this would save to a file
    }
    
    static std::map<String, Value> LoadFromFile(const String& filename) {
        // In a real implementation, this would load from a file
        return std::map<String, Value>();
    }
};

}