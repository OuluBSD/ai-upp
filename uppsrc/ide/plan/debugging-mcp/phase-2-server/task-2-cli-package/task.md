# Task 2-2: CLI Package — uppsrc/dbg

## Overview

Create a new package `uppsrc/dbg` that builds a standalone headless binary `dbg`.

The binary connects to a running TheIDE MCP server as a TCP client, forwards requests received
on stdin to the server, and writes responses to stdout.  This makes TheIDE's debug tools
available to any AI host that speaks stdio JSON-RPC (e.g. Cursor MCP, Claude Desktop).

## Reference Comparison

In the TypeScript reference:
- `cli.ts` starts a stdio MCP server using `StdioServerTransport`.
- `mcp-client.ts` is an HTTP client that proxies all tool calls to the VSCode extension.

Our equivalent:
- `dbg` reads JSON-RPC from stdin, connects to `localhost:7326`, forwards, writes response to stdout.
- No intermediate MCP SDK needed — it's a simple TCP proxy with framing.

## Package Location

`uppsrc/dbg/`

## Files to Create

### dbg.upp
```
description "DbgMcpCli";

uses
    Core;

file
    AGENTS.md,
    dbg.h,
    main.cpp,
    TcpProxy.h,
    TcpProxy.cpp;
```

Note: uses only `Core` (no GUI, no `ide` packages).  The binary is headless.

### AGENTS.md
```
Scope: uppsrc/dbg

Purpose
- Standalone CLI binary that proxies MCP JSON-RPC requests to a running TheIDE MCP server.
- Reads newline-delimited JSON-RPC 2.0 from stdin, forwards to MCP server via TCP,
  writes responses to stdout. Logs to stderr.

Build
- script/build.py -mc 0 -j 12 dbg
- No GUI flag: add +NOGUI to build method or use headless build.

Protocol
- Same length-prefixed JSON-RPC as uppsrc/MCP/Server.cpp.
- See DbgMcpCli::Loop() in TcpProxy.cpp.
```

### dbg.h (umbrella header)
```cpp
#ifndef _dbg_dbg_h_
#define _dbg_dbg_h_

#include <Core/Core.h>

NAMESPACE_UPP

#include "TcpProxy.h"

END_UPP_NAMESPACE

#endif
```

### TcpProxy.h
```cpp
// DbgMcpCli: reads stdin JSON-RPC, proxies to MCP TCP server, writes stdout.
class DbgMcpCli {
public:
    // Connect to host:port. Returns false on failure.
    bool Connect(const String& host, int port);

    // Main loop: read stdin -> TCP -> stdout.
    // Returns exit code (0=ok, 1=error).
    int  Loop();

    // One-shot: send a single method call, print result, exit.
    int  OneShot(const String& method, const String& params_json = "{}");

private:
    void WriteFramed(TcpSocket& sock, const String& json);
    bool ReadFramed(TcpSocket& sock, String& out_json);

    String host = "127.0.0.1";
    int    port = 7326;
    int    next_id = 1;
};
```

### main.cpp

```cpp
#include "dbg.h"

NAMESPACE_UPP

CONSOLE_APP_MAIN
{
    const Vector<String>& args = CommandLine();

    String host = "127.0.0.1";
    int    port  = 7326;
    bool   oneshot = false;
    String method, params;

    for(int i = 0; i < args.GetCount(); i++) {
        String a = args[i];
        if(a == "--help" || a == "-h") {
            Cout() << "dbg [--host H] [--port P] [method [params_json]]\n"
                   << "  If method given: one-shot call, print result, exit.\n"
                   << "  Otherwise: stdio JSON-RPC proxy to TheIDE MCP server.\n";
            SetExitCode(0); return;
        }
        else if(a.StartsWith("--host=")) host = a.Mid(7);
        else if(a.StartsWith("--port=")) port = StrInt(a.Mid(7));
        else if(!a.StartsWith("-")) {
            oneshot = true;
            method = a;
            if(i + 1 < args.GetCount()) params = args[++i];
        }
    }

    DbgMcpCli cli;
    if(!cli.Connect(host, port)) {
        Cerr() << "dbg: cannot connect to " << host << ":" << port << "\n";
        SetExitCode(1); return;
    }

    if(oneshot)
        SetExitCode(cli.OneShot(method, params.IsEmpty() ? "{}" : params));
    else
        SetExitCode(cli.Loop());
}

END_UPP_NAMESPACE
```

### TcpProxy.cpp (Loop logic)

Stdio proxy loop:
1. Read one line from stdin (newline-delimited JSON-RPC, matching typical stdio MCP convention).
2. Validate JSON parse.
3. Inject/replace `"id"` to track response matching.
4. Send to TCP server using the same length-prefix framing as `McpServerCore` (`%d\n<json>`).
5. Read response frame from server.
6. Write response line to stdout.
7. Flush stdout.

For `OneShot`: build a minimal JSON-RPC request `{"jsonrpc":"2.0","id":"1","method":"...","params":{...}}`,
send it, read one response, print to stdout.

## Framing Protocol (from uppsrc/MCP/Server.cpp)

Reading `McpServerCore::ReadFramed` source to understand the wire format:
- Message: `<length>\n<json_bytes>` where length is decimal byte count of the JSON.
- Verify this matches what `mcp_client.sh` sends.

## Build Integration

Add `dbg` to the build system.  It is NOT part of `ide.upp`.  It's a separate top-level package.

Build command:
```bash
script/build.py -mc 0 -j 12 dbg
```

Binary lands in `bin/dbg` (or wherever U++ puts headless binaries).

## AI Host Configuration

For Claude Desktop / Cursor, add to `mcp.json`:
```json
{
  "mcpServers": {
    "theide-debug": {
      "command": "/path/to/bin/dbg",
      "args": []
    }
  }
}
```

TheIDE must be running with MCP server enabled (port 7326 default).

## Status: DONE
