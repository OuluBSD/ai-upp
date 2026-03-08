# Task 3-1: Breakpoint Tools

## Tools

- `debug.breakpoint.set`
- `debug.breakpoint.clear`
- `debug.breakpoint.list`

## DebugBridge Implementation

### SetBreakpoint(file, line, condition)

```cpp
String DebugBridge::SetBreakpoint(const String& file, int line, const String& cond)
{
    String err;
    bool posted = RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        // Debugger::SetBreakpoint(filename, line, bp)
        // bp="" means remove; non-empty string means set (can be condition).
        String bp = cond.IsEmpty() ? "1" : cond; // "1" = unconditional breakpoint marker
        if(!ide->debugger->SetBreakpoint(file, line, bp))
            err = "SetBreakpoint returned false";
    });
    if(!posted) return "Timeout posting to GUI thread";
    if(!err.IsEmpty()) return err;

    // Update tracked list
    Mutex::Lock __(bp_mutex);
    // Remove existing on same location, then add
    for(int i = breakpoints.GetCount() - 1; i >= 0; i--)
        if(breakpoints[i].file == file && breakpoints[i].line == line)
            breakpoints.Remove(i);
    DbgBreakpoint& b = breakpoints.Add();
    b.file = file; b.line = line; b.condition = cond; b.enabled = true;
    return String();
}
```

### ClearBreakpoint(file, line)

```cpp
String DebugBridge::ClearBreakpoint(const String& file, int line)
{
    String err;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide || !ide->debugger) { err = "No active debugger"; return; }
        // bp="" clears the breakpoint per Debugger interface contract
        ide->debugger->SetBreakpoint(file, line, "");
    });

    Mutex::Lock __(bp_mutex);
    for(int i = breakpoints.GetCount() - 1; i >= 0; i--)
        if(breakpoints[i].file == file && breakpoints[i].line == line)
            breakpoints.Remove(i);
    return err;
}
```

### GetBreakpoints()

```cpp
Vector<DbgBreakpoint> DebugBridge::GetBreakpoints() const
{
    Mutex::Lock __(bp_mutex);
    return clone(breakpoints);  // U++ clone() for deep copy
}
```

## Breakpoint Without Active Session

`Debugger::SetBreakpoint` is also used to set *pending* breakpoints before `debug.session.start`.
TheIDE's GDB backend calls `break file:line` on start if breakpoints were pre-set.

However, without an active debugger the `DebugBridge` can still track the breakpoint list.
On `debug.session.start`, the IDE's `StartDebug()` flow will call `SetBreakpoint` on the
created debugger for each pre-set breakpoint — but only if the IDE normally does this.

**Verify**: Does TheIDE persist breakpoints between sessions?  Check `ide.cpp` / `SerializeSession`.
If yes, we don't need to replay them.  If no, we may need to replay from `sDebugBridge.breakpoints`
after session start.  Defer to Phase 4 if needed.

## File Path Normalization

AI clients may send relative paths or paths with different separators.  Normalize:
```cpp
String NormalizeDebugPath(const String& p) {
    // Convert to absolute if relative, using IdeGetNestFolder() as base
    String r = NormalizePath(p);
    return r;
}
```

Apply in `SetBreakpoint` and `ClearBreakpoint` before matching/passing to debugger.

## JSON Protocol

### debug.breakpoint.set
Request params:
```json
{ "file": "uppsrc/ide/main.cpp", "line": 42, "condition": "" }
```
Response:
```json
{ "set": true, "file": "uppsrc/ide/main.cpp", "line": 42 }
```
Error (line <= 0):
```json
{ "error": { "code": -32602, "message": "file and line (>0) required" } }
```

### debug.breakpoint.clear
Request params: `{ "file": "...", "line": 42 }`
Response: `{ "cleared": true }`

### debug.breakpoint.list
Request params: `{}` or none
Response:
```json
{
  "breakpoints": [
    { "file": "main.cpp", "line": 10, "condition": "", "enabled": true },
    { "file": "foo.cpp",  "line": 25, "condition": "x > 0", "enabled": true }
  ]
}
```

## Status: DONE
