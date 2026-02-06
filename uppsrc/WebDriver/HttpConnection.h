#ifndef _WebDriver_HttpConnection_h_
#define _WebDriver_HttpConnection_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

class Http_connection {
public:
	Http_connection(const String& host, int port);
	
	Http_response Get(const String& path) const;
	Http_response Post(const String& path, const String& data) const;
	Http_response Delete(const String& path) const;

private:
	String host_;
	int port_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif