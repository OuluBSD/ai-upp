#ifdef flagGUI
#include "MCP.h"
#include <ide/ide.h>

NAMESPACE_UPP

McpServer sMcpServer;

bool McpServer::Start(int port) {
    if(!port && TheIde())
        port = TheIde()->mcp_server_port;
    Thread::AtShutdown(StopMcpServer);
    return McpServerCore::Start(port ? port : 7326);
}

void McpServer::Stop() {
    McpServerCore::Stop();
}

String McpServer::HandleExtended(const McpRequest& req) {
    if(req.method == "workspace.info") {
        ValueMap r; r.Add("name", GetCurrentWorkspaceName()); r.Add("packages", GetCurrentWorkspacePackageCount());
        return MakeResult(req.id, r);
    }
    if(req.method == "mcp.index.status") {
        ValueMap r; String builder = "unknown"; bool ready = false;
        if(const MakeBuild* make = TheIde()) { VectorMap<String, String> bm = GetMethodVars(make->method); builder = bm.Get("BUILDER", "GCC"); ready = ToUpper(builder) == "SCRIPT"; }
        r.Add("ready", ready); r.Add("builder", builder); r.Add("last_update", (int)0); r.Add("stale_files", 0);
        if(!ready) r.Add("note", "SCRIPT builder required to populate AST/index via ScriptBuilder");
        return MakeResult(req.id, r);
    }
    if(req.method == "mcp.index.refresh") { ValueMap r; r.Add("accepted", true); r.Add("mode", "script_build"); return MakeResult(req.id, r); }
#ifndef flagV1
    if(req.method == "node.locate") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params; String file = AsString(p.Get("file", Value())); int line = (int)(p.Get("line", 0)); int col = (int)(p.Get("column", 0));
        ValueMap r; EnvStatusInfo st = EnvStatus();
        if(st.initialized) { EnvNodeInfo ei = EnvLocate(file, line, col); r.Add("id", ei.id); r.Add("kind", ei.kind); r.Add("name", ei.name); r.Add("file", ei.file); r.Add("start_line", ei.start_line); r.Add("start_col", ei.start_col); r.Add("end_line", ei.end_line); r.Add("end_col", ei.end_col); }
        else { McpNode n = sMcpIndex.Locate(file, line, col); r.Add("id", n.id); r.Add("kind", n.kind); r.Add("name", n.name); r.Add("file", n.file); r.Add("start_line", n.start_line); r.Add("start_col", n.start_col); r.Add("end_line", n.end_line); r.Add("end_col", n.end_col); r.Add("note", "index_not_ready"); }
        return MakeResult(req.id, r);
    }
    if(req.method == "node.get") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params; String id = AsString(p.Get("id", Value())); ValueMap r; EnvStatusInfo st = EnvStatus();
        if(st.initialized) { EnvNodeInfo ei = EnvGet(id); r.Add("id", ei.id); r.Add("kind", ei.kind); r.Add("name", ei.name); r.Add("file", ei.file); r.Add("start_line", ei.start_line); r.Add("start_col", ei.start_col); r.Add("end_line", ei.end_line); r.Add("end_col", ei.end_col); r.Add("code", EnvCodeById(id)); }
        else { McpNode n = sMcpIndex.Get(id); r.Add("id", n.id); r.Add("kind", n.kind); r.Add("name", n.name); r.Add("file", n.file); r.Add("start_line", n.start_line); r.Add("start_col", n.start_col); r.Add("end_line", n.end_line); r.Add("end_col", n.end_col); r.Add("code", String()); r.Add("note", "index_not_ready"); }
        return MakeResult(req.id, r);
    }
    if(req.method == "node.definition") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params; String id = AsString(p.Get("id", Value())); ValueArray items; EnvStatusInfo st = EnvStatus();
        if(st.initialized) { Vector<EnvNodeInfo> defs = EnvDefinition(id); for(const auto& d : defs) { ValueMap v; v.Add("id", d.id); v.Add("file", d.file); v.Add("start_line", d.start_line); v.Add("start_col", d.start_col); v.Add("end_line", d.end_line); v.Add("end_col", d.end_col); items.Add(v); } }
        ValueMap r; r.Add("items", items); if(!st.initialized) r.Add("note", "index_not_ready"); return MakeResult(req.id, r);
    }
    if(req.method == "node.references") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params; String id = AsString(p.Get("id", Value())); String page = AsString(p.Get("page_token", String())); int limit = (int)(p.Get("limit", 200));
        ValueArray items; String next; EnvStatusInfo st = EnvStatus();
        if(st.initialized) { EnvRefPage pg = EnvReferences(id, page, limit); next = pg.next_page_token; for(const auto& it : pg.items) { ValueMap v; v.Add("id", it.id); v.Add("file", it.file); v.Add("start_line", it.start_line); v.Add("start_col", it.start_col); v.Add("end_line", it.end_line); v.Add("end_col", it.end_col); items.Add(v); } }
        ValueMap r; r.Add("items", items); r.Add("next_page_token", next); if(!st.initialized) r.Add("note", "index_not_ready"); return MakeResult(req.id, r);
    }
