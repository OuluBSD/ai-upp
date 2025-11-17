#ifndef _QwenManager_QwenConnection_h_
#define _QwenManager_QwenConnection_h_


class QwenConnection {
private:
    // Reference to the server configuration
    QwenServerConnectionConf* config = nullptr;

    // The actual Qwen client
    std::unique_ptr<Qwen::QwenClient> client;

    // Client configuration
    Qwen::QwenClientConfig client_config;

    // Connection status
    bool connected = false;
    bool healthy = false;

    // Session management
    String current_session_id;

public:
    QwenConnection();
    ~QwenConnection();

    void Init(QwenServerConnectionConf& srv_config);
    bool Connect();
    void Disconnect();
    bool IsConnected() const { return connected; }
    bool IsHealthy() const { return healthy; }

    // Send user input to the Qwen server
    bool SendUserInput(const String& content);

    // Get connection status
    String GetStatus() const;

    // Poll for incoming messages
    int PollMessages(int timeout_ms = 0);

    // Session management
    bool CreateSession();
    bool AttachToSession(const String& sessionId);
    bool ListSessions(Vector<String>& sessionList);
    bool DeleteSession(const String& sessionId);
    String GetCurrentSessionId() const;
};


#endif