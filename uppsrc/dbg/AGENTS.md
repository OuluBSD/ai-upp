Scope: uppsrc/dbg

Purpose
- Standalone headless CLI binary that proxies MCP JSON-RPC requests to a running TheIDE MCP
  server over TCP, and re-exports them over stdio for AI hosts (Claude Desktop, Cursor, etc.).
- Reads newline-delimited JSON-RPC 2.0 from stdin, forwards to TheIDE MCP server via TCP on
  localhost:7326 (default), writes responses to stdout. Logs/errors go to stderr.

Build
- script/build.py -mc 0 -j 12 dbg    (headless, no GUI flag needed)
- Binary: bin/dbg

Usage
  Interactive proxy (AI host connects via stdio):
    bin/dbg [--host H] [--port P]

  One-shot call (print one result and exit):
    bin/dbg [--host H] [--port P] <method> [params_json]
    bin/dbg debug.state
    bin/dbg debug.breakpoint.set '{"file":"main.cpp","line":5}'

  Requires TheIDE running with MCP server enabled (port 7326 by default).

AI Host Configuration (Claude Desktop / Cursor mcp.json)
  {
    "mcpServers": {
      "theide-debug": {
        "command": "/path/to/bin/dbg",
        "args": []
      }
    }
  }

Wire Protocol (same as McpServerCore)
- Newline-delimited JSON-RPC 2.0 (each message is one line).
- dbg reads lines from stdin, sends each to the server, writes the server's response line
  to stdout and flushes. This matches the McpServerCore framing (ReadFramed / WritePending).

File Map
- dbg.h       : umbrella header
- TcpProxy.h/cpp : DbgMcpCli class — connect, Loop (proxy), OneShot
- main.cpp    : CONSOLE_APP_MAIN entry point, CLI argument parsing
