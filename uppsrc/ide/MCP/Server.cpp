#include <ide/ide.h>

NAMESPACE_UPP

McpServer sMcpServer;

bool McpServer::Start(int port) {
    if(running)
        return true;
    if (!port && TheIde())
        port = TheIde()->mcp_server_port;
    listen_port = port ? port : 7326; // default MCP port
    stop = 0;
    thr.Run([=] { const_cast<McpServer*>(this)->Loop(); });
    running = true;
    return true;
}

void McpServer::Stop() {
    if(!running)
        return;
    stop = 1;
    if(thr.IsOpen())
        thr.Wait();
    if(clients_thr.IsOpen())
        clients_thr.Wait();
    running = false;
}

void McpServer::Loop() {
    TcpSocket listener;
    if(!listener.Listen(listen_port, 128, true)) {
        RLOG("MCP: listen failed on port " << listen_port << ": " << listener.GetErrorDesc());
        return;
    }
    RLOG("MCP: listening on port " << listen_port);
    listener.NoDelay();

	clients_thr.Start(THISBACK(HandleClients));
	
    while(!stop) {
        // Accept new clients non-blocking
        One<TcpSocket> s;
        s.Create();
        s->Timeout(5);
        if(s->Accept(listener)) {
            s->NoDelay();
            McpClient c;
            c.sock = s.Detach();
            c.last_activity = GetSysTime();
            c.id = next_client_id++;
            clients_lock.EnterWrite();
            clients.Add(pick(c));
            clients_lock.LeaveWrite();
        }
    }
}

void McpServer::HandleClients() {
    while(!stop) {
        // Handle client IO
        for(int i = 0;; i++) {
            clients_lock.EnterRead();
            if (i >= clients.GetCount()) {
                clients_lock.LeaveRead();
                break;
            }
            McpClient& c = clients[i];
            clients_lock.LeaveRead();
            if(!c.sock->IsOpen()) { clients_lock.EnterWrite(); clients.Remove(i--); clients_lock.LeaveWrite(); continue; }
            Vector<String> msgs;
            if(ReadFramed(c, msgs)) {
                for(const String& line : msgs) {
                    McpRequest req;
                    String reply;
                    if(ParseRequest(line, req)) {
                        reply = Handle(req);
                        if(!IsNotification(req)) {
                            reply.Cat('\n');
                            c.outbuf.Cat(reply);
                        }
                    } else {
                        // Parse error → respond with null id per spec
                        reply = MakeError(String(), PARSE_ERROR, "Parse error");
                        reply.Cat('\n');
                        c.outbuf.Cat(reply);
                    }
                }
            }
            WritePending(c);
            if(c.sock->IsError() || c.sock->IsEof()) { clients_lock.EnterWrite(); clients.Remove(i--); clients_lock.LeaveWrite(); continue; }
        }
        Sleep(5);
    }
    // Cleanup
    for(auto& c : clients) if(c.sock->IsOpen()) c.sock->Close();
    
    clients_lock.EnterWrite();
    clients.Clear();
    clients_lock.LeaveWrite();
}

