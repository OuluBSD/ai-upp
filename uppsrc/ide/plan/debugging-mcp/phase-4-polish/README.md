# Phase 4: Polish

## Goal

Add MCP resource URIs for debug state, make the CLI proxy robust, and write AGENTS.md
documentation for both `uppsrc/ide/MCP` and `uppsrc/dbg`.

## Tasks

| Task | Description |
|---|---|
| task-1-resources | Add `debug://` resource URIs to McpServer |
| task-2-cli-proxy | Robust stdio<->TCP proxy in `dbg` with auto-reconnect and heartbeat |

## Resource URI Scheme

Following the reference implementation's resource pattern:

| URI | Content |
|---|---|
| `debug://state` | Current `DbgState` as JSON |
| `debug://breakpoints` | Full breakpoint list |
| `debug://stack` | Call stack frames |
| `debug://locals` | Local variables |
| `debug://threads` | Thread list |

These map to the same `DebugBridge` methods used by the tools, but presented as resources
(read-only, GET-style) for AI clients that prefer resource access over tool calls.

Note: The current `McpServerCore` is JSON-RPC 2.0 over TCP, not the HTTP/SSE MCP transport
used in the reference.  Resources in our protocol are just additional method namespaces
(e.g. `resource.get` with `{ "uri": "debug://state" }`).  Add a `resource.get` handler to
`McpServerCore::Handle` or `McpServer::HandleExtended`.

## CLI Robustness

- Auto-reconnect: if TCP connection to server drops, retry with backoff (1s, 2s, 4s, max 30s).
- Heartbeat: send `mcp.ping` every 30s; close and reconnect on failure.
- Error passthrough: if the IDE server returns an error JSON-RPC response, pass it directly
  to stdout (don't swallow it).
- stdin EOF: clean exit with code 0.
- Server not running: print `{"jsonrpc":"2.0","id":null,"error":{"code":-32000,"message":"TheIDE MCP server not running on port 7326. Start TheIDE first."}}` and exit 1.

## AGENTS.md Updates

After all phases complete:
1. Update `uppsrc/ide/MCP/AGENTS.md` — add debug section describing all `debug.*` endpoints.
2. Create `uppsrc/dbg/AGENTS.md` — purpose, build, usage, protocol.
3. Update main `THREAD_DEPENDENCIES.md` if debugging-mcp becomes an active thread.

## Acceptance Test Script

`uppsrc/ide/MCP/debug_client.sh` — extends `mcp_client.sh` with a scripted debug session:
```bash
#!/bin/bash
# Automated roundtrip test of debug.* MCP tools.
# Requires: TheIDE running with MCP server, HelloDbg binary compiled.
PORT=7326
send() { echo "$1" | nc -q1 localhost $PORT; }
send '{"jsonrpc":"2.0","id":"1","method":"debug.state","params":{}}'
send '{"jsonrpc":"2.0","id":"2","method":"debug.breakpoint.set","params":{"file":"main.cpp","line":5}}'
# ... etc
```

## Status: PLANNING
