#include "dbg.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();

	String host = "127.0.0.1";
	int    port  = 7326;
	String method;
	String params;

	for(int i = 0; i < args.GetCount(); i++) {
		const String& a = args[i];
		if(a == "--help" || a == "-h") {
			Cout() << "Usage: dbg [--host HOST] [--port PORT] [method [params_json]]\n"
			       << "\n"
			       << "  No method: stdio JSON-RPC proxy to TheIDE MCP server.\n"
			       << "  With method: one-shot call, print result, exit.\n"
			       << "\n"
			       << "Options:\n"
			       << "  --host HOST   MCP server host (default: 127.0.0.1)\n"
			       << "  --port PORT   MCP server port (default: 7326)\n"
			       << "\n"
			       << "Examples:\n"
			       << "  dbg debug.state\n"
			       << "  dbg debug.breakpoint.set '{\"file\":\"main.cpp\",\"line\":5}'\n"
			       << "  echo '{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"method\":\"debug.state\",\"params\":{}}' | dbg\n";
			SetExitCode(0);
			return;
		}
		if(a.StartsWith("--host="))      host = a.Mid(7);
		else if(a.StartsWith("--port=")) port = StrInt(a.Mid(7));
		else if(a == "--host" && i + 1 < args.GetCount())  host = args[++i];
		else if(a == "--port" && i + 1 < args.GetCount())  port = StrInt(args[++i]);
		else if(!a.StartsWith("-") && method.IsEmpty()) {
			method = a;
			if(i + 1 < args.GetCount() && !args[i + 1].StartsWith("-"))
				params = args[++i];
		}
	}

	DbgMcpCli cli;
	if(!cli.Connect(host, port)) {
		SetExitCode(1);
		return;
	}

	if(!method.IsEmpty())
		SetExitCode(cli.OneShot(method, params.IsEmpty() ? "{}" : params));
	else
		SetExitCode(cli.Loop());
}
