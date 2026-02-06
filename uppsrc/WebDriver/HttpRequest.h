#ifndef _WebDriver_HttpRequest_h_
#define _WebDriver_HttpRequest_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

struct Http_request {
	String method;
	String url;
	Vector<String> headers;
	String body;
	
	Http_request(const String& method, const String& url);
};

} // namespace detail

END_UPP_NAMESPACE

#endif