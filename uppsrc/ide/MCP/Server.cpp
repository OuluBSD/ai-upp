#include <ide/ide.h>

NAMESPACE_UPP

McpServer sMcpServer;

bool McpServer::Start(int port) {
    if(running)
        return true;
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
    listener.NoDelay();
    if(!listener.Listen(listen_port, 128, true)) {
        RLOG("MCP: listen failed on port " << listen_port << ": " << listener.GetErrorDesc());
        return;
    }
    RLOG("MCP: listening on port " << listen_port);

	clients_thr.Start(THISBACK(HandleClients));
	
    while(!stop) {
        // Accept new clients non-blocking
        listener.Timeout(5);
        One<TcpSocket> s;
        s.Create();
        if(s->Accept(listener)) {
            s->NoDelay();
            clients_lock.EnterWrite();
            clients.Add(s.Detach());
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
            TcpSocket& s = clients[i];
            clients_lock.LeaveRead();
            
            if(!s.IsOpen()) { clients_lock.EnterWrite(); clients.Remove(i--); clients_lock.LeaveWrite(); continue; }
            while(s.Peek() && s.IsOpen()) {
                String line = s.GetLine();
                if(IsNull(line)) break;
                McpRequest req;
                String reply;
                if(ParseRequest(line, req))
                    reply = Handle(req);
                else
                    reply = MakeError("", -32700, "Parse error");
                if(!IsNull(reply)) {
                    reply.Cat('\n');
                    s.Put(reply);
                }
            }
            if(s.IsError() || s.IsEof()) { clients_lock.EnterWrite(); clients.Remove(i--); clients_lock.LeaveWrite(); continue; }
        }
        Sleep(5);
    }
    // Cleanup
    for(auto& c : clients) if(c.IsOpen()) c.Close();
    
    clients_lock.EnterWrite();
    clients.Clear();
    clients_lock.LeaveWrite();
}

String McpServer::Handle(const McpRequest& req) {
    if(req.method == "mcp.ping") {
        ValueMap r; r.Add("text", "pong");
        return MakeResult(req.id, r);
    }
    if(req.method == "workspace.info") {
        ValueMap r;
        r.Add("name", GetCurrentWorkspaceName());
        r.Add("packages", GetCurrentWorkspacePackageCount());
        return MakeResult(req.id, r);
    }
    return MakeError(req.id, -32601, "Method not found");
}

bool StartMcpServer(const McpConfig&) { return sMcpServer.Start(); }
void StopMcpServer() { sMcpServer.Stop(); }
bool McpIsRunning() { return sMcpServer.IsRunning(); }

END_UPP_NAMESPACE