#endif // !flagV1
    if(req.method == "edits.apply") {
        if(!IsValueArray(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected params as array of edits");
        ValueArray a = req.params; for(const Value& v : a) if(!IsValueMap(v)) return MakeError(req.id, INVALID_PARAMS, "Edit must be object"); ValueMap r; r.Add("applied", a.GetCount()); return MakeResult(req.id, r);
    }

    // --- build / run handlers ------------------------------------------------

    if(req.method == "build.start") {
        String err = sDebugBridge.BuildStart();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "build.stop") {
        String err = sDebugBridge.BuildStop();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "build.status") {
        ValueMap r; r.Add("building", sDebugBridge.IsBuilding()); return MakeResult(req.id, r);
    }
    if(req.method == "run.start") {
        String err = sDebugBridge.RunStart();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }

    // --- debug.* handlers ---------------------------------------------------

    if(req.method == "debug.state") {
        return MakeResult(req.id, ToValue(sDebugBridge.GetState()));
    }
    if(req.method == "debug.session.start") {
        String err = sDebugBridge.Start();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.session.stop") {
        String err = sDebugBridge.Stop();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.continue") {
        String err = sDebugBridge.Continue();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.step.over") {
        String err = sDebugBridge.StepOver();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.step.into") {
        String err = sDebugBridge.StepInto();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.step.out") {
        String err = sDebugBridge.StepOut();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.pause") {
        String err = sDebugBridge.Pause();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.breakpoint.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        ValueMap p = req.params;
        String file = AsString(p.Get("file", Value()));
        int    line = (int)(p.Get("line", 0));
        String cond = AsString(p.Get("condition", Value()));
        if(file.IsEmpty() || line <= 0) return MakeError(req.id, INVALID_PARAMS, "file and line (>0) required");
        String err = sDebugBridge.SetBreakpoint(file, line, cond);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("set", true); r.Add("file", file); r.Add("line", line); return MakeResult(req.id, r);
    }
    if(req.method == "debug.breakpoint.clear") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        ValueMap p = req.params;
        String file = AsString(p.Get("file", Value()));
        int    line = (int)(p.Get("line", 0));
        if(file.IsEmpty() || line <= 0) return MakeError(req.id, INVALID_PARAMS, "file and line (>0) required");
        String err = sDebugBridge.ClearBreakpoint(file, line);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("cleared", true); return MakeResult(req.id, r);
    }
    if(req.method == "debug.breakpoint.list") {
        Vector<DbgBreakpoint> bps = sDebugBridge.GetBreakpoints();
        ValueArray arr;
        for(const DbgBreakpoint& b : bps) arr.Add(ToValue(b));
        ValueMap r; r.Add("breakpoints", arr); return MakeResult(req.id, r);
    }
    if(req.method == "debug.stack") {
        DbgState st = sDebugBridge.GetState();
        if(!st.active)  return MakeError(req.id, INTERNAL_ERROR, "No active debug session");
        if(!st.paused)  return MakeError(req.id, INTERNAL_ERROR, "Not paused — hit a breakpoint first");
        int limit = 30;
        if(IsValueMap(req.params)) limit = max(1, min(200, (int)(ValueMap(req.params).Get("limit", limit))));
        Vector<DbgFrame> frames = sDebugBridge.GetStackFrames(limit);
        ValueArray arr;
        for(const DbgFrame& f : frames) arr.Add(ToValue(f));
        ValueMap r; r.Add("frames", arr); return MakeResult(req.id, r);
    }
    if(req.method == "debug.locals") {
        DbgState st = sDebugBridge.GetState();
        if(!st.active)  return MakeError(req.id, INTERNAL_ERROR, "No active debug session");
        if(!st.paused)  return MakeError(req.id, INTERNAL_ERROR, "Not paused — hit a breakpoint first");
        VectorMap<String,String> locals = sDebugBridge.GetLocals();
        ValueArray arr;
        for(int i = 0; i < locals.GetCount(); i++) {
            ValueMap v; v.Add("name", locals.GetKey(i)); v.Add("value", locals[i]); v.Add("type", ""); arr.Add(v);
        }
        ValueMap r; r.Add("locals", arr); return MakeResult(req.id, r);
    }
    if(req.method == "debug.evaluate") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String expr = AsString(ValueMap(req.params).Get("expression", Value()));
        if(expr.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "expression required");
        DbgState st = sDebugBridge.GetState();
        if(!st.active)  return MakeError(req.id, INTERNAL_ERROR, "No active debug session");
        if(!st.paused)  return MakeError(req.id, INTERNAL_ERROR, "Not paused — hit a breakpoint first");
        String result = sDebugBridge.Evaluate(expr);
        ValueMap r; r.Add("result", result); return MakeResult(req.id, r);
    }
    if(req.method == "debug.threads") {
        DbgState st = sDebugBridge.GetState();
        if(!st.active)  return MakeError(req.id, INTERNAL_ERROR, "No active debug session");
        Vector<String> threads = sDebugBridge.GetThreads();
        ValueArray arr; for(const String& t : threads) arr.Add(t);
        ValueMap r; r.Add("threads", arr); return MakeResult(req.id, r);
    }

    // --- resource.* handlers (Phase 4 stub) ---------------------------------

    if(req.method == "resource.list") {
        ValueArray arr;
        auto addRes = [&](const char* uri, const char* title) {
            ValueMap m; m.Add("uri", uri); m.Add("title", title); m.Add("mimeType", "application/json"); arr.Add(m);
        };
        addRes("debug://state",       "Debug session state");
        addRes("debug://breakpoints", "Active breakpoints");
        addRes("debug://stack",       "Call stack frames");
        addRes("debug://locals",      "Local variables");
        addRes("debug://threads",     "Thread list");
        ValueMap r; r.Add("resources", arr); return MakeResult(req.id, r);
    }
    if(req.method == "resource.get") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String uri = AsString(ValueMap(req.params).Get("uri", Value()));
        String content;
        if(uri == "debug://state")
            content = AsJSON(ToValue(sDebugBridge.GetState()));
        else if(uri == "debug://breakpoints") {
            Vector<DbgBreakpoint> bps = sDebugBridge.GetBreakpoints();
            ValueArray a; for(const DbgBreakpoint& b : bps) a.Add(ToValue(b)); content = AsJSON(a);
        }
        else if(uri == "debug://stack") {
            Vector<DbgFrame> frames = sDebugBridge.GetStackFrames(30);
            ValueArray a; for(const DbgFrame& f : frames) a.Add(ToValue(f)); content = AsJSON(a);
        }
        else if(uri == "debug://locals") {
            VectorMap<String,String> locs = sDebugBridge.GetLocals();
            ValueArray a;
            for(int i = 0; i < locs.GetCount(); i++) { ValueMap v; v.Add("name", locs.GetKey(i)); v.Add("value", locs[i]); a.Add(v); }
            content = AsJSON(a);
        }
        else if(uri == "debug://threads") {
            Vector<String> threads = sDebugBridge.GetThreads();
            ValueArray a; for(const String& t : threads) a.Add(t); content = AsJSON(a);
        }
        else return MakeError(req.id, METHOD_NOT_FOUND, "Unknown resource URI: " + uri);
        ValueMap r; r.Add("uri", uri); r.Add("mimeType", "application/json"); r.Add("text", content);
        return MakeResult(req.id, r);
    }

    // --- mainconfig handlers -------------------------------------------------

    if(req.method == "mainconfig.list") {
        ValueMap r; r.Add("configs", sIdeBridge.ListMainConfigs()); r.Add("current", sIdeBridge.GetMainConfig());
        return MakeResult(req.id, r);
    }
    if(req.method == "mainconfig.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String param = AsString(ValueMap(req.params).Get("param", Value()));
        if(param.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "param required");
        String err = sIdeBridge.SetMainConfig(param);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("set", param); return MakeResult(req.id, r);
    }

    // --- build method handlers -----------------------------------------------

    if(req.method == "buildmethod.list") {
        ValueMap r; r.Add("methods", sIdeBridge.ListBuildMethods()); r.Add("current", sIdeBridge.GetBuildMethod());
        return MakeResult(req.id, r);
    }
    if(req.method == "buildmethod.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String name = AsString(ValueMap(req.params).Get("name", Value()));
        if(name.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "name required");
        String err = sIdeBridge.SetBuildMethod(name);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("set", name); return MakeResult(req.id, r);
    }

    // --- build mode (debug/release) ------------------------------------------

    if(req.method == "buildmode.get") {
        ValueMap r; r.Add("mode", sIdeBridge.GetBuildMode()); return MakeResult(req.id, r);
    }
    if(req.method == "buildmode.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String mode = AsString(ValueMap(req.params).Get("mode", Value()));
        String err = sIdeBridge.SetBuildMode(mode);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("set", mode); return MakeResult(req.id, r);
    }

    // --- package handlers ----------------------------------------------------

    if(req.method == "package.list") {
        ValueMap r; r.Add("packages", sIdeBridge.ListAssemblyPackages()); r.Add("active", sIdeBridge.GetActivePackage());
        return MakeResult(req.id, r);
    }
    if(req.method == "package.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String name = AsString(ValueMap(req.params).Get("name", Value()));
        if(name.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "name required");
        String err = sIdeBridge.SetActivePackage(name);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("set", name); return MakeResult(req.id, r);
    }
    if(req.method == "package.files") {
        String pkg;
        if(IsValueMap(req.params)) pkg = AsString(ValueMap(req.params).Get("package", Value()));
        if(pkg.IsEmpty()) pkg = sIdeBridge.GetActivePackage();
        ValueMap r; r.Add("package", pkg); r.Add("files", sIdeBridge.ListPackageFiles(pkg));
        return MakeResult(req.id, r);
    }

    // --- file / editor handlers ----------------------------------------------

    if(req.method == "file.get") {
        ValueMap r; r.Add("path", sIdeBridge.GetActiveFile()); return MakeResult(req.id, r);
    }
    if(req.method == "file.open") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String path = AsString(ValueMap(req.params).Get("path", Value()));
        if(path.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "path required");
        String err = sIdeBridge.SetActiveFile(path);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("opened", path); return MakeResult(req.id, r);
    }
    if(req.method == "editor.path") {
        int idx = 0;
        if(IsValueMap(req.params)) idx = (int)(ValueMap(req.params).Get("editor", 0));
        ValueMap r; r.Add("path", sIdeBridge.GetEditorPath(idx)); return MakeResult(req.id, r);
    }
    if(req.method == "editor.cursor.get") {
        int idx = 0;
        if(IsValueMap(req.params)) idx = (int)(ValueMap(req.params).Get("editor", 0));
        auto pos = sIdeBridge.GetEditorCursor(idx);
        ValueMap r; r.Add("line", pos.line); r.Add("col", pos.col); r.Add("cursor", (int)pos.cursor);
        return MakeResult(req.id, r);
    }
    if(req.method == "editor.cursor.set") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        ValueMap p = req.params;
        int idx = (int)(p.Get("editor", 0));
        if(p.Find("cursor") >= 0) {
            int64 cursor = (int64)(int)(p.Get("cursor", 0));
            String err = sIdeBridge.SetEditorCursorPos(cursor, idx);
            if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        } else {
            int line = (int)(p.Get("line", 0));
            int col  = (int)(p.Get("col", 0));
            String err = sIdeBridge.SetEditorCursor(line, col, idx);
            if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        }
        ValueMap r; r.Add("set", true); return MakeResult(req.id, r);
    }
    if(req.method == "editor.lines") {
        int idx = 0;
        if(IsValueMap(req.params)) idx = (int)(ValueMap(req.params).Get("editor", 0));
        ValueMap r; r.Add("count", sIdeBridge.GetEditorLineCount(idx)); return MakeResult(req.id, r);
    }
    if(req.method == "editor.line.get") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        ValueMap p = req.params;
        int line = (int)(p.Get("line", 0));
        int idx  = (int)(p.Get("editor", 0));
        ValueMap r; r.Add("line", line); r.Add("text", sIdeBridge.GetEditorLine(line, idx));
        return MakeResult(req.id, r);
    }
    if(req.method == "editor.insert") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String text = AsString(ValueMap(req.params).Get("text", Value()));
        String err = sIdeBridge.InsertEditorText(text);
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("inserted", true); return MakeResult(req.id, r);
    }

    // --- console / errors ----------------------------------------------------

    if(req.method == "console.get") {
        int tail = 0;
        if(IsValueMap(req.params)) tail = (int)(ValueMap(req.params).Get("tail", 0));
        String text = tail > 0 ? sIdeBridge.GetConsoleTail(tail) : sIdeBridge.GetConsoleText();
        ValueMap r; r.Add("text", text); return MakeResult(req.id, r);
    }
    if(req.method == "errors.get") {
        auto errs = sIdeBridge.GetErrors();
        ValueArray arr;
        for(const auto& e : errs) {
            ValueMap v; v.Add("file", e.file); v.Add("line", e.line); v.Add("text", e.text); v.Add("warning", e.is_warning);
            arr.Add(v);
        }
        ValueMap r; r.Add("errors", arr); r.Add("error_count", sIdeBridge.GetErrorCount()); r.Add("warning_count", sIdeBridge.GetWarningCount());
        return MakeResult(req.id, r);
    }

    // --- find ----------------------------------------------------------------

    if(req.method == "find.infiles") {
        if(!IsValueMap(req.params)) return MakeError(req.id, INVALID_PARAMS, "Expected object");
        String pattern = AsString(ValueMap(req.params).Get("pattern", Value()));
        if(pattern.IsEmpty()) return MakeError(req.id, INVALID_PARAMS, "pattern required");
        bool replace = (bool)(int)(ValueMap(req.params).Get("replace", 0));
        sIdeBridge.FindInFiles(pattern, replace);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "find.next") {
        String err = sIdeBridge.EditorFindNext();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "find.prev") {
        String err = sIdeBridge.EditorFindPrev();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }

    // --- valgrind ------------------------------------------------------------

    if(req.method == "valgrind.run") {
        String err = sIdeBridge.RunValgrind();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }

    // --- assist --------------------------------------------------------------

    if(req.method == "assist.suggestions") {
        ValueMap r; r.Add("items", sIdeBridge.GetAssistSuggestions()); return MakeResult(req.id, r);
    }
    if(req.method == "assist.goto") {
        String err = sIdeBridge.AssistGoto();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "assist.usage") {
        String err = sIdeBridge.AssistUsage();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "assist.query") {
        String err = sIdeBridge.AssistQueryId();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }
    if(req.method == "assist.context_goto") {
        String err = sIdeBridge.AssistContextGoto();
        if(!err.IsEmpty()) return MakeError(req.id, INTERNAL_ERROR, err);
        ValueMap r; r.Add("accepted", true); return MakeResult(req.id, r);
    }

    return String();
}

bool StartMcpServer(const McpConfig&) { Thread::AtShutdown(StopMcpServer); return sMcpServer.Start(); }
void StopMcpServer() { sMcpServer.Stop(); }
bool McpIsRunning() { return sMcpServer.IsRunning(); }

END_UPP_NAMESPACE
#endif // flagGUI
