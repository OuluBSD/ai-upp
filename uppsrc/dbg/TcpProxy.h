#ifndef _dbg_TcpProxy_h_
#define _dbg_TcpProxy_h_

// DbgMcpCli: stdio <-> TCP proxy for TheIDE MCP server.
//
// McpServerCore uses newline-delimited JSON-RPC: each message is a single line
// (no length prefix). DbgMcpCli reads lines from stdin, sends them to the server,
// and writes response lines to stdout.

class DbgMcpCli {
public:
	// Connect to MCP server. Returns false if connection fails.
	bool Connect(const String& host = "127.0.0.1", int port = 7326);

	// Stdio proxy loop: read stdin lines -> TCP -> stdout.
	// Returns exit code (0=ok, 1=connection error).
	int Loop();

	// Send one method call, print result to stdout, return exit code.
	int OneShot(const String& method, const String& params_json = "{}");

private:
	bool SendLine(const String& json);
	bool RecvLine(String& out_json);

	TcpSocket sock;
	String    host = "127.0.0.1";
	int       port = 7326;
	int       next_id = 1;
};

#endif
