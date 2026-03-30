#include "Core.h"

#include <cstdio>
#include <iostream>

NAMESPACE_UPP

static const int MLUI_PROTOCOL_VERSION = 1;

static bool MluiParsePort(const String& text, int& out_port)
{
	const char *end = NULL;
	int port = ScanInt(text, &end);
	if(!end || *end || port <= 0 || port > 65535)
		return false;
	out_port = port;
	return true;
}

static bool MluiParseInt(const String& text, int& out_value)
{
	const char *end = NULL;
	int n = ScanInt(text, &end);
	if(!end || *end)
		return false;
	out_value = n;
	return true;
}

bool MluiParseBindAddress(const String& spec, String& host, int& port)
{
	String s = TrimBoth(spec);
	if(s.IsEmpty())
		return false;

	int q = s.ReverseFind(':');
	if(q >= 0) {
		host = TrimBoth(s.Left(q));
		String p = TrimBoth(s.Mid(q + 1));
		if(!MluiParsePort(p, port))
			return false;
		if(host.IsEmpty())
			host = "0.0.0.0";
		return true;
	}

	if(MluiParsePort(s, port)) {
		host = "0.0.0.0";
		return true;
	}

	host = s;
	port = 8082;
	return true;
}

bool MluiParseJsonRequest(const String& request_json, MluiRequest& out, String& error)
{
	Value req = ParseJSON(request_json);
	if(req.IsError() || !IsValueMap(req)) {
		error = "Invalid JSON request";
		return false;
	}

	ValueMap req_map = req;
	out.id = req_map.Get("id", Value());
	out.method = req_map.Get("method", Value());
	if(out.method.IsEmpty())
		out.method = req_map.Get("action", Value());
	if(out.method.IsEmpty()) {
		error = "Missing method/action";
		return false;
	}

	Value params = req_map.Get("params", ValueMap());
	out.params.Clear();
	if(IsValueMap(params))
		out.params = params;

	error.Clear();
	return true;
}

String MluiMakeJsonResponse(bool ok, const Value& id, const Value& result, const String& error)
{
	ValueMap out;
	if(!IsNull(id))
		out.Add("id", id);
	out.Add("protocol_version", MLUI_PROTOCOL_VERSION);
	out.Add("ok", ok);
	if(ok)
		out.Add("result", result);
	else
		out.Add("error", error);
	return AsJSON(out);
}

static void MluiWarmupValueTypes()
{
	// Value type registrations must happen in ST context before worker threads use Value/JSON.
	ValueArray arr;
	arr.Add(1);
	arr.Add("x");
	ValueMap map;
	map.Add("id", 0);
	map.Add("ok", true);
	map.Add("arr", arr);
	map.Add("time", GetSysTime());
	(void)AsJSON(map);
}

String MluiRuntime::TrimCR(const String& s)
{
	if(!s.IsEmpty() && s[s.GetCount() - 1] == '\r')
		return s.Left(s.GetCount() - 1);
	return s;
}

void MluiRuntime::SetJsonHandler(const MluiJsonHandler& handler)
{
	json_handler = handler;
}

void MluiRuntime::Configure(bool stdio, bool tcp, const String& host, int port)
{
	use_stdio = stdio;
	use_tcp = tcp;
	bind_host = host;
	bind_port = port;
}

bool MluiRuntime::IsStarted() const
{
	return started != 0;
}

void MluiRuntime::StdioLoop()
{
	std::fprintf(stderr, "MLUI: stdin/stdout mode started\n");
	std::string line;
	while(!stop) {
		if(!std::getline(std::cin, line)) {
			std::cin.clear();
			Sleep(100);
			continue;
		}
		String req(line.c_str());
		if(req.IsEmpty())
			continue;
		String resp = json_handler ? json_handler(req)
		                           : MluiMakeJsonResponse(false, Value(), Value(), "No MLUI JSON handler");
		std::cout << resp.ToStd() << "\n";
		std::cout.flush();
	}
}

void MluiRuntime::HandleTcpClient(Socket& client)
{
	client.Timeout(3000);
	String first = TrimCR(client.GetLine(1 << 20));
	if(client.IsError() || client.IsEof() || first.IsEmpty())
		return;

	if(first.Find("HTTP/") >= 0) {
		HandleHttpRequest(client, first);
		return;
	}

	String resp = json_handler ? json_handler(first)
	                           : MluiMakeJsonResponse(false, Value(), Value(), "No MLUI JSON handler");
	resp.Cat('\n');
	client.PutAll(resp);
}

