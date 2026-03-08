# Task 4-1: MCP Resources for Debug State

## Overview

Add a `resource.get` method to `McpServer::HandleExtended` that serves read-only debug state
via URI-addressed resources.  This mirrors the TypeScript reference's `registerResource` pattern.

## Protocol Extension

### resource.list
Params: none
Returns: array of available resource descriptors.

```json
{
  "resources": [
    { "uri": "debug://state",       "title": "Debug session state",      "mimeType": "application/json" },
    { "uri": "debug://breakpoints", "title": "Active breakpoints",        "mimeType": "application/json" },
    { "uri": "debug://stack",       "title": "Call stack frames",         "mimeType": "application/json" },
    { "uri": "debug://locals",      "title": "Local variables",           "mimeType": "application/json" },
    { "uri": "debug://threads",     "title": "Thread list",               "mimeType": "application/json" }
  ]
}
```

### resource.get
Params: `{ "uri": "debug://state" }`
Returns: `{ "uri": "debug://state", "mimeType": "application/json", "text": "{ ... }" }`

## Handler Implementation

```cpp
if(req.method == "resource.list") {
    ValueArray arr;
    auto add = [&](const char* uri, const char* title) {
        ValueMap m;
        m.Add("uri", uri); m.Add("title", title); m.Add("mimeType", "application/json");
        arr.Add(m);
    };
    add("debug://state",       "Debug session state");
    add("debug://breakpoints", "Active breakpoints");
    add("debug://stack",       "Call stack frames");
    add("debug://locals",      "Local variables");
    add("debug://threads",     "Thread list");
    ValueMap r; r.Add("resources", arr);
    return MakeResult(req.id, r);
}

if(req.method == "resource.get") {
    if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
    String uri = AsString(ValueMap(req.params).Get("uri", Value()));

    String content;
    if(uri == "debug://state")
        content = AsJSON(ToValue(sDebugBridge.GetState()));
    else if(uri == "debug://breakpoints") {
        Vector<DbgBreakpoint> bps = sDebugBridge.GetBreakpoints();
        ValueArray arr;
        for(const DbgBreakpoint& b : bps) arr.Add(ToValue(b));
        content = AsJSON(arr);
    }
    else if(uri == "debug://stack") {
        Vector<DbgFrame> frames = sDebugBridge.GetStackFrames(20);
        ValueArray arr;
        for(const DbgFrame& f : frames) arr.Add(ToValue(f));
        content = AsJSON(arr);
    }
    else if(uri == "debug://locals") {
        VectorMap<String,String> locals = sDebugBridge.GetLocals();
        ValueArray arr;
        for(int i = 0; i < locals.GetCount(); i++) {
            ValueMap v; v.Add("name", locals.GetKey(i)); v.Add("value", locals[i]);
            arr.Add(v);
        }
        content = AsJSON(arr);
    }
    else if(uri == "debug://threads") {
        Vector<String> threads = sDebugBridge.GetThreads();
        ValueArray arr; for(const String& t : threads) arr.Add(t);
        content = AsJSON(arr);
    }
    else
        return MakeError(req.id, METHOD_NOT_FOUND, "Unknown resource URI: " + uri);

    ValueMap r;
    r.Add("uri",      uri);
    r.Add("mimeType", "application/json");
    r.Add("text",     content);
    return MakeResult(req.id, r);
}
```

## mcp.capabilities Update

Add `"resources"` to the capabilities response in `McpServerCore::Handle` (or `HandleExtended`):

```cpp
// In Handle("mcp.capabilities"):
r.Add("resources", true);
r.Add("resource_methods", "resource.list,resource.get");
```

## Status: TODO
