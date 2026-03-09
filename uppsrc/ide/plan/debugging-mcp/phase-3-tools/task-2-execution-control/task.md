# Task 3-2: Execution Control Tools

## Tools

- `debug.session.start`
- `debug.session.stop`
- `debug.continue`
- `debug.step.over`
- `debug.step.into`
- `debug.step.out`
- `debug.pause`

## Mapping to TheIDE APIs

| MCP Tool | DebugBridge method | Debugger call |
|---|---|---|
| `debug.session.start` | `Start()` | `PostCallback(TheIde()->StartDebug)` |
| `debug.session.stop` | `Stop()` | `debugger->Stop()` on GUI |
| `debug.continue` | `Continue()` | `debugger->Run()` on GUI |
| `debug.step.over` | `StepOver()` | `gdb->Cmd("next")` / `lldb->Cmd("thread step-over")` |
| `debug.step.into` | `StepInto()` | `gdb->Cmd("step")` / `lldb->Cmd("thread step-in")` |
| `debug.step.out` | `StepOut()` | `gdb->Cmd("finish")` / `lldb->Cmd("thread step-out")` |
| `debug.pause` | `Pause()` | `Gdb::BreakRunning(pid)` / `lldb->Cmd("process interrupt")` |

## Finding StartDebug

Grep for `StartDebug` in `uppsrc/ide/`:
```
rg -n "StartDebug\|void.*Debug\b" uppsrc/ide/ide.h
```
Likely it's `Ide::Debug()` or `Ide::BuildAndDebug()`.  The exact method triggers the build
(if needed) then launches GDB/LLDB.  Use `PostCallback` to call it asynchronously and return
`{ "accepted": true }` immediately — the caller cannot block waiting for GDB to start.

## DebugBridge::Start()

```cpp
String DebugBridge::Start()
{
    if(!TheIde()) return "IDE not available";
    // Asynchronous — do not wait.
    PostCallback([] { if(TheIde()) TheIde()->Debug(); });
    return String();  // empty = ok
}
```

Note: `Debug()` or `BuildAndDebug()` — verify the exact name.

## DebugBridge::Stop()

```cpp
String DebugBridge::Stop()
{
    String err;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        ide->debugger->Stop();
    });
    return err;
}
```

## DebugBridge::Continue()

```cpp
String DebugBridge::Continue()
{
    String err;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        ide->debugger->Run();
    });
    return err;
}
```

## DebugBridge::StepOver/Into/Out

```cpp
String DebugBridge::StepOver()
{
    String err;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            g->Step("next");
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            l->Step("thread step-over");
        else
            err = "Step not supported for this backend";
    });
    return err;
}
```

`Gdb::Step(cmd)` and `LLDB::Step(cmd)` are already implemented in the Debuggers package —
they handle the GDB_PROMPT and update UI state.  We call them directly on the GUI thread.

## DebugBridge::Pause()

GDB: `Gdb::BreakRunning(pid)` is a static method that sends SIGINT to the debuggee.
LLDB: send `process interrupt` command.

```cpp
String DebugBridge::Pause()
{
    String err;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            Gdb::BreakRunning(g->pid);
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            LLDB::BreakRunning(l->pid);
        else
            err = "Pause not supported for this backend";
    });
    return err;
}
```

Note: `pid` is a protected field.  Either make `DebugBridge` a friend of `Gdb`/`LLDB`, or
add a public `GetPid()` accessor, or call `g->BreakRunning()` as a non-static method if that
variant exists.  Check `Gdb.h` — `BreakRunning()` (non-static) calls `Gdb::BreakRunning(pid)`.

## Async Response Timing

`debug.session.start` is fire-and-forget (returns immediately).  All others block on `RunOnGui`
with 5 second timeout.  If GDB is slow (e.g. stepping into a template), 5s may not be enough.
Consider 10s timeout for step operations.

## State After Steps

After `StepOver/Into/Out` returns, the debugger is paused again.  The client should call
`debug.state` to get the new file/line.  We do NOT poll or push state updates — the protocol
is purely request/response for now.

## Status: DONE
