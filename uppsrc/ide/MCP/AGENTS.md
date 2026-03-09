Scope: uppsrc/ide/MCP

Purpose
- MCP server for TheIDE: exposes workspace/project/file/build APIs over a JSON-RPC 2.0 protocol over TCP (initial MVP). Designed to power OpenAI Codex via MCP with IDE-grade code intelligence and edits.

Conventions
- Follow Header Include Policy (U++ BLITZ): all .cpp/.icpp start with `#include "MCP.h"` and only add rare local includes after.
- `MCP.h` is the main header; it aggregates public headers and wraps them in `NAMESPACE_UPP`.
- Keep Protocol minimal and versioned: start with `mcp.ping` and `workspace.info` for MVP.

Integration Points
- Package is added to `uppsrc/ide/ide.upp`.
- Start/Stop APIs are provided; TheIDE can toggle server on startup or via menu later.

Split
- Headless core lives in `uppsrc/MCP`:
  - Protocol, Log, Server (`McpServerCore`) with transport/framing/capabilities and core endpoints.
- IDE integration lives here (`uppsrc/ide/MCP`):
  - `McpServer` is a thin wrapper subclassing `McpServerCore` and overriding `HandleExtended` to add IDE-only endpoints and lifecycle wiring.

File Map
- MCP.upp: manifest (lists AGENTS.md, CURRENT_TASK.md first).
- CURRENT_TASK.md: working notes and next steps.
- MCP.h / MCP.cpp: main header + start/stop entrypoints.
- Protocol.h: request/response structs and serialization helpers.
- DebugProtocol.h: DbgState/DbgBreakpoint/DbgFrame structs, ToValue() converters,
    ParseBacktrace() (GDB+LLDB), ParseLocals(). All inline, header-only.
- DebugBridge.h / DebugBridge.cpp: thread-safe facade over the live Gdb/LLDB/Pdb.
    Uses PostCallback+Semaphore (RunOnGui) for cross-thread calls from MCP server thread.
    sDebugBridge is the global singleton.
- Server.h / Server.cpp: IDE wrapper over headless `McpServerCore`. Handles:
    workspace.*, node.*, edits.*, debug.*, resource.*
- WorkspaceBridge.h / WorkspaceBridge.cpp: adapters to Ide workspace queries.
- Index.h / Index.cpp: adapter over IdeMetaEnvironment/MetaEnvironment for AST-backed queries.
- mcp_client.sh: TCP test client. Auto-discovers methods via mcp.capabilities or by
  grepping `Server.cpp` for `req.method == "..."`. Keep handler methods in `Server.cpp`
  as `if(req.method == "name")` so the client picks them up automatically.

Endpoints
- mcp.ping, mcp.capabilities, mcp.log.get/clear, mcp.index.status/refresh
- workspace.info, node.locate/get/definition/references, edits.apply
- debug.state — active/paused/file/line/backend snapshot (GuiLock, always safe)
- debug.session.start/stop — launch BuildAndDebug or stop debugger
- debug.continue, debug.step.over/into/out, debug.pause — execution control
- debug.breakpoint.set/clear/list — breakpoint management + tracking
- debug.stack — call stack frames (requires paused session)
- debug.locals — local variables (requires paused session)
- debug.evaluate — expression evaluation (requires paused session)
- debug.threads — thread list
- resource.list, resource.get — debug:// URI scheme for read-only state queries
    URIs: debug://state, debug://breakpoints, debug://stack, debug://locals, debug://threads

Threading Model
- McpServerCore runs on its own thread. DebugBridge bridges to the GUI thread.
- Read-only state (GetState): GuiLock, no blocking.
- Commands (Step, Continue, Evaluate, etc.): PostCallback + Semaphore::Wait(8000ms).
- Gdb/LLDB::Cmd() pumps ProcessEvents() internally while waiting for GDB output —
  no GUI deadlock risk.

Codex Focus
- AST lives in MetaEnvironment (Core2/VfsValue.h) exposed via IdeMetaEnvironment (ide/Vfs/Ide.h).
- AST is built by ScriptBuilder (ide/Builders/ScriptBuilder.cpp) when the SCRIPT builder is selected.
- MCP node.* endpoints query this environment for symbol-safe operations (find/refs/rename/etc.).

Related
- uppsrc/dbg: standalone CLI binary that proxies debug.* over stdio for AI hosts.
- uppsrc/MCP: headless TCP JSON-RPC core (McpServerCore, Protocol, Log).
