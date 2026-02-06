#include "WebDriver.h"

NAMESPACE_UPP

namespace detail {

HttpResponse HttpClient::Get(const String& url) const {
	Upp::HttpRequest client(url);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

HttpResponse HttpClient::Post(const String& url, const String& data) const {
	Upp::HttpRequest client(url);
	client.Post(data);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

HttpResponse HttpClient::Delete(const String& url) const {
	Upp::HttpRequest client(url);
	client.Method(Upp::HttpRequest::METHOD_DELETE);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

} // namespace detail

END_UPP_NAMESPACE