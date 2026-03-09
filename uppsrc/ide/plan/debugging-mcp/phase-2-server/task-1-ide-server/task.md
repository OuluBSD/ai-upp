# Task 2-1: IDE Server — debug.* handlers in Server.cpp

## File to Edit

`uppsrc/ide/MCP/Server.cpp` — extend `McpServer::HandleExtended`

## Pattern

Follow the existing pattern exactly:
```cpp
if(req.method == "debug.state") {
    DbgState s = sDebugBridge.GetState();
    return MakeResult(req.id, ToValue(s));
}
```

## Full Handler List

### debug.state
Params: none
Returns: `DbgState` as ValueMap (`active`, `paused`, `file`, `line`, `backend`).

```cpp
if(req.method == "debug.state") {
    return MakeResult(req.id, ToValue(sDebugBridge.GetState()));
}
```

### debug.session.start
Params: none (uses IDE's currently selected build method and main package)
Returns: `{ "accepted": true }` immediately; actual start is async.

Implementation: Call `PostCallback([] { if(TheIde()) TheIde()->StartDebug(); });`
`StartDebug` is declared in `ide.h` / implemented in `ide.cpp` — verify exact name with grep.

```cpp
if(req.method == "debug.session.start") {
    if(!TheIde()) return MakeError(req.id, INTERNAL_ERROR, "IDE not available");
    PostCallback([] { TheIde()->StartDebug(); });
    ValueMap r; r.Add("accepted", true);
    return MakeResult(req.id, r);
}
```

### debug.session.stop
Params: none
Returns: `{ "accepted": true }`

```cpp
if(req.method == "debug.session.stop") {
    String err = sDebugBridge.Stop();
    if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
    ValueMap r; r.Add("accepted", true);
    return MakeResult(req.id, r);
}
```

### debug.continue
Params: none
Returns: `{ "accepted": true }`

```cpp
if(req.method == "debug.continue") {
    String err = sDebugBridge.Continue();
    if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
    ValueMap r; r.Add("accepted", true);
    return MakeResult(req.id, r);
}
```

### debug.step.over / debug.step.into / debug.step.out / debug.pause

Same pattern as `debug.continue` calling the respective `DebugBridge` method.

### debug.breakpoint.set
Params: `{ "file": "path/to/file.cpp", "line": 42, "condition": "" }`
Returns: `{ "set": true, "file": "...", "line": 42 }`

```cpp
if(req.method == "debug.breakpoint.set") {
    if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
    ValueMap p = req.params;
    String file = AsString(p.Get("file", Value()));
    int line = (int)p.Get("line", 0);
    String cond = AsString(p.Get("condition", Value()));
    if(file.IsEmpty() || line <= 0)
        return MakeError(req.id, INVALID_PARAMS, "file and line (>0) required");
    String err = sDebugBridge.SetBreakpoint(file, line, cond);
    if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
    ValueMap r; r.Add("set", true); r.Add("file", file); r.Add("line", line);
    return MakeResult(req.id, r);
}
```

### debug.breakpoint.clear
Params: `{ "file": "...", "line": 42 }`
Returns: `{ "cleared": true }`

### debug.breakpoint.list
Params: none
Returns: `{ "breakpoints": [ { "file", "line", "condition", "enabled" }, ... ] }`

```cpp
if(req.method == "debug.breakpoint.list") {
    Vector<DbgBreakpoint> bps = sDebugBridge.GetBreakpoints();
    ValueArray arr;
    for(const DbgBreakpoint& b : bps)
        arr.Add(ToValue(b));
    ValueMap r; r.Add("breakpoints", arr);
    return MakeResult(req.id, r);
}
```

### debug.stack
Params: none (optional: `{ "limit": 20 }`)
Returns: `{ "frames": [ { "index", "function", "file", "line", "address" }, ... ] }`

```cpp
if(req.method == "debug.stack") {
    // sDebugBridge.GetStackTrace() returns raw bt text; parse it here or in bridge.
    // Better: return as { "raw": "...", "frames": [...] }
}
```

### debug.locals
Params: none
Returns: `{ "locals": [ { "name", "value", "type" }, ... ] }`

### debug.evaluate
Params: `{ "expression": "myVar.field" }`
Returns: `{ "result": "42" }`

```cpp
if(req.method == "debug.evaluate") {
    if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
    String expr = AsString(ValueMap(req.params).Get("expression", Value()));
    if(expr.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "expression required");
    String result = sDebugBridge.Evaluate(expr);
    ValueMap r; r.Add("result", result);
    return MakeResult(req.id, r);
}
```

### debug.threads
Params: none
Returns: `{ "threads": [ "Thread 1: main at ...", ... ] }`

## Error Guard

For all handlers that require an active debug session:
```cpp
DbgState st = sDebugBridge.GetState();
if(!st.active) return MakeError(req.id, INTERNAL_ERROR, "No active debug session");
if(!st.paused) return MakeError(req.id, INTERNAL_ERROR, "Not paused — hit a breakpoint first");
```

Apply this guard to: `debug.stack`, `debug.locals`, `debug.evaluate`, `debug.threads`,
`debug.step.*`, `debug.continue`.

## mcp_client.sh Update

Add all new `debug.*` methods to the grep pattern so the test script auto-discovers them.
The script already greps `req.method == "..."` — the new handlers follow the same pattern.

## Status: DONE
