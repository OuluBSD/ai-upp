#ifndef _WebDriver_Conversions_h_
#define _WebDriver_Conversions_h_

#include <Core/Core.h>



NAMESPACE_UPP

template<typename T>
picojson::value To_json(const T& value);

template<typename T>
T From_json(const picojson::value& value);

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
picojson::value To_json_impl(const T& value, Default_tag) {
	// Compile error here usually indicates
	// that compiler doesn't know how to convert the type T
	// to the picojson::value. Define Custom_to_json
	// function (see examples below) in the T's namespace
	// to resolve the issue.
	return picojson::value(value);
}

template<typename T>
picojson::value To_json_impl(const T& value, Iterable_tag) {
	typedef typename std::iterator_traits<decltype(std::begin(value))>::value_type Item;
	picojson::value result = picojson::value(picojson::array());
	picojson::array& dst = result.get<picojson::array>();
	std::transform(std::begin(value), std::end(value), std::back_inserter(dst), [](const Item& item) {
		return To_json(item);
	});
	return result;
}

} // conversions_detail

inline
picojson::value Custom_to_json(const char* value) {
	return picojson::value(value);
}

inline
picojson::value Custom_to_json(const String& value) {
	return To_json(value.CStr());
}

inline
picojson::value Custom_to_json(const picojson::value& value) {
	return value;
}

inline
picojson::value Custom_to_json(const picojson::object& value) {
	return picojson::value(value);
}


inline
picojson::value Custom_to_json(int value) {
	return picojson::value(static_cast<double>(value));
}

template<typename T>
picojson::value Custom_to_json(const T& value) {
	using conversions_detail::To_json_impl;
	using conversions_detail::Tag;
	return To_json_impl(value, typename Tag<T>::type());
}

template<typename T>
picojson::value To_json(const T& value) {
	return Custom_to_json(value);
}

///////////////////////////////////////////////////////////////////

namespace conversions_detail {

template<typename T>
void From_json_impl(const picojson::value& value, T& result, Default_tag) {
	// Compile error here usually indicates
	// that compiler doesn't know how to convert the picojson::value
	// to the type T. Define Custom_from_json function (see examples below)
	// in the T's namespace to resolve the issue.
	result = value.get<T>();
}

template<typename T>
void From_json_impl(const picojson::value& value, T& result, Iterable_tag) {
	WEBDRIVERXX_CHECK(value.is<picojson::array>(), "Value is not an array");
	const picojson::array& array = value.get<picojson::array>();
	typedef typename std::iterator_traits<decltype(std::begin(result))>::value_type Item;
	std::transform(array.begin(), array.end(), std::back_inserter(result), From_json<Item>);
}

} // conversions_detail

inline
void Custom_from_json(const picojson::value& value, String& result) {
	result = value.to_str();
}

inline
void Custom_from_json(const picojson::value& value, bool& result) {
	result = value.evaluate_as_boolean();
}

inline
void Custom_from_json(const picojson::value& value, int& result) {
	WEBDRIVERXX_CHECK(value.is<double>(), "Value is not a number");
	result = static_cast<int>(value.get<double>());
}

inline
void Custom_from_json(const picojson::value& value, unsigned& result) {
	WEBDRIVERXX_CHECK(value.is<double>(), "Value is not a number");
	result = static_cast<unsigned>(value.get<double>());
}

inline
void Custom_from_json(const picojson::value& value, picojson::value& result) {
	result = value;
}

inline
void Custom_from_json(const picojson::value& value, picojson::object& result) {
	WEBDRIVERXX_CHECK(value.is<picojson::object>(), "Value is not an object");
	result = value.get<picojson::object>();
}


template<typename T>
void Custom_from_json(const picojson::value& value, T& result) {
	using conversions_detail::From_json_impl;
	using conversions_detail::Tag;
	return From_json_impl(value, result, typename Tag<T>::type());
}

template<typename T>
T From_json(const picojson::value& value) {
	T result;
	Custom_from_json(value, result);
	return result;
}

template<typename T>
T Optional_from_json(const picojson::value& value, const T& default_value = T()) {
	return value.is<picojson::null>() ? default_value : From_json<T>(value);
}

///////////////////////////////////////////////////////////////////

inline
picojson::value Custom_to_json(const Size& size) {
	picojson::object obj;
	obj["width"] = picojson::value(static_cast<double>(size.width));
	obj["height"] = picojson::value(static_cast<double>(size.height));
	return picojson::value(obj);
}

inline
void Custom_from_json(const picojson::value& value, Size& result) {
	WEBDRIVERXX_CHECK(value.is<picojson::object>(), "Size is not an object");
	const picojson::object& obj = value.get<picojson::object>();
	auto it = obj.find("width");
	if (it != obj.end()) result.width = static_cast<int>(it->second.get<double>());
	it = obj.find("height");
	if (it != obj.end()) result.height = static_cast<int>(it->second.get<double>());
}

