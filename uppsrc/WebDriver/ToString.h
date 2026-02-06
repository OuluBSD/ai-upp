#ifndef _WebDriver_ToString_h_
#define _WebDriver_ToString_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

template<typename T>
String To_string(const T& value) {
	return AsString(value);
}

// Specializations for common types
template<>
inline String To_string<bool>(const bool& value) {
	return value ? "true" : "false";
}

template<>
inline String To_string<int>(const int& value) {
	return AsString(value);
}

template<>
inline String To_string<double>(const double& value) {
	return AsString(value);
}

template<>
inline String To_string<String>(const String& value) {
	return value;
}

} // namespace detail

END_UPP_NAMESPACE

#endif