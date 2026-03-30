#ifndef _Core_Mlui_h_
#define _Core_Mlui_h_

class Socket;

struct MluiRequest : Moveable<MluiRequest> {
	Value id;
	String method;
	ValueMap params;
};

typedef Function<String(const String&)> MluiJsonHandler;

bool   MluiParseBindAddress(const String& spec, String& host, int& port);
bool   MluiParseJsonRequest(const String& request_json, MluiRequest& out, String& error);
String MluiMakeJsonResponse(bool ok, const Value& id, const Value& result = Value(), const String& error = String());

class MluiRuntime {
public:
	typedef MluiRuntime CLASSNAME;

	void SetJsonHandler(const MluiJsonHandler& handler);
	void Configure(bool use_stdio, bool use_tcp, const String& host, int port);
	void Start();
	bool IsStarted() const;

private:
	Atomic started {0};
	Atomic stop {0};
	Thread stdio_thread;
	Thread tcp_thread;
	bool   use_stdio = false;
	bool   use_tcp = false;
	String bind_host;
	int    bind_port = 0;
	MluiJsonHandler json_handler;

	void StdioLoop();
	void TcpLoop();
	void HandleTcpClient(Socket& client);
	bool HandleHttpRequest(Socket& client, const String& first_line);
	static String TrimCR(const String& s);
};

#endif
