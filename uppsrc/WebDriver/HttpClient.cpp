#include "WebDriver.h"

NAMESPACE_UPP

namespace detail {

Http_response Http_client::Get(const String& url) const {
	Upp::HttpRequest client(url);
	Http_response resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

Http_response Http_client::Post(const String& url, const String& data) const {
	Upp::HttpRequest client(url);
	client.Post(data);
	Http_response resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

Http_response Http_client::Delete(const String& url) const {
	Upp::HttpRequest client(url);
	client.Method(Upp::HttpRequest::METHOD_DELETE);
	Http_response resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

} // namespace detail

END_UPP_NAMESPACE