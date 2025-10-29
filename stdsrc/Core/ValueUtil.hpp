#pragma once
#ifndef _Core_ValueUtil_hpp_
#define _Core_ValueUtil_hpp_

#include <vector>
#include <string>
#include "Core.h"

NAMESPACE_UPP

// Value utility functions
Value MakeValue(const std::string& str);
Value MakeValue(int i);
Value MakeValue(double d);
Value MakeValue(bool b);
Value MakeValue(const std::vector<Value>& vec);

bool IsNull(const Value& v);
bool IsString(const Value& v);
bool IsInt(const Value& v);
bool IsDouble(const Value& v);
bool IsBool(const Value& v);
bool IsArray(const Value& v);

std::string ValueToString(const Value& v, const std::string& default_value = "");
int ValueToInt(const Value& v, int default_value = 0);
double ValueToDouble(const Value& v, double default_value = 0.0);
bool ValueToBool(const Value& v, bool default_value = false);
std::vector<Value> ValueToArray(const Value& v, const std::vector<Value>& default_value = {});

// Container utilities for Value
bool Contains(const std::vector<Value>& container, const Value& item);
int FindIndex(const std::vector<Value>& container, const Value& item);

// String-specific value utilities
ValueMap StringToValueMap(const std::string& json_str);
std::string ValueMapToString(const ValueMap& map);

// Array utilities
Value ConcatArrays(const std::vector<Value>& arr1, const std::vector<Value>& arr2);
std::vector<Value> SliceArray(const std::vector<Value>& arr, int start = 0, int end = -1);

// Value comparison
bool EqualValues(const Value& a, const Value& b);
int CompareValues(const Value& a, const Value& b);

END_UPP_NAMESPACE

#endif