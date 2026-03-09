# Phase 3: Tools Implementation

## Goal

Implement all 15 `debug.*` tools end-to-end: `DebugBridge` methods fully fleshed out, handlers
in `Server.cpp` complete, and roundtrip tested with `mcp_client.sh`.

## Tasks

| Task | Tools |
|---|---|
| task-1-breakpoints | `debug.breakpoint.set`, `debug.breakpoint.clear`, `debug.breakpoint.list` |
| task-2-execution-control | `debug.session.start`, `debug.session.stop`, `debug.continue`, `debug.step.*`, `debug.pause` |
| task-3-state-inspection | `debug.state`, `debug.stack`, `debug.locals`, `debug.evaluate`, `debug.threads` |

## Acceptance Criteria

Given a trivial C++ target (e.g. `upptst/HelloDbg` — a 5-line main.cpp that prints and loops):

1. `debug.breakpoint.set` with `{"file":"main.cpp","line":3}` — breakpoint appears in GDB.
2. `debug.session.start` — GDB starts, hits breakpoint.
3. `debug.state` returns `{"active":true,"paused":true,"file":"main.cpp","line":3,"backend":"gdb"}`.
4. `debug.stack` returns at least one frame with `"function":"main"`.
5. `debug.locals` returns any local variables.
6. `debug.evaluate` with `{"expression":"argc"}` returns `"1"` (or actual value).
7. `debug.step.over` advances to next line; `debug.state` reflects new line.
8. `debug.continue` resumes; `debug.state` returns `paused:false`.
9. `debug.session.stop` terminates.
10. `debug.breakpoint.list` returns empty after stop (or persisted list if we keep them).

## GDB-Specific Notes

- `Gdb::Cmd(const char*)` sends a command and returns the response string.
- Must be called on GUI thread (it calls `LocalProcess::Write` and polls with `LocalProcess::Read`).
- Safe via `DebugBridge::RunOnGui`.
- Parsing: GDB output uses `GDB_PROMPT` sentinel; `AfterTag`, `FindTag` helpers exist in `GdbCmd.cpp`.

## LLDB-Specific Notes

- Same `Cmd()` pattern as Gdb.
- LLDB commands differ: `next` vs `thread step-over`, `finish` vs `thread step-out`.
- Use `dynamic_cast<LLDB*>` to dispatch.

## PDB Notes (Windows-only)

- `Pdb` does not have a `Cmd()` method.
- Step/continue are via `Pdb::Step*()` methods (check `Pdb.h`).
- Out of scope for initial implementation; `DebugBridge` returns
  `"pdb backend: step/eval not yet supported"` error for unsupported ops.
