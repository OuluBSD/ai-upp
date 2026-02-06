#ifndef _WebDriver_Conversions_h_
#define _WebDriver_Conversions_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Forward declarations for JSON conversion functions
template<typename T>
T From_json(const Value& value) {
	T var;
	LoadFromJsonValue(var, value);
	return var;
}

template<typename T>
Value To_json(const T& value) {
	return StoreAsJsonValue(value);
}

// Specialization for Value
template<>
inline Value From_json<Value>(const Value& value) {
	return value;
}

template<>
inline Value To_json<Value>(const Value& value) {
	return value;
}

// Specialization for String
template<>
inline String From_json<String>(const Value& value) {
	return IsString(value) ? (String)value : String();
}

template<>
inline Value To_json<String>(const String& value) {
	return Value(value);
}

// Specialization for int
template<>
inline int From_json<int>(const Value& value) {
	return IsNumber(value) ? (int)value : 0;
}

template<>
inline Value To_json<int>(const int& value) {
	return Value(value);
}

// Specialization for bool
template<>
inline bool From_json<bool>(const Value& value) {
	return IsNumber(value) ? (bool)value : false;
}

template<>
inline Value To_json<bool>(const bool& value) {
	return Value(value);
}

// Specialization for double
template<>
inline double From_json<double>(const Value& value) {
	return IsNumber(value) ? (double)value : 0.0;
}

template<>
inline Value To_json<double>(const double& value) {
	return Value(value);
}

// Specialization for ValueArray
template<>
inline ValueArray From_json<ValueArray>(const Value& value) {
	return value.Is<ValueArray>() ? value.Get<ValueArray>() : ValueArray();
}

template<>
inline Value To_json<ValueArray>(const ValueArray& value) {
	return Value(value);
}

// Specialization for ValueMap
template<>
inline ValueMap From_json<ValueMap>(const Value& value) {
	return value.Is<ValueMap>() ? value.Get<ValueMap>() : ValueMap();
}

template<>
inline Value To_json<ValueMap>(const ValueMap& value) {
	return Value(value);
}

END_UPP_NAMESPACE

#endif