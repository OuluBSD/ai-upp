#ifndef _WebDriver_HttpConnection_h_
#define _WebDriver_HttpConnection_h_

#include <Core/Core.h>

NAMESPACE_UPP

namespace detail {

class HttpConnection {
public:
	HttpConnection(const String& host, int port);
	
	HttpResponse Get(const String& path) const;
	HttpResponse Post(const String& path, const String& data) const;
	HttpResponse Delete(const String& path) const;

private:
	String host_;
	int port_;
};

} // namespace detail

END_UPP_NAMESPACE

#endif