#include "WebDriver.h"

NAMESPACE_UPP

namespace detail {

HttpConnection::HttpConnection(const String& host, int port)
	: host_(host)
	, port_(port)
{
}

HttpResponse HttpConnection::Get(const String& path) const {
	Upp::HttpRequest client(host_ + ":" + AsString(port_) + path);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

HttpResponse HttpConnection::Post(const String& path, const String& data) const {
	Upp::HttpRequest client(host_ + ":" + AsString(port_) + path);
	client.Post(data);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

HttpResponse HttpConnection::Delete(const String& path) const {
	Upp::HttpRequest client(host_ + ":" + AsString(port_) + path);
	client.Method(Upp::HttpRequest::METHOD_DELETE);
	HttpResponse resp;
	resp.body = client.Execute();
	resp.http_code = client.GetStatusCode();
	return resp;
}

} // namespace detail

END_UPP_NAMESPACE