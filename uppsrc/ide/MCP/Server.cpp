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
