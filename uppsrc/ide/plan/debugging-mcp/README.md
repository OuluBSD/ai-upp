# Track: debugging-mcp

## Goal

Expose TheIDE's debugger (GDB, LLDB, PDB) state and control operations over the existing MCP
JSON-RPC 2.0 TCP server so that AI assistants (Claude, Codex, Cursor, Windsurf, etc.) can drive
debugging sessions programmatically.

In addition, provide a standalone CLI binary (`uppsrc/dbg`) that connects to the running IDE MCP
server and re-exports the same debugging API over stdio for AI hosts that require a local stdio
MCP server.

## Reference

`/tmp/mcp-debug-ref/mcp-debug-tools-0.2.1/` — TypeScript/VSCode reference implementation
(hwanyong/mcp-debug-tools).  Key insights:
- Three-tier architecture: IDE extension <-> HTTP proxy CLI <-> AI host (stdio).
- ~20 tools across 5 groups: breakpoints, execution control, state inspection, session
  management, workspace info.
- Our equivalent: TheIDE MCP server (TCP/JSON-RPC) <-> `dbg` CLI (stdio) <-> AI host.
- Transport: our core uses length-prefixed JSON-RPC over TCP (not HTTP/SSE); the CLI connects
  as a TCP client and re-exports via stdio.

## Architecture

```
AI host (Claude / Cursor / Windsurf)
    | stdio JSON-RPC
    v
uppsrc/dbg  (DbgMcpCli standalone binary)
    | TCP JSON-RPC (localhost:7326)
    v
TheIDE MCP server  (uppsrc/ide/MCP  -  McpServer)
    | GuiLock / Event<> callbacks
    v
Active Debugger  (Gdb / LLDB / Pdb)  in  uppsrc/ide/Debuggers
```

## Package Map

| Package | Role |
|---|---|
| `uppsrc/MCP` | Headless JSON-RPC TCP core (already exists) |
| `uppsrc/ide/MCP` | IDE wrapper, adds workspace/node/edit handlers (already exists) |
| `uppsrc/ide/MCP` (extended) | **New**: add `debug.*` handlers via `DebugBridge` |
| `uppsrc/ide/Debuggers` | GDB/LLDB/PDB backends (already exists, read-only interface) |
| `uppsrc/dbg` | **New**: standalone CLI that proxies debug.* tools over stdio |

## Phases

| Phase | Name | Deliverable |
|---|---|---|
| 1 | Foundation | Protocol types + DebugBridge (thread-safe accessor to live debugger) |
| 2 | Server integration | debug.* handlers in McpServer; `dbg` package skeleton |
| 3 | Tools | All 15 debug tools implemented end-to-end |
| 4 | Polish | MCP resources, CLI proxy robustness, AGENTS docs |

## Status: DONE (phases 1-3 complete, phase 4 resources/docs shipped inline)
