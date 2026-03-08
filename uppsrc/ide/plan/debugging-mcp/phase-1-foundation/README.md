# Phase 1: Foundation

## Goal

Define the `debug.*` protocol extension and implement `DebugBridge` — a thin, thread-safe
facade over the live `Debugger` instance held by `TheIde()`.

No user-visible changes in this phase.  All work is in `uppsrc/ide/MCP/`.

## Tasks

| Task | File(s) | Description |
|---|---|---|
| task-1-protocol-design | `DebugProtocol.h` | Structs + JSON helpers for debug.* |
| task-2-debugbridge | `DebugBridge.h/.cpp` | Thread-safe accessor + command queuing |

## Acceptance Criteria

- `DebugBridge::GetState()` returns valid JSON when called from the MCP server thread.
- Breakpoint list, current file/line, and debug-active flag are all readable without UI thread.
- No GUI lockups: all bridge calls either use `GuiLock` or post to the GUI thread via
  `PostCallback` and block on a promise/event.
- Unit-testable without starting GDB.

## Dependencies

- Existing `Debugger` interface in `uppsrc/ide/Common/Common.h`.
- `IdeIsDebug()`, `IdeGetFileName()`, `IdeGetFileLine()` in `uppsrc/ide/Core/Core.h`.
- `TheIde()->debugger` (type `One<Debugger>`) for calling `SetBreakpoint`, `Run`, `Stop`, etc.
