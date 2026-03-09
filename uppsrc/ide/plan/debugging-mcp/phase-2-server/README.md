# Phase 2: Server Integration

## Goal

Wire `DebugBridge` into `McpServer::HandleExtended` to handle all `debug.*` methods, and create
the `uppsrc/dbg` package skeleton for the CLI binary.

## Tasks

| Task | Description |
|---|---|
| task-1-ide-server | Add `debug.*` handlers to `Server.cpp` |
| task-2-cli-package | Create `uppsrc/dbg` package with .upp manifest and CLI skeleton |

## Acceptance Criteria

- `mcp_client.sh` can call `debug.state` and receive a valid JSON response while TheIDE is running.
- `debug.session.start` triggers the current target's debug launch.
- `dbg` compiles as a standalone binary and connects to the running MCP server on port 7326.
- `dbg --help` prints usage; `dbg debug.state` prints the JSON response.

## Dependencies

- Phase 1 complete: `DebugBridge`, `DebugProtocol.h`.
- `uppsrc/ide/ide.h` for `TheIde()`.
- `uppsrc/ide/Build.cpp` for `BuildAndDebug` / `StartDebug` function (see `Ide::StartDebug`).
