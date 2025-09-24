#ifndef _ide_MCP_Server_h_
#define _ide_MCP_Server_h_


struct McpClient {
    One<TcpSocket> sock;
    String         inbuf;
    String         outbuf;
    Time           last_activity;
    int            id = 0;
};

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
    bool ReadFramed(McpClient& c, Vector<String>& out_msgs);
    bool WritePending(McpClient& c);

    Thread  thr;
    Thread  clients_thr;
    Atomic  stop {0};
    bool    running = false;
    int     listen_port = 0;
    RWMutex clients_lock;
    Array<McpClient> clients;
    int next_client_id = 1;
    int max_message_bytes = 4 * 1024 * 1024; // 4 MB cap
};

extern McpServer sMcpServer;

#endif
