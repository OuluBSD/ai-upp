#include "WebDriver.h"

NAMESPACE_UPP

// Implementation for Conversions will go here

// Explicit instantiation of the specialization
template <>
ValueArray From_json<ValueArray>(const picojson::value& value) {
	ValueArray result;
	if (value.is<picojson::array>()) {
		const picojson::array& arr = value.get<picojson::array>();
		for (const auto& item : arr) {
			result.Add(ToValue(item));
		}
	}
	return result;
}

END_UPP_NAMESPACE