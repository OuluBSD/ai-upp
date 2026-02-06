#ifndef _WebDriver_Conversions_h_
#define _WebDriver_Conversions_h_

#include <Core/Core.h>



NAMESPACE_UPP

template<typename T>
Value To_json(const T& value);

template<typename T>
T From_json(const Value& value);

// Declaration for ValueArray specialization
template<>
ValueArray From_json<ValueArray>(const Value& value);

namespace conversions_detail {

struct Default_tag {};
struct Iterable_tag {};

using namespace detail;

template<typename T>
struct Tag :
	if_<is_iterable<T>, type_is<Iterable_tag>,
	type_is<Default_tag>
	> {};

template<typename T>
Value To_json_impl(const T& value, Default_tag) {
	// Compile error here usually indicates
	// that compiler doesn't know how to convert the type T
	// to the Value. Define Custom_to_json
	// function (see examples below) in the T's namespace
	// to resolve the issue.
	return Value(value);
}

template<typename T>
Value To_json_impl(const T& value, Iterable_tag) {
	typedef typename std::iterator_traits<decltype(std::begin(value))>::value_type Item;
	Value result = Value::Array();
	ValueArray& dst = result;
	std::transform(std::begin(value), std::end(value), std::back_inserter(dst), [](const Item& item) {
		return To_json(item);
	});
	return result;
}

} // conversions_detail

inline
Value Custom_to_json(const char* value) {
	return Value(value);
}

inline
Value Custom_to_json(const String& value) {
	return To_json(value.CStr());
}

inline
Value Custom_to_json(const Value& value) {
	return value;
}

inline
Value Custom_to_json(const ValueMap& value) {
	return Value(value);
}


inline
Value Custom_to_json(int value) {
	return Value(static_cast<double>(value));
}

template<typename T>
Value Custom_to_json(const T& value) {
	using conversions_detail::To_json_impl;
	using conversions_detail::Tag;
	return To_json_impl(value, typename Tag<T>::type());
}

template<typename T>
Value To_json(const T& value) {
	return Custom_to_json(value);
}

///////////////////////////////////////////////////////////////////

namespace conversions_detail {

template<typename T>
void From_json_impl(const Value& value, T& result, Default_tag) {
	// Compile error here usually indicates
	// that compiler doesn't know how to convert the Value
	// to the type T. Define Custom_from_json function (see examples below)
	// in the T's namespace to resolve the issue.
	result = value.Get<T>();
}

template<typename T>
void From_json_impl(const Value& value, T& result, Iterable_tag) {
	WEBDRIVERXX_CHECK(value.IsArray(), "Value is not an array");
	const ValueArray& array = value.Get<ValueArray>();
	typedef typename std::iterator_traits<decltype(std::begin(result))>::value_type Item;
	std::transform(array.Begin(), array.End(), std::back_inserter(result), From_json<Item>);
}

} // conversions_detail

inline
void Custom_from_json(const Value& value, String& result) {
	result = value.ToString();
}

inline
void Custom_from_json(const Value& value, bool& result) {
	result = value.ToBool();
}

inline
void Custom_from_json(const Value& value, int& result) {
	WEBDRIVERXX_CHECK(value.IsNumber(), "Value is not a number");
	result = value.ToInt();
}

inline
void Custom_from_json(const Value& value, unsigned& result) {
	WEBDRIVERXX_CHECK(value.IsNumber(), "Value is not a number");
	result = static_cast<unsigned>(value.ToDouble());
}

inline
void Custom_from_json(const Value& value, Value& result) {
	result = value;
}

inline
void Custom_from_json(const Value& value, ValueMap& result) {
	WEBDRIVERXX_CHECK(value.IsObject(), "Value is not an object");
	result = value.Get<ValueMap>();
}


template<typename T>
void Custom_from_json(const Value& value, T& result) {
	using conversions_detail::From_json_impl;
	using conversions_detail::Tag;
	return From_json_impl(value, result, typename Tag<T>::type());
}

template<typename T>
T From_json(const Value& value) {
	T result;
	Custom_from_json(value, result);
	return result;
}

template<typename T>
T Optional_from_json(const Value& value, const T& default_value = T()) {
	return IsNull(value) ? default_value : From_json<T>(value);
}

///////////////////////////////////////////////////////////////////

inline
Value Custom_to_json(const Size& size) {
	ValueMap obj;
	obj.Add("width", static_cast<double>(size.width));
	obj.Add("height", static_cast<double>(size.height));
	return Value(obj);
}

inline
void Custom_from_json(const Value& value, Size& result) {
	WEBDRIVERXX_CHECK(value.IsObject(), "Size is not an object");
	const ValueMap& obj = value.Get<ValueMap>();
	Value width_val = obj.Get("width", Value());
	if (!IsNull(width_val)) result.width = static_cast<int>(width_val.ToDouble());
	Value height_val = obj.Get("height", Value());
	if (!IsNull(height_val)) result.height = static_cast<int>(height_val.ToDouble());
}

inline
Value Custom_to_json(const Point& position) {
	ValueMap obj;
	obj.Add("x", static_cast<double>(position.x));
	obj.Add("y", static_cast<double>(position.y));
	return Value(obj);
}

inline
void Custom_from_json(const Value& value, Point& result) {
	WEBDRIVERXX_CHECK(value.IsObject(), "Point is not an object");
	const ValueMap& obj = value.Get<ValueMap>();
	Value x_val = obj.Get("x", Value());
	if (!IsNull(x_val)) result.x = static_cast<int>(x_val.ToDouble());
	Value y_val = obj.Get("y", Value());
	if (!IsNull(y_val)) result.y = static_cast<int>(y_val.ToDouble());
}

inline
Value Custom_to_json(const Cookie& cookie) {
	ValueMap obj;
	obj.Add("name", cookie.name);
	obj.Add("value", cookie.value);
	if (!cookie.path.IsEmpty()) obj.Add("path", cookie.path);
	if (!cookie.domain.IsEmpty()) obj.Add("domain", cookie.domain);
	if (cookie.secure) obj.Add("secure", true);
	if (cookie.http_only) obj.Add("httpOnly", true);
	if (cookie.expiry != Cookie::No_expiry) obj.Add("expiry", static_cast<double>(cookie.expiry));
	return Value(obj);
}

inline
void Custom_from_json(const Value& value, Cookie& result) {
	WEBDRIVERXX_CHECK(value.IsObject(), "Cookie is not an object");
	const ValueMap& obj = value.Get<ValueMap>();
	Value name_val = obj.Get("name", Value());
	if (!IsNull(name_val)) result.name = name_val.ToString();
	Value value_val = obj.Get("value", Value());
	if (!IsNull(value_val)) result.value = value_val.ToString();
	Value path_val = obj.Get("path", Value());
	if (!IsNull(path_val)) result.path = path_val.ToString();
	Value domain_val = obj.Get("domain", Value());
	if (!IsNull(domain_val)) result.domain = domain_val.ToString();
	Value secure_val = obj.Get("secure", Value());
	if (!IsNull(secure_val)) result.secure = secure_val.ToBool();
	Value http_only_val = obj.Get("httpOnly", Value());
	if (!IsNull(http_only_val)) result.http_only = http_only_val.ToBool();
	Value expiry_val = obj.Get("expiry", Value());
	if (!IsNull(expiry_val)) result.expiry = static_cast<int>(expiry_val.ToDouble());
}

// Helper function to convert Upp::Value to JSON string
inline String AsJSON(const Value& val) {
	return AsJSON(val);
}

END_UPP_NAMESPACE

#endif