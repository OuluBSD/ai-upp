#include "WebDriver.h"

NAMESPACE_UPP

Js_args::Js_args() {
}

Value Js_args::operator[](int index) const {
	if (index >= 0 && index < args_.GetCount()) {
		return args_[index];
	}
	return Value();
}

int Js_args::Size() const {
	return static_cast<int>(args_.GetCount());
}

END_UPP_NAMESPACE