bool MluiRuntime::HandleHttpRequest(Socket& client, const String& first_line)
{
	VectorMap<String, String> headers;
	for(;;) {
		String h = TrimCR(client.GetLine(1 << 16));
		if(client.IsError())
			return false;
		if(h.IsEmpty())
			break;
		int q = h.Find(':');
		if(q > 0) {
			String key = ToLower(TrimBoth(h.Left(q)));
			String val = TrimBoth(h.Mid(q + 1));
			headers.GetAdd(key) = val;
		}
	}

	Vector<String> parts = Split(first_line, ' ');
	String method = parts.GetCount() >= 1 ? parts[0] : String();
	String path = parts.GetCount() >= 2 ? parts[1] : String();

	String request_json;
	if(method == "GET") {
		ValueMap req;
		if(path.StartsWith("/snapshot"))
			req.Add("method", "snapshot");
		else
			req.Add("method", "ping");
		request_json = AsJSON(req);
	}
	else if(method == "POST") {
		int content_length = 0;
		if(headers.Find("content-length") >= 0)
			MluiParseInt(headers.Get("content-length"), content_length);
		if(content_length < 0)
			content_length = 0;
		request_json = content_length ? client.GetAll(content_length) : String();
	}
	else {
		String body = MluiMakeJsonResponse(false, Value(), Value(), "Unsupported HTTP method");
		String hdr;
		hdr << "HTTP/1.1 405 Method Not Allowed\r\n"
		    << "Content-Type: application/json\r\n"
		    << "Content-Length: " << body.GetCount() << "\r\n"
		    << "Connection: close\r\n\r\n";
		client.PutAll(hdr + body);
		return false;
	}

	if(request_json.IsEmpty()) {
		String body = MluiMakeJsonResponse(false, Value(), Value(), "Empty HTTP request body");
		String hdr;
		hdr << "HTTP/1.1 400 Bad Request\r\n"
		    << "Content-Type: application/json\r\n"
		    << "Content-Length: " << body.GetCount() << "\r\n"
		    << "Connection: close\r\n\r\n";
		client.PutAll(hdr + body);
		return false;
	}

	String body = json_handler ? json_handler(request_json)
	                           : MluiMakeJsonResponse(false, Value(), Value(), "No MLUI JSON handler");
	String hdr;
	hdr << "HTTP/1.1 200 OK\r\n"
	    << "Content-Type: application/json\r\n"
	    << "Content-Length: " << body.GetCount() << "\r\n"
	    << "Connection: close\r\n\r\n";
	client.PutAll(hdr + body);
	return true;
}

void MluiRuntime::TcpLoop()
{
	Socket listener;
	listener.Timeout(100);

	bool listen_ok = false;
	if(bind_host.IsEmpty() || bind_host == "0.0.0.0" || bind_host == "*")
		listen_ok = listener.Listen(bind_port, 64, false, true);
	else {
		IpAddrInfo addr;
		if(addr.Execute(bind_host, bind_port, IpAddrInfo::FAMILY_ANY))
			listen_ok = listener.Listen(addr, bind_port, 64, false, true);
		if(!listen_ok)
			listen_ok = listener.Listen(bind_port, 64, false, true);
	}

	if(!listen_ok) {
		std::fprintf(stderr, "MLUI: listen failed at %s:%d: %s\n",
		             bind_host.ToStd().c_str(), bind_port, listener.GetErrorDesc().ToStd().c_str());
		return;
	}

	std::fprintf(stderr, "MLUI: listening at %s:%d\n", bind_host.ToStd().c_str(), bind_port);

	while(!stop) {
		One<Socket> client;
		client.Create();
		client->Timeout(100);
		if(client->Accept(listener)) {
			client->NoDelay();
			HandleTcpClient(*client);
			client->Close();
		}
		else if(listener.IsError()) {
			listener.ClearError();
			Sleep(10);
		}
	}

	std::fprintf(stderr, "MLUI: tcp server stopped\n");
}

void MluiRuntime::Start()
{
	if(started)
		return;
	if(!use_stdio && !use_tcp)
		return;
	if(!json_handler)
		return;

	MluiWarmupValueTypes();

	started = 1;
	stop = 0;

	if(use_stdio)
		stdio_thread.Run([this] { StdioLoop(); });
	if(use_tcp)
		tcp_thread.Run([this] { TcpLoop(); });
}

END_UPP_NAMESPACE
