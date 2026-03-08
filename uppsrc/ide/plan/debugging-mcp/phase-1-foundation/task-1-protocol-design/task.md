# Task 1-1: Protocol Design — DebugProtocol.h

## File to Create

`uppsrc/ide/MCP/DebugProtocol.h`

## Purpose

Extend the existing `Protocol.h` / `McpRequest` / `McpResponse` pattern with debug-specific
structs and JSON round-trip helpers.  Mirrors what `tools-parameters.ts` + typed responses do
in the reference implementation, but in C++ / U++ Value style.

## Reference Comparison

The TypeScript reference (`src/tools-parameters.ts`) uses Zod schemas for runtime parameter
validation.  Our equivalent: plain `ValueMap` access with inline validation returning
`MakeError(req.id, INVALID_PARAMS, "...")` on bad input.

## Method Namespace

All new methods use the `debug.` prefix:

| Method | Description |
|---|---|
| `debug.state` | Returns whether a debug session is active, current file/line, paused state |
| `debug.session.start` | Launch the currently configured target under the debugger |
| `debug.session.stop` | Stop the running debug session |
| `debug.continue` | Resume execution |
| `debug.step.over` | Step over (next) |
| `debug.step.into` | Step into |
| `debug.step.out` | Step out (finish) |
| `debug.pause` | Break into the running process |
| `debug.breakpoint.set` | Set or clear a breakpoint on file:line |
| `debug.breakpoint.clear` | Clear breakpoint on file:line |
| `debug.breakpoint.list` | List all active breakpoints |
| `debug.stack` | Return the current call stack |
| `debug.locals` | Return local variables in the current frame |
| `debug.evaluate` | Evaluate an expression in the debugger |
| `debug.threads` | List threads |

## Structs to Define

```cpp
// Returned by debug.state
struct DbgState {
    bool   active;       // debugger is running
    bool   paused;       // execution is stopped at a breakpoint/step
    String file;         // current source file (empty if not paused)
    int    line;         // current line (0 if not paused)
    String backend;      // "gdb", "lldb", "pdb", or ""
};

// A single breakpoint entry
struct DbgBreakpoint {
    String file;
    int    line;
    String condition;   // "" = unconditional
    bool   enabled;
};

// A single stack frame
struct DbgFrame {
    int    index;
    String function;
    String file;
    int    line;
    String address;
};

// A single variable/local
struct DbgVar {
    String name;
    String value;
    String type;
};
```

## JSON Helpers

Implement `ToValue(const DbgState&)`, `ToValue(const DbgBreakpoint&)`, etc. returning `Value`
(U++ `ValueMap`) so they can be passed directly to `MakeResult(req.id, v)`.

## Thread Safety Note

These are pure data structs — no locking concerns here.  `DebugBridge` (task-2) will fill them
under appropriate locking.

## Implementation Notes

- Follow `Protocol.h` style: inline helpers in header, no separate .cpp needed.
- Add `#include "DebugProtocol.h"` to `MCP.h` after `Protocol.h`.
- Keep structs POD-like; no U++ `Serializable` needed (we only need JSON-out, not JSON-in).

## Status: DONE