inline
picojson::value Custom_to_json(const Point& position) {
	picojson::object obj;
	obj["x"] = picojson::value(static_cast<double>(position.x));
	obj["y"] = picojson::value(static_cast<double>(position.y));
	return picojson::value(obj);
}

inline
void Custom_from_json(const picojson::value& value, Point& result) {
	WEBDRIVERXX_CHECK(value.is<picojson::object>(), "Point is not an object");
	const picojson::object& obj = value.get<picojson::object>();
	auto it = obj.find("x");
	if (it != obj.end()) result.x = static_cast<int>(it->second.get<double>());
	it = obj.find("y");
	if (it != obj.end()) result.y = static_cast<int>(it->second.get<double>());
}

inline
picojson::value Custom_to_json(const Cookie& cookie) {
	picojson::object obj;
	obj["name"] = picojson::value(cookie.name);
	obj["value"] = picojson::value(cookie.value);
	if (!cookie.path.IsEmpty()) obj["path"] = picojson::value(cookie.path);
	if (!cookie.domain.IsEmpty()) obj["domain"] = picojson::value(cookie.domain);
	if (cookie.secure) obj["secure"] = picojson::value(true);
	if (cookie.http_only) obj["httpOnly"] = picojson::value(true);
	if (cookie.expiry != Cookie::No_expiry) obj["expiry"] = picojson::value(static_cast<double>(cookie.expiry));
	return picojson::value(obj);
}

inline
void Custom_from_json(const picojson::value& value, Cookie& result) {
	WEBDRIVERXX_CHECK(value.is<picojson::object>(), "Cookie is not an object");
	const picojson::object& obj = value.get<picojson::object>();
	auto it = obj.find("name");
	if (it != obj.end()) result.name = it->second.to_str();
	it = obj.find("value");
	if (it != obj.end()) result.value = it->second.to_str();
	it = obj.find("path");
	if (it != obj.end()) result.path = it->second.to_str();
	it = obj.find("domain");
	if (it != obj.end()) result.domain = it->second.to_str();
	it = obj.find("secure");
	if (it != obj.end()) result.secure = it->second.get<bool>();
	it = obj.find("httpOnly");
	if (it != obj.end()) result.http_only = it->second.get<bool>();
	it = obj.find("expiry");
	if (it != obj.end()) result.expiry = static_cast<int>(it->second.get<double>());
}

// Helper function to convert picojson::value to Upp::Value
inline Value ToValue(const picojson::value& pico_val) {
	if (pico_val.is<bool>()) {
		return Value(pico_val.get<bool>());
	} else if (pico_val.is<double>()) {
		return Value(pico_val.get<double>());
	} else if (pico_val.is<std::string>()) {
		return Value(String(pico_val.to_str().c_str()));
	} else if (pico_val.is<picojson::object>()) {
		Value result = Value::Object();
		const picojson::object& obj = pico_val.get<picojson::object>();
		for (const auto& pair : obj) {
			result.Add(String(pair.first.c_str()), ToValue(pair.second));
		}
		return result;
	} else if (pico_val.is<picojson::array>()) {
		Value result = Value::Array();
		const picojson::array& arr = pico_val.get<picojson::array>();
		for (const auto& item : arr) {
			result.Add(ToValue(item));
		}
		return result;
	} else {
		return Value(); // null value
	}
}

// Helper function to convert Upp::Value to picojson::value
inline picojson::value ToPicoJson(const Value& upp_val) {
	if (upp_val.IsVoid()) {
		return picojson::value();
	} else if (upp_val.IsBool()) {
		return picojson::value(upp_val.Get<bool>());
	} else if (upp_val.IsInt()) {
		return picojson::value(static_cast<double>(upp_val.Get<int>()));
	} else if (upp_val.IsDouble()) {
		return picojson::value(upp_val.Get<double>());
	} else if (upp_val.IsString()) {
		return picojson::value(upp_val.Get<String>().ToStd());
	} else if (upp_val.IsArray()) {
		picojson::array arr;
		for (int i = 0; i < upp_val.GetCount(); ++i) {
			arr.push_back(ToPicoJson(upp_val[i]));
		}
		return picojson::value(arr);
	} else if (upp_val.IsObject()) {
		picojson::object obj;
		for (const auto& key : upp_val.GetKeys()) {
			obj[key.ToStd()] = ToPicoJson(upp_val[key]);
		}
		return picojson::value(obj);
	} else {
		return picojson::value(); // null value
	}
}

END_UPP_NAMESPACE

#endif