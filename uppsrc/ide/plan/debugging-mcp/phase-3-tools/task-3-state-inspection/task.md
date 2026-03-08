# Task 3-3: State Inspection Tools

## Tools

- `debug.state`
- `debug.stack`
- `debug.locals`
- `debug.evaluate`
- `debug.threads`

## debug.state — DbgState

### DebugBridge::GetState()

```cpp
DbgState DebugBridge::GetState() const
{
    DbgState s;
    GuiLock __;
    Ide* ide = TheIde();
    if(!ide || !ide->debugger) {
        s.active = false;
        return s;
    }
    s.active = IdeIsDebug();
    s.file   = IdeGetFileName();
    s.line   = IdeGetFileLine();
    s.paused = s.active && !s.file.IsEmpty();

    // Backend detection via dynamic_cast
    if(dynamic_cast<Gdb*>(ide->debugger.Get()))       s.backend = "gdb";
    else if(dynamic_cast<LLDB*>(ide->debugger.Get())) s.backend = "lldb";
#ifdef PLATFORM_WIN32
    else if(dynamic_cast<Pdb*>(ide->debugger.Get()))  s.backend = "pdb";
#endif
    return s;
}
```

`IdeGetFileName()` returns the current source file shown in the editor (set by
`IdeSetDebugPos`).  It's empty when execution is running (not stopped).

`IdeIsDebug()` returns true while a debugger is active (from `ide/Core/Core.h`).

### ToValue(DbgState)

```cpp
inline Value ToValue(const DbgState& s) {
    ValueMap m;
    m.Add("active",  s.active);
    m.Add("paused",  s.paused);
    m.Add("file",    s.file);
    m.Add("line",    s.line);
    m.Add("backend", s.backend);
    return m;
}
```

## debug.stack — Call Stack

### DebugBridge::GetStackFrames()

Returns `Vector<DbgFrame>` parsed from `bt` output.

```cpp
Vector<DbgFrame> DebugBridge::GetStackFrames(int limit) const
{
    Vector<DbgFrame> frames;
    String raw;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide) return;
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            raw = g->Cmd(("bt " + AsString(limit)).Begin());
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            raw = l->Cmd("bt");
    });
    // Parse GDB bt output: lines like "#0  main (argc=1 ...) at main.cpp:5"
    ParseBt(raw, frames);
    return frames;
}
```

### ParseBt (GDB format)

```
#0  main (argc=1, argv=0x...) at /path/to/main.cpp:5
#1  0x... in __libc_start_main (...) from /lib/...
```

Parse with:
- Frame index: digit after `#`
- Function name: word after address / `in`
- File: after `at `
- Line: after `:`
- Address: hex after `#N  0x`

Use `CParser` or simple string scanning (follow pattern in `FormatFrame()` in `Gdb.cpp`).

## debug.locals — Local Variables

### DebugBridge::GetLocals()

GDB: `info locals` output format: `varname = value` (one per line, may span multiple lines
for complex types).

```cpp
VectorMap<String, String> DebugBridge::GetLocals() const
{
    VectorMap<String, String> result;
    String raw;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide) return;
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            raw = g->Cmd("info locals");
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            raw = l->Cmd("frame variable");
    });
    // Parse "name = value" lines
    StringStream ss(raw);
    while(!ss.IsEof()) {
        String line = ss.GetLine();
        int eq = line.Find('=');
        if(eq > 0) {
            String name  = TrimBoth(line.Left(eq));
            String value = TrimBoth(line.Mid(eq + 1));
            if(!name.IsEmpty())
                result.Add(name, value);
        }
    }
    return result;
}
```

### Response JSON

```json
{
  "locals": [
    { "name": "x", "value": "42", "type": "" },
    { "name": "s", "value": "\"hello\"", "type": "" }
  ]
}
```

Type info: GDB `info locals` doesn't include types by default.  Use `whatis <var>` to get type.
That requires a second round-trip per variable — skip for initial implementation, leave `"type"` as `""`.

## debug.evaluate — Expression Evaluation

### DebugBridge::Evaluate()

```cpp
String DebugBridge::Evaluate(const String& expr) const
{
    String result;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide) { result = "<IDE not available>"; return; }
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            result = g->Print(expr);   // Gdb::Print wraps "print <expr>"
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            result = l->Print(expr);   // LLDB::Print wraps "expression <expr>"
        else
            result = "<unsupported backend>";
    });
    return result;
}
```

`Gdb::Print(expr)` calls `Cmd("print " + expr)` and strips the `$N = ` prefix from the
response.  `Gdb::Print0` returns the raw result string.  Check implementation in `Gdb.cpp`.

## debug.threads — Thread List

### DebugBridge::GetThreads()

GDB: `info threads` output:
```
  Id   Target Id         Frame
* 1    Thread 0x... (LWP 12345) "main" at main.cpp:5
  2    Thread 0x... (LWP 12346) "worker" in pthread_cond_wait
```

```cpp
Vector<String> DebugBridge::GetThreads() const
{
    Vector<String> result;
    String raw;
    RunOnGui([&] {
        Ide* ide = TheIde();
        if(!ide) return;
        if(Gdb* g = dynamic_cast<Gdb*>(ide->debugger.Get()))
            raw = g->ObtainThreadsInfo();  // already exists in Gdb.h
        else if(LLDB* l = dynamic_cast<LLDB*>(ide->debugger.Get()))
            raw = l->ObtainThreadsInfo();
    });
    // Split by lines, trim, skip header
    StringStream ss(raw);
    ss.GetLine(); // skip header
    while(!ss.IsEof()) {
        String line = TrimBoth(ss.GetLine());
        if(!line.IsEmpty())
            result.Add(line);
    }
    return result;
}
```

`Gdb::ObtainThreadsInfo()` already exists — it calls `Cmd("info threads")` and returns raw output.

### Response JSON

```json
{
  "threads": [
    "* 1  Thread 0x... (LWP 12345) \"main\" at main.cpp:5",
    "  2  Thread 0x... (LWP 12346) \"worker\" in pthread_cond_wait"
  ]
}
```

## Thread Safety Summary for Phase 3

| Operation | Strategy |
|---|---|
| `GetState()` | `GuiLock` — no callbacks, instant read |
| `GetStackFrames()` | `RunOnGui` + `Cmd("bt")` — 5s timeout |
| `GetLocals()` | `RunOnGui` + `Cmd("info locals")` — 5s timeout |
| `Evaluate()` | `RunOnGui` + `Cmd("print expr")` — 5s timeout |
| `GetThreads()` | `RunOnGui` + `ObtainThreadsInfo()` — 5s timeout |

## Status: DONE
