#ifndef _ide_MCP_Server_h_
#define _ide_MCP_Server_h_


class McpServer {
public:
	typedef McpServer CLASSNAME;
    bool Start(int port = 0); // 0 -> choose default
    void Stop();
    bool IsRunning() const { return running; }
    int  Port() const { return listen_port; }

private:
    void Loop();
    String Handle(const McpRequest& req);
    void HandleClients();

    Thread  thr;
    Thread  clients_thr;
    Atomic  stop {0};
    bool    running = false;
    int     listen_port = 0;
    RWMutex clients_lock;
    Array<TcpSocket> clients;
};

extern McpServer sMcpServer;

#endif
