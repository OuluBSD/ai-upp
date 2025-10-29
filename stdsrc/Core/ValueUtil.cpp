#include "Core.h"
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <type_traits>

NAMESPACE_UPP

// Value utility functions

Value MakeValue(const std::string& str) {
    return Value(str);
}

Value MakeValue(int i) {
    return Value(i);
}

Value MakeValue(double d) {
    return Value(d);
}

Value MakeValue(bool b) {
    return Value(b);
}

Value MakeValue(const std::vector<Value>& vec) {
    return Value(vec);
}

bool IsNull(const Value& v) {
    return v.IsNull();
}

bool IsString(const Value& v) {
    return v.IsString();
}

bool IsInt(const Value& v) {
    return v.IsInt();
}

bool IsDouble(const Value& v) {
    return v.IsDouble();
}

bool IsBool(const Value& v) {
    return v.IsBool();
}

bool IsArray(const Value& v) {
    return v.IsArray();
}

std::string ValueToString(const Value& v, const std::string& default_value) {
    if (v.IsString()) {
        return v.ToString();
    } else if (v.IsInt()) {
        return std::to_string(v.ToInt());
    } else if (v.IsDouble()) {
        return std::to_string(v.ToDouble());
    } else if (v.IsBool()) {
        return v.ToBool() ? "true" : "false";
    }
    return default_value;
}

int ValueToInt(const Value& v, int default_value) {
    if (v.IsInt()) {
        return v.ToInt();
    } else if (v.IsDouble()) {
        return static_cast<int>(v.ToDouble());
    } else if (v.IsString()) {
        try {
            return std::stoi(v.ToString());
        } catch (...) {
            return default_value;
        }
    } else if (v.IsBool()) {
        return v.ToBool() ? 1 : 0;
    }
    return default_value;
}

double ValueToDouble(const Value& v, double default_value) {
    if (v.IsDouble()) {
        return v.ToDouble();
    } else if (v.IsInt()) {
        return static_cast<double>(v.ToInt());
    } else if (v.IsString()) {
        try {
            return std::stod(v.ToString());
        } catch (...) {
            return default_value;
        }
    } else if (v.IsBool()) {
        return v.ToBool() ? 1.0 : 0.0;
    }
    return default_value;
}

bool ValueToBool(const Value& v, bool default_value) {
    if (v.IsBool()) {
        return v.ToBool();
    } else if (v.IsInt()) {
        return v.ToInt() != 0;
    } else if (v.IsDouble()) {
        return v.ToDouble() != 0.0;
    } else if (v.IsString()) {
        std::string s = v.ToString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s == "true" || s == "1" || s == "yes" || s == "on";
    }
    return default_value;
}

std::vector<Value> ValueToArray(const Value& v, const std::vector<Value>& default_value) {
    if (v.IsArray()) {
        return v.ToArray();
    }
    return default_value;
}

// Container utilities for Value
bool Contains(const std::vector<Value>& container, const Value& item) {
    return std::find(container.begin(), container.end(), item) != container.end();
}

int FindIndex(const std::vector<Value>& container, const Value& item) {
    auto it = std::find(container.begin(), container.end(), item);
    if (it != container.end()) {
        return it - container.begin();
    }
    return -1;
}

// String-specific value utilities
ValueMap StringToValueMap(const std::string& json_str) {
    // This would parse a JSON string into a ValueMap
    // For this implementation, return an empty map
    return ValueMap();
}

std::string ValueMapToString(const ValueMap& map) {
    // This would serialize a ValueMap to JSON string
    // For this implementation, return empty string
    return "{}";
}

// Array utilities
Value ConcatArrays(const std::vector<Value>& arr1, const std::vector<Value>& arr2) {
    std::vector<Value> result = arr1;
    result.insert(result.end(), arr2.begin(), arr2.end());
    return Value(result);
}

std::vector<Value> SliceArray(const std::vector<Value>& arr, int start, int end) {
    if (start < 0) start = 0;
    if (end < 0) end = static_cast<int>(arr.size());
    if (start >= static_cast<int>(arr.size())) return {};
    if (end > static_cast<int>(arr.size())) end = arr.size();
    if (start >= end) return {};
    
    return std::vector<Value>(arr.begin() + start, arr.begin() + end);
}

// Value comparison
bool EqualValues(const Value& a, const Value& b) {
    return a == b;
}

int CompareValues(const Value& a, const Value& b) {
    if (a.IsNull() && b.IsNull()) return 0;
    if (a.IsNull()) return -1;
    if (b.IsNull()) return 1;
    
    // For this implementation, we'll do basic type-aware comparison
    if (a.IsInt() && b.IsInt()) return (a.ToInt() < b.ToInt()) ? -1 : (a.ToInt() > b.ToInt()) ? 1 : 0;
    if (a.IsDouble() && b.IsDouble()) return (a.ToDouble() < b.ToDouble()) ? -1 : (a.ToDouble() > b.ToDouble()) ? 1 : 0;
    if (a.IsString() && b.IsString()) return (a.ToString() < b.ToString()) ? -1 : (a.ToString() > b.ToString()) ? 1 : 0;
    if (a.IsBool() && b.IsBool()) return (a.ToBool() == b.ToBool()) ? 0 : (a.ToBool() ? 1 : -1);
    
    // Different types are considered unequal
    return -1; // Or some other indication of type mismatch
}

END_UPP_NAMESPACE