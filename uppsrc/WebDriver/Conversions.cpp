#include "WebDriver.h"

NAMESPACE_UPP

// Implementation for Conversions will go here

// Explicit instantiation of the specialization
template <>
ValueArray From_json<ValueArray>(const Value& value) {
	ValueArray result;
	if (value.IsArray()) {
		const ValueArray& arr = value.Get<ValueArray>();
		for (const auto& item : arr) {
			result.Add(item);
		}
	}
	return result;
}

END_UPP_NAMESPACE