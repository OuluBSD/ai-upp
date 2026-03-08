# Task 1-2: DebugBridge — thread-safe debugger facade

## Files to Create

- `uppsrc/ide/MCP/DebugBridge.h`
- `uppsrc/ide/MCP/DebugBridge.cpp`

## Purpose

`DebugBridge` is a singleton (like `sMcpServer`) that:
1. Reads debugger state safely from MCP server thread.
2. Posts debugger commands to the GUI thread and waits for completion.
3. Tracks the active breakpoint list (since the debugger backends don't expose a queryable BP list).

The McpServer runs on its own thread; the Debugger lives on the GUI thread and must only be
touched under GuiLock or via PostCallback.  DebugBridge mediates this.

## Reference Comparison

In the TypeScript reference (`src/tools.ts`), all tools call `vscode.debug.*` API which is
already thread-safe from extension context.  We have no such luxury: our debugger is a plain
C++ object on the GUI thread.  The bridge pattern is our equivalent of the VSCode extension
message passing.

## Class Interface

```cpp
class DebugBridge {
public:
    // --- Read operations (callable from any thread) ---

    // Returns a snapshot of current debug state.
    // Uses GuiLock to safely read TheIde()->debugger.
    DbgState GetState() const;

    // Returns copy of tracked breakpoint list.
    // Protected by its own mutex (not GuiLock).
    Vector<DbgBreakpoint> GetBreakpoints() const;

    // --- Write operations (post to GUI thread, wait) ---

    // Sets or clears a breakpoint. bp="" clears; non-empty sets.
    // Calls TheIde()->debugger->SetBreakpoint(file, line, bp) on GUI thread.
    // Returns error string on failure, empty on success.
    String SetBreakpoint(const String& file, int line, const String& condition);
    String ClearBreakpoint(const String& file, int line);

    // Execution control — all post to GUI thread.
    String Continue();    // debugger->Run()
    String StepOver();    // Gdb: Cmd("next"), LLDB: Cmd("thread step-over")
    String StepInto();    // Gdb: Cmd("step"), LLDB: Cmd("thread step-in")
    String StepOut();     // Gdb: Cmd("finish"), LLDB: Cmd("thread step-out")
    String Pause();       // Gdb/LLDB: BreakRunning()
    String Stop();        // debugger->Stop()
    String Start();       // TheIde()->BuildAndDebug() or equivalent

    // --- State inspection (GUI thread, blocking) ---

    // Returns current call stack as text (calls Gdb::CopyStack equivalent).
    String GetStackTrace() const;

    // Evaluates expression and returns value string.
    String Evaluate(const String& expr) const;

    // Returns locals as key=value map.
    VectorMap<String, String> GetLocals() const;

    // Returns thread list as descriptive strings.
    Vector<String> GetThreads() const;

private:
    // Posts a lambda to GUI thread and blocks until done.
    // timeout_ms=0 means no timeout (waits forever).
    bool RunOnGui(Function<void()> fn, int timeout_ms = 3000);

    // Tracked breakpoints (separate from debugger internals).
    mutable Mutex          bp_mutex;
    Vector<DbgBreakpoint>  breakpoints;
};

extern DebugBridge sDebugBridge;
```

## RunOnGui Implementation

```cpp
bool DebugBridge::RunOnGui(Function<void()> fn, int timeout_ms)
{
    Semaphore done;
    bool ok = true;
    PostCallback([&] {
        fn();
        done.Release();
    });
    if(timeout_ms > 0)
        ok = done.Wait(timeout_ms);
    else
        done.Wait();
    return ok;
}
```

U++ `PostCallback` posts to the GUI event loop.  `Semaphore::Wait(ms)` returns false on timeout.

## Breakpoint Tracking

Since `Gdb`/`LLDB` don't expose their internal breakpoint data structures, `DebugBridge`
maintains its own `Vector<DbgBreakpoint>`.  On every `SetBreakpoint`/`ClearBreakpoint` call
it updates this list under `bp_mutex`.

When a debug session ends (future: hook into `IdeEndDebug`), clear the list.

## Accessing Gdb/LLDB-specific Commands

`TheIde()->debugger` is a `One<Debugger>` — the `Debugger` interface has only `SetBreakpoint`,
`Run`, `Stop`, `IsFinished`.

For `StepOver/Into/Out` and `Evaluate`/`GetLocals`, we need to dynamic-cast:

```cpp
if(Gdb* gdb = dynamic_cast<Gdb*>(TheIde()->debugger.Get())) {
    result = gdb->Cmd("next");
}
else if(LLDB* lldb = dynamic_cast<LLDB*>(TheIde()->debugger.Get())) {
    result = lldb->Cmd("thread step-over");
}
```

For `Evaluate`:
```cpp
if(Gdb* gdb = ...) result = gdb->Print(expr);
if(LLDB* lldb = ...) result = lldb->Print(expr);
```

## Backend Detection

```cpp
String DbgState::backend is set by:
if(dynamic_cast<Gdb*>(...))  return "gdb";
if(dynamic_cast<LLDB*>(...)) return "lldb";
#ifdef PLATFORM_WIN32
if(dynamic_cast<Pdb*>(...))  return "pdb";
#endif
```

## GetLocals Implementation

GDB: `gdb->Locals()` updates the `locals` ArrayCtrl member, then we read it.
Problem: `Locals()` has side effects (updates UI).  Instead call:
```cpp
String raw = gdb->Cmd("info locals");
// parse key=value lines
```
Or use `gdb->Print("*(local_var)")` per variable.  Simplest: call `gdb->Cmd("info locals")` and
parse output lines of form `name = value`.

LLDB equivalent: `lldb->Cmd("frame variable")`.

## GetStackTrace Implementation

GDB: `gdb->Cmd("bt " + AsString(max_stack_trace_size))` — parse `#N  func at file:line`.
LLDB: `lldb->Cmd("bt")`.

## Files to Add to MCP.upp

```
DebugProtocol.h,
DebugBridge.h,
DebugBridge.cpp,
```

## Status: TODO
