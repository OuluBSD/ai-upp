Scope: uppsrc/ide/MCP

Purpose
- MCP server for TheIDE: exposes workspace/project/file/build APIs over a simple JSON-RPC-like protocol over stdio (initial MVP).

Conventions
- Follow Header Include Policy (U++ BLITZ): all .cpp/.icpp start with `#include "MCP.h"` and only add rare local includes after.
- `MCP.h` is the main header; it aggregates public headers and wraps them in `NAMESPACE_UPP`.
- Keep Protocol minimal and versioned: start with `mcp.ping` and `workspace.info` for MVP.

Integration Points
- Package is added to `uppsrc/ide/ide.upp`.
- Start/Stop APIs are provided; TheIDE can toggle server on startup or via menu later.

File Map
- MCP.upp: manifest (lists AGENTS.md, CURRENT_TASK.md first).
- CURRENT_TASK.md: working notes and next steps.
- MCP.h / MCP.cpp: main header + start/stop entrypoints.
- Protocol.h: request/response structs and serialization helpers.
- Server.h / Server.cpp: stdio transport, router, thread.
- WorkspaceBridge.h / WorkspaceBridge.cpp: adapters to Ide workspace queries.

MVP Endpoints
- mcp.ping -> { "result": "pong" }
- workspace.info -> basic info about current workspace (name, package count).

