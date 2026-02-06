#ifndef _WebDriver_Types_h_
#define _WebDriver_Types_h_

#include <Core/Core.h>


NAMESPACE_UPP

typedef unsigned long long Time_point;
typedef unsigned Duration;

#if 0
struct Size {
	int width;
	int height;
	Size() : width(0), height(0) {}
};

struct Point {
	int x;
	int y;
	Point() : x(0), y(0) {}
	Point(int x, int y) : x(x), y(y) {}
};
#endif

typedef Point Offset;

struct Cookie {
	enum {
		No_expiry = 0
	};

	String name;
	String value;
	String path;
	String domain;
	bool secure;
	bool http_only;
	int expiry; // seconds since midnight, January 1, 1970 UTC

	Cookie() : secure(false), http_only(false), expiry(No_expiry) {}
	Cookie(
		const String& name,
		const String& value,
		const String& path = String(),
		const String& domain = String(),
		bool secure = false,
		bool http_only = false,
		int expiry = No_expiry
		)
		: name(name)
		, value(value)
		, path(path)
		, domain(domain)
		, secure(secure)
		, http_only(http_only)
		, expiry(expiry)
	{}

	bool operator == (const Cookie& c) const {
		return name == c.name
			&& value == c.value
			&& path == c.path
			&& domain == c.domain
			&& secure == c.secure
			&& http_only == c.http_only
			&& expiry == c.expiry
			;
	}
};

namespace timeout {

typedef const char* Type;

Type const Implicit = "implicit";
Type const Page_load = "page load";
Type const Script = "script";

} // namespace timeout

namespace mouse {
enum Button {
	Left_button = 0,
	Middle_button = 1,
	Right_button = 2
};
} // namespace mouse

END_UPP_NAMESPACE

#endif