String McpServer::Handle(const McpRequest& req) {
    if(IsNull(req.method))
        return MakeError(req.id, INVALID_REQUEST, "Invalid request: missing method");
    if(req.method == "mcp.ping") {
        ValueMap r; r.Add("text", "pong");
        return MakeResult(req.id, r);
    }
    if(req.method == "mcp.capabilities") {
        ValueArray methods;
        methods.Add("mcp.ping");
        methods.Add("mcp.capabilities");
        methods.Add("workspace.info");
        methods.Add("mcp.index.status");
        methods.Add("mcp.index.refresh");
        methods.Add("node.locate");
        methods.Add("node.get");
        methods.Add("node.definition");
        methods.Add("node.references");
        methods.Add("edits.apply");
        ValueMap caps;
        caps.Add("protocol", "jsonrpc-2.0");
        caps.Add("supports_batch", false);
        caps.Add("methods", methods);
        return MakeResult(req.id, caps);
    }
    if(req.method == "workspace.info") {
        ValueMap r;
        r.Add("name", GetCurrentWorkspaceName());
        r.Add("packages", GetCurrentWorkspacePackageCount());
        return MakeResult(req.id, r);
    }
    if(req.method == "mcp.index.status") {
        ValueMap r;
        String builder = "unknown";
        bool ready = false;
        if(const MakeBuild* make = TheIde()) {
            VectorMap<String, String> bm = GetMethodVars(make->method);
            builder = bm.Get("BUILDER", "GCC");
            ready = ToUpper(builder) == "SCRIPT";
        }
        // Access the environments (future use)
        IdeMetaEnvironment& ienv = IdeMetaEnv();
        MetaEnvironment& env = ienv.env;
        r.Add("ready", ready);
        r.Add("builder", builder);
        r.Add("last_update", (int)0);
        r.Add("stale_files", 0);
        return MakeResult(req.id, r);
    }
    if(req.method == "mcp.index.refresh") {
        // Placeholder: Accept and report that refresh was requested.
        ValueMap r;
        r.Add("accepted", true);
        r.Add("mode", "script_build");
        return MakeResult(req.id, r);
    }
    // Node/query handlers (env-backed with fallback)
    if(req.method == "node.locate") {
        if(!IsValueMap(req.params))
            return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params;
        String file = AsString(p.Get("file", Value())) ;
        int line = (int)p.Get("line", 0);
        int col  = (int)p.Get("column", 0);
        ValueMap r;
        EnvStatusInfo st = EnvStatus();
        if(st.initialized) {
            EnvNodeInfo ei = EnvLocate(file, line, col);
            r.Add("id", ei.id);
            r.Add("kind", ei.kind);
            r.Add("name", ei.name);
            r.Add("file", ei.file);
            r.Add("start_line", ei.start_line);
            r.Add("start_col", ei.start_col);
            r.Add("end_line", ei.end_line);
            r.Add("end_col", ei.end_col);
        } else {
            McpNode n = sMcpIndex.Locate(file, line, col);
            r.Add("id", n.id);
            r.Add("kind", n.kind);
            r.Add("name", n.name);
            r.Add("file", n.file);
            r.Add("start_line", n.start_line);
            r.Add("start_col", n.start_col);
            r.Add("end_line", n.end_line);
            r.Add("end_col", n.end_col);
            r.Add("note", "index_not_ready");
        }
        return MakeResult(req.id, r);
    }
    if(req.method == "node.get") {
        if(!IsValueMap(req.params))
            return MakeError(req.id, INVALID_PARAMS, "Expected params object");
        ValueMap p = req.params;
        String id = AsString(p.Get("id", Value()));
        ValueMap r;
        EnvStatusInfo st = EnvStatus();
        if(st.initialized) {
            EnvNodeInfo ei = EnvGet(id);
            r.Add("id", ei.id);
            r.Add("kind", ei.kind);
            r.Add("name", ei.name);
            r.Add("file", ei.file);
            r.Add("start_line", ei.start_line);
            r.Add("start_col", ei.start_col);
            r.Add("end_line", ei.end_line);
            r.Add("end_col", ei.end_col);
            r.Add("code", EnvCodeById(id));
        } else {
            McpNode n = sMcpIndex.Get(id);
            r.Add("id", n.id);
            r.Add("kind", n.kind);
            r.Add("name", n.name);
            r.Add("file", n.file);
            r.Add("start_line", n.start_line);
            r.Add("start_col", n.start_col);
            r.Add("end_line", n.end_line);
            r.Add("end_col", n.end_col);
            r.Add("code", String());
            r.Add("note", "index_not_ready");
        }
        return MakeResult(req.id, r);
    }
    if(req.method == "node.definition") {
        ValueMap r; r.Add("status", "not_implemented");
        return MakeResult(req.id, r);
    }
    if(req.method == "node.references") {
        ValueMap r; r.Add("status", "not_implemented");
        return MakeResult(req.id, r);
    }
    if(req.method == "edits.apply") {
        // Accepts array of planned edits; currently just validates shape
        if(!IsValueArray(req.params))
            return MakeError(req.id, INVALID_PARAMS, "Expected params as array of edits");
        ValueArray a = req.params;
        for(const Value& v : a) {
            if(!IsValueMap(v))
                return MakeError(req.id, INVALID_PARAMS, "Edit must be object");
        }
        ValueMap r; r.Add("applied", a.GetCount());
        return MakeResult(req.id, r);
    }
    return MakeError(req.id, METHOD_NOT_FOUND, "Method not found");
}

bool StartMcpServer(const McpConfig&) {
	Thread::AtShutdown(StopMcpServer);
	return sMcpServer.Start();
}
void StopMcpServer() { sMcpServer.Stop(); }
bool McpIsRunning() { return sMcpServer.IsRunning(); }

END_UPP_NAMESPACE
 
bool McpServer::ReadFramed(McpClient& c, Vector<String>& out_msgs) {
    c.sock->Timeout(0);
    if(!c.sock->Peek()) return false;
    String chunk = c.sock->GetLine();
    if(c.sock->IsError()) { c.sock->Close(); return false; }
    if(chunk.IsEmpty()) return false;
    c.inbuf.Cat(chunk);
    c.last_activity = GetSysTime();
    if(c.inbuf.GetLength() > max_message_bytes) { c.sock->Close(); return false; }
    for(;;) {
        int p = c.inbuf.Find('\n');
        if(p < 0) break;
        String line = c.inbuf.Mid(0, p);
        c.inbuf.Remove(0, p+1);
        if(line.GetCount()) out_msgs.Add(line);
    }
    return out_msgs.GetCount();
}

bool McpServer::WritePending(McpClient& c) {
    if(c.outbuf.IsEmpty()) return false;
    int n = c.sock->Put(c.outbuf);
    if(c.sock->IsError()) { c.sock->Close(); return false; }
    if(n > 0) {
        c.outbuf.Remove(0, n);
        c.last_activity = GetSysTime();
        return true;
    }
    return false;
}
