#pragma once
#ifndef _Core_WebSocket_h_
#define _Core_WebSocket_h_

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include "Core.h"

// WebSocket implementation for stdsrc

class WebSocketException : public std::exception {
private:
    std::string message;
    
public:
    WebSocketException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class WebSocket {
public:
    enum Opcode {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA
    };
    
    enum State {
        CONNECTING,
        OPEN,
        CLOSING,
        CLOSED
    };
    
    // Callback types
    using OpenCallback = std::function<void()>;
    using MessageCallback = std::function<void(const std::string&, bool is_binary)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    using CloseCallback = std::function<void(int, const std::string&)>;
    
private:
    // Connection state
    State state = CLOSED;
    std::string url;
    std::string host;
    std::string path;
    int port = 80;
    bool secure = false;
    
    // Networking
    std::unique_ptr<class TcpSocket> socket;
    
    // Handshake
    std::string key;
    std::string origin;
    std::vector<std::string> protocols;
    std::string selected_protocol;
    
    // Message handling
    std::queue<std::pair<std::string, bool>> message_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    // Threading
    std::thread io_thread;
    std::atomic<bool> running{false};
    std::mutex state_mutex;
    
    // Callbacks
    OpenCallback on_open;
    MessageCallback on_message;
    ErrorCallback on_error;
    CloseCallback on_close;
    
    // Buffering
    std::vector<uint8_t> receive_buffer;
    std::vector<uint8_t> send_buffer;
    
    // WebSocket frame state
    bool fragmented = false;
    Opcode fragment_opcode = CONTINUATION;
    std::vector<uint8_t> fragment_buffer;
    
    // Utility functions
    static std::string GenerateKey();
    static std::string Base64Encode(const std::string& data);
    static std::string Base64Decode(const std::string& data);
    static std::string Sha1Hash(const std::string& data);
    
    // Handshake functions
    bool PerformHandshake();
    std::string CreateHandshakeRequest();
    bool ParseHandshakeResponse(const std::string& response);
    
    // Frame handling
    struct FrameHeader {
        bool fin;
        bool rsv1, rsv2, rsv3;
        Opcode opcode;
        bool masked;
        uint64_t payload_length;
        uint32_t masking_key;
    };
    
    bool ParseFrameHeader(FrameHeader& header, const std::vector<uint8_t>& buffer, size_t& offset);
    std::vector<uint8_t> CreateFrame(Opcode opcode, const std::vector<uint8_t>& payload, bool fin = true, bool mask = false);
    void ApplyMasking(std::vector<uint8_t>& data, uint32_t masking_key);
    
    // I/O functions
    void IoThread();
    bool SendFrame(Opcode opcode, const std::vector<uint8_t>& payload, bool fin = true);
    bool ReceiveFrame();
    
    // Event handling
    void HandleOpen();
    void HandleMessage(const std::vector<uint8_t>& payload, Opcode opcode, bool fin);
    void HandleError(const std::string& error);
    void HandleClose(int code, const std::string& reason);
    
    // Utility functions
    std::vector<uint8_t> StringToBytes(const std::string& str);
    std::string BytesToString(const std::vector<uint8_t>& bytes);
    
public:
    // Constructors and destructor
    WebSocket();
    ~WebSocket();
    
    // Connection management
    bool Connect(const std::string& url);
    bool Connect(const std::string& host, int port, const std::string& path, bool secure = false);
    void Close(int code = 1000, const std::string& reason = "");
    void Disconnect();
    
    // Message sending
    bool Send(const std::string& message);
    bool SendBinary(const std::vector<uint8_t>& data);
    bool SendPing(const std::string& data = "");
    bool SendPong(const std::string& data = "");
    
    // Fragmented message sending
    bool BeginTextMessage();
    bool BeginBinaryMessage();
    bool SendFragment(const std::string& fragment, bool is_final = false);
    bool SendBinaryFragment(const std::vector<uint8_t>& fragment, bool is_final = false);
    bool EndMessage();
    
    // State queries
    State GetState() const;
    bool IsOpen() const;
    bool IsConnecting() const;
    bool IsClosing() const;
    bool IsClosed() const;
    
    // Configuration
    void SetOrigin(const std::string& origin);
    void AddProtocol(const std::string& protocol);
    void ClearProtocols();
    std::string GetSelectedProtocol() const;
    
    // Callback registration
    void OnOpen(OpenCallback callback);
    void OnMessage(MessageCallback callback);
    void OnError(ErrorCallback callback);
    void OnClose(CloseCallback callback);
    
    // Event loop (blocking)
    void Run();
    void Stop();
    
    // Non-blocking message polling
    bool PollMessage(std::string& message, bool& is_binary);
    bool WaitForMessage(std::string& message, bool& is_binary, int timeout_ms = -1);
    
    // Utility functions
    static bool IsValidUrl(const std::string& url);
    static std::string ParseHost(const std::string& url);
    static int ParsePort(const std::string& url);
    static std::string ParsePath(const std::string& url);
    static bool IsSecure(const std::string& url);
    
    // String representation
    std::string ToString() const;
};

// WebSocket server-side connection
class WebSocketServer {
public:
    struct Connection {
        std::weak_ptr<WebSocket> ws;
        std::string remote_address;
        int remote_port;
        
