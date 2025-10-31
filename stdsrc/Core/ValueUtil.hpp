#pragma once
#ifndef _Core_ValueUtil_hpp_
#define _Core_ValueUtil_hpp_

#include <vector>
#include <string>
#include <map>
#include "Core.h"

NAMESPACE_UPP

inline
void ValueMap::Add(const Value& key, const Value& value) {
	Data& d = UnShare();
	d.key.Add(key);
	d.value.Add(value);
}

inline
int StdValueCompare(const Value& a, const Value& b, const LanguageInfo& f)
{
	return IsString(a) && IsString(b) ? CompareStrings(a, b, f) : a.Compare(b);
}

inline
int StdValueCompare(const Value& a, const Value& b, int language)
{
	return StdValueCompare(a, b, GetLanguageInfo(language));
}

inline
int StdValueCompare(const Value& a, const Value& b)
{
	return StdValueCompare(a, b, GetLanguageInfo());
}

inline
int StdValueCompareDesc(const Value& a, const Value& b, const LanguageInfo& f)
{
	return -StdValueCompare(a, b, f);
}

inline
int StdValueCompareDesc(const Value& a, const Value& b, int language)
{
	return -StdValueCompare(a, b, language);
}

inline
int StdValueCompareDesc(const Value& a, const Value& b)
{
	return -StdValueCompare(a, b);
}

#ifdef DEPRECATED
template <class T>
struct RawRef : public RefManager {
	virtual void  SetValue(void *p, const Value& v)       { *(T *) p = RawValue<T>::Extract(v); }
	virtual Value GetValue(const void *p)                 { return RawValue<T>(*(const T *) p); }
	virtual int   GetType()                               { return GetValueTypeNo<T>(); }
	virtual ~RawRef() {}
};

template <class T>
Ref RawAsRef(T& x) {
	return Ref(&x, &Single< RawRef<T> >());
}

template <class T>
struct RichRef : public RawRef<T> {
	virtual Value GetValue(const void *p)                 { return RichToValue(*(T *) p); }
	virtual bool  IsNull(const void *p)                   { return UPP::IsNull(*(T *) p); }
	virtual void  SetValue(void *p, const Value& v)       { *(T *) p = T(v); }
	virtual void  SetNull(void *p)                        { UPP::SetNull(*(T*)p); }
};

template <class T>
Ref RichAsRef(T& x) {
	return Ref(&x, &Single< RichRef<T> >());
}
#endif

// Additional Value utility functions
inline Value MakeValue(const std::string& str) { return Value(str); }
inline Value MakeValue(int i) { return Value(i); }
inline Value MakeValue(double d) { return Value(d); }
inline Value MakeValue(bool b) { return Value(b); }
inline Value MakeValue(const std::vector<Value>& vec) { 
    ValueArray arr;
    for (const auto& v : vec) {
        arr.Add(v);
    }
    return Value(arr);
}

inline bool IsNull(const Value& v) { return UPP::IsNull(v); }
inline bool IsString(const Value& v) { return v.Is<String>(); }
inline bool IsInt(const Value& v) { return v.Is<int>(); }
inline bool IsDouble(const Value& v) { return v.Is<double>(); }
inline bool IsBool(const Value& v) { return v.Is<bool>(); }
inline bool IsArray(const Value& v) { return v.Is<ValueArray>(); }

inline std::string ValueToString(const Value& v, const std::string& default_value = "") {
    return v.Is<String>() ? v.ToString() : default_value;
}

inline int ValueToInt(const Value& v, int default_value = 0) {
    return v.Is<int>() ? (int)v : default_value;
}

inline double ValueToDouble(const Value& v, double default_value = 0.0) {
    return v.Is<double>() ? (double)v : default_value;
}

inline bool ValueToBool(const Value& v, bool default_value = false) {
    return v.Is<bool>() ? (bool)v : default_value;
}

inline std::vector<Value> ValueToArray(const Value& v, const std::vector<Value>& default_value = {}) {
    if (v.Is<ValueArray>()) {
        const ValueArray& arr = v;
        std::vector<Value> result;
        result.reserve(arr.GetCount());
        for (int i = 0; i < arr.GetCount(); ++i) {
            result.push_back(arr[i]);
        }
        return result;
    }
    return default_value;
}

// Container utilities for Value
inline bool Contains(const std::vector<Value>& container, const Value& item) {
    return std::find(container.begin(), container.end(), item) != container.end();
}

inline int FindIndex(const std::vector<Value>& container, const Value& item) {
    auto it = std::find(container.begin(), container.end(), item);
    return it != container.end() ? static_cast<int>(it - container.begin()) : -1;
}

// String-specific value utilities
inline ValueMap StringToValueMap(const std::string& json_str) {
    ValueMap map;
    // TODO: Implement JSON parsing
    return map;
}

inline std::string ValueMapToString(const ValueMap& map) {
    // TODO: Implement JSON serialization
    return "{}";
}

// Array utilities
inline Value ConcatArrays(const std::vector<Value>& arr1, const std::vector<Value>& arr2) {
    ValueArray result;
    for (const auto& v : arr1) {
        result.Add(v);
    }
    for (const auto& v : arr2) {
        result.Add(v);
    }
    return Value(result);
}

inline std::vector<Value> SliceArray(const std::vector<Value>& arr, int start = 0, int end = -1) {
    std::vector<Value> result;
    int size = static_cast<int>(arr.size());
    
    if (start < 0) start = 0;
    if (end < 0 || end > size) end = size;
    if (start >= end) return result;
    
    result.reserve(end - start);
    for (int i = start; i < end; ++i) {
        result.push_back(arr[i]);
    }
    return result;
}

// Value comparison
inline bool EqualValues(const Value& a, const Value& b) {
    return a == b;
}

inline int CompareValues(const Value& a, const Value& b) {
    return a.Compare(b);
}

END_UPP_NAMESPACE

#endif