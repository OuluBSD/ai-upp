#ifndef _ide_MCP_Server_h_
#define _ide_MCP_Server_h_


class McpServer {
public:
    bool Start();
    void Stop();
    bool IsRunning() const { return running; }

private:
    void Loop();
    String Handle(const McpRequest& req);

    Thread  thr;
    Atomic  stop {0};
    bool    running = false;
};

extern McpServer sMcpServer;

#endif
