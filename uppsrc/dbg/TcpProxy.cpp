#include "dbg.h"

using namespace Upp;

bool DbgMcpCli::Connect(const String& h, int p)
{
	host = h;
	port = p;
	sock.Timeout(10000);
	if(!sock.Connect(host, port)) {
		Cerr() << "dbg: cannot connect to " << host << ":" << port
		       << " — is TheIDE running with MCP server enabled?\n";
		return false;
	}
	Cerr() << "dbg: connected to " << host << ":" << port << "\n";
	return true;
}

bool DbgMcpCli::SendLine(const String& json)
{
	sock.Put(json + "\n");
	return !sock.IsError();
}

bool DbgMcpCli::RecvLine(String& out_json)
{
	sock.Timeout(30000);
	out_json = sock.GetLine();
	return !sock.IsError() && !sock.IsEof();
}

int DbgMcpCli::Loop()
{
	char buf[65536];
	for(;;) {
		if(!fgets(buf, sizeof(buf), stdin))
			break; // EOF or error
		String line = TrimBoth(String(buf));
		if(line.IsEmpty())
			continue;

		// Validate JSON before forwarding.
		Value v = ParseJSON(line);
		if(v.IsError()) {
			Cout() << "{\"jsonrpc\":\"2.0\",\"id\":null,\"error\":"
			          "{\"code\":-32700,\"message\":\"Parse error: invalid JSON\"}}\n";
			Cout().Flush();
			continue;
		}

		if(!SendLine(line)) {
			Cerr() << "dbg: server connection lost while sending\n";
			return 1;
		}

		String resp;
		if(!RecvLine(resp)) {
			Cerr() << "dbg: server connection lost while receiving\n";
			return 1;
		}

		Cout() << resp << "\n";
		Cout().Flush();
	}
	return 0;
}

int DbgMcpCli::OneShot(const String& method, const String& params_json)
{
	String id = AsString(next_id++);
	String req = "{\"jsonrpc\":\"2.0\",\"id\":\"" + id + "\","
	             "\"method\":\"" + method + "\","
	             "\"params\":" + params_json + "}";

	if(!SendLine(req)) {
		Cerr() << "dbg: failed to send request\n";
		return 1;
	}

	String resp;
	if(!RecvLine(resp)) {
		Cerr() << "dbg: no response from server\n";
		return 1;
	}

	Cout() << resp << "\n";
	return 0;
}
