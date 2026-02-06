#ifndef _WebDriver_JsArgs_h_
#define _WebDriver_JsArgs_h_

#include <Core/Core.h>


NAMESPACE_UPP

class Js_args { // copyable
public:
	Js_args();

	template<typename T>
	Js_args(const T& value);

	template<typename T>
	Js_args& Add(const T& value);

	picojson::value operator[](int index) const;

	int Size() const;

private:
	Vector<picojson::value> args_;
};

template<typename T>
Js_args::Js_args(const T& value) {
	Add(value);
}

template<typename T>
Js_args& Js_args::Add(const T& value) {
	args_.Add(To_json(value));
	return *this;
}

END_UPP_NAMESPACE

#endif