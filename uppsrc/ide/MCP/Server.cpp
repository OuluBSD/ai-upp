#include "MCP.h"

NAMESPACE_UPP

McpServer sMcpServer;

bool McpServer::Start() {
    if(running)
        return true;
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
    running = false;
}

void McpServer::Loop() {
    StdLogSetup(LOG_COUT|LOG_FILE);
    while(!stop) {
        String line;
        
        // TODO read line from tcp server
        
        if(IsNull(line)) {
            Sleep(10);
            continue;
        }
        McpRequest req;
        String reply;
        if(ParseRequest(line, req))
            reply = Handle(req);
        else
            reply = MakeError("", -32700, "Parse error");
        if(!IsNull(reply))
            Cout() << reply << '\n';
    }
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

