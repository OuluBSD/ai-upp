#include "WebDriver.h"

NAMESPACE_UPP

Js_args::Js_args() {
}

picojson::value Js_args::operator[](int index) const {
	if (index >= 0 && index < args_.size()) {
		return args_[index];
	}
	return picojson::value();
}

int Js_args::Size() const {
	return static_cast<int>(args_.size());
}

END_UPP_NAMESPACE