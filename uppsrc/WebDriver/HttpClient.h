#ifndef _WebDriver_HttpClient_h_
#define _WebDriver_HttpClient_h_

#include <Core/Core.h>


NAMESPACE_UPP

namespace detail {

struct HttpResponse {
	int http_code;
	String body;
};

struct IHttpClient : public SharedObjectBase {
	virtual ~IHttpClient() {}
	virtual HttpResponse Get(const String& url) const = 0;
	virtual HttpResponse Post(const String& url, const String& data) const = 0;
	virtual HttpResponse Delete(const String& url) const = 0;
};

class HttpClient : public IHttpClient {
public:
	virtual HttpResponse Get(const String& url) const override;
	virtual HttpResponse Post(const String& url, const String& data) const override;
	virtual HttpResponse Delete(const String& url) const override;
};

} // namespace detail

END_UPP_NAMESPACE

#endif