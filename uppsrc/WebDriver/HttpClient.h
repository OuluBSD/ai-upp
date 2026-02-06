#ifndef _WebDriver_HttpClient_h_
#define _WebDriver_HttpClient_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

struct Http_response {
	int http_code;
	String body;
};

struct IHttp_client {
	virtual ~IHttp_client() {}
	virtual Http_response Get(const String& url) const = 0;
	virtual Http_response Post(const String& url, const String& data) const = 0;
	virtual Http_response Delete(const String& url) const = 0;
};

class Http_client : IHttp_client {
public:
	virtual Http_response Get(const String& url) const override;
	virtual Http_response Post(const String& url, const String& data) const override;
	virtual Http_response Delete(const String& url) const override;
};

} // namespace detail

END_UPP_NAMESPACE

#endif