        bool IsOpen() const { 
            auto ws_ptr = ws.lock();
            return ws_ptr && ws_ptr->IsOpen();
        }
        
        void Close(int code = 1000, const std::string& reason = "") {
            auto ws_ptr = ws.lock();
            if (ws_ptr) {
                ws_ptr->Close(code, reason);
            }
        }
        
        bool Send(const std::string& message) {
            auto ws_ptr = ws.lock();
            return ws_ptr && ws_ptr->Send(message);
        }
        
        bool SendBinary(const std::vector<uint8_t>& data) {
            auto ws_ptr = ws.lock();
            return ws_ptr && ws_ptr->SendBinary(data);
        }
    };
    
private:
    std::unique_ptr<class TcpServer> server;
    std::vector<std::shared_ptr<WebSocket>> connections;
    std::mutex connections_mutex;
    
    // Callbacks
    std::function<void(Connection&)> on_connect;
    std::function<void(Connection&, const std::string&, bool)> on_message;
    std::function<void(Connection&, int, const std::string&)> on_disconnect;
    
    // Server configuration
    std::string origin;
    std::vector<std::string> protocols;
    
    // Threading
    std::thread accept_thread;
    std::atomic<bool> accepting{false};
    
    // Utility functions
    void AcceptThread();
    std::shared_ptr<WebSocket> CreateWebSocket(class TcpSocket* socket);
    
public:
    WebSocketServer();
    ~WebSocketServer();
    
    // Server management
    bool Start(int port, const std::string& address = "0.0.0.0");
    void Stop();
    
    // Connection management
    std::vector<Connection> GetConnections() const;
    size_t GetConnectionCount() const;
    void CloseAllConnections(int code = 1000, const std::string& reason = "");
    
    // Broadcasting
    void Broadcast(const std::string& message);
    void BroadcastBinary(const std::vector<uint8_t>& data);
    
    // Callback registration
    void OnConnect(std::function<void(Connection&)> callback);
    void OnMessage(std::function<void(Connection&, const std::string&, bool)> callback);
    void OnDisconnect(std::function<void(Connection&, int, const std::string&)> callback);
    
    // Configuration
    void SetOrigin(const std::string& origin);
    void AddProtocol(const std::string& protocol);
    void ClearProtocols();
    
    // Utility functions
    bool IsRunning() const;
    int GetPort() const;
    
    // String representation
    std::string ToString() const;
};

// WebSocket utility functions
namespace WebSocketUtils {
    std::string GenerateRandomKey();
    std::string Base64Encode(const std::string& data);
    std::string Base64Decode(const std::string& data);
    std::string Sha1Hash(const std::string& data);
    
    bool ParseUrl(const std::string& url, std::string& host, int& port, std::string& path, bool& secure);
    std::string CreateUrl(const std::string& host, int port, const std::string& path, bool secure);
    
    std::vector<uint8_t> MaskData(const std::vector<uint8_t>& data, uint32_t masking_key);
    uint32_t GenerateMaskingKey();
    
    // Status codes
    enum StatusCode {
        NORMAL_CLOSURE = 1000,
        GOING_AWAY = 1001,
        PROTOCOL_ERROR = 1002,
        UNSUPPORTED_DATA = 1003,
        RESERVED_1004 = 1004,
        NO_STATUS_RECEIVED = 1005,
        ABNORMAL_CLOSURE = 1006,
        INVALID_FRAME_PAYLOAD_DATA = 1007,
        POLICY_VIOLATION = 1008,
        MESSAGE_TOO_BIG = 1009,
        MISSING_EXTENSION = 1010,
        INTERNAL_SERVER_ERROR = 1011,
        TLS_HANDSHAKE_FAILED = 1015
    };
    
    std::string GetStatusCodeDescription(int code);
}

// Global functions
inline bool WebSocketConnect(WebSocket& ws, const std::string& url) {
    return ws.Connect(url);
}

inline void WebSocketClose(WebSocket& ws, int code = 1000, const std::string& reason = "") {
    ws.Close(code, reason);
}

inline bool WebSocketSend(WebSocket& ws, const std::string& message) {
    return ws.Send(message);
}

inline bool WebSocketSendBinary(WebSocket& ws, const std::vector<uint8_t>& data) {
    return ws.SendBinary(data);
}

inline bool WebSocketIsOpen(const WebSocket& ws) {
    return ws.IsOpen();
}

inline WebSocket::State WebSocketGetState(const WebSocket& ws) {
    return ws.GetState();
}

// Streaming operator
template<typename Stream>
void operator%(Stream& s, WebSocket& ws) {
    // WebSocket cannot be serialized directly
    // This is just a placeholder to satisfy interface requirements
    s.LoadError();
}

// String conversion
inline std::string AsString(const WebSocket& ws) {
    return ws.ToString();
}

inline std::string AsString(const WebSocketServer& server) {
    return server.ToString();
}

#endif