#pragma once
#ifndef _Core_NetNode_h_
#define _Core_NetNode_h_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>
#include <atomic>
#include "Core.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

// NetNode - network node implementation for stdsrc
// Provides a flexible networking framework for distributed applications

class NetAddress {
private:
    std::string ip;
    int port;
    
public:
    NetAddress() : port(0) {}
    NetAddress(const std::string& ip_address, int port_number) 
        : ip(ip_address), port(port_number) {}
    
    NetAddress(const std::string& address_port) {
        size_t colon_pos = address_port.rfind(':');
        if (colon_pos != std::string::npos) {
            ip = address_port.substr(0, colon_pos);
            port = std::stoi(address_port.substr(colon_pos + 1));
        } else {
            ip = address_port;
            port = 0;
        }
    }
    
    const std::string& GetIP() const { return ip; }
    int GetPort() const { return port; }
    
    std::string ToString() const {
        return ip + ":" + std::to_string(port);
    }
    
    bool IsValid() const {
        return !ip.empty() && port > 0 && port <= 65535;
    }
    
    bool operator==(const NetAddress& other) const {
        return ip == other.ip && port == other.port;
    }
    
    bool operator!=(const NetAddress& other) const {
        return !(*this == other);
    }
    
    bool operator<(const NetAddress& other) const {
        if (ip != other.ip) return ip < other.ip;
        return port < other.port;
    }
};

// Message structure
struct NetMessage {
    NetAddress source;
    NetAddress destination;
    std::string type;
    std::string payload;
    std::chrono::steady_clock::time_point timestamp;
    int ttl;
    
    NetMessage() : timestamp(std::chrono::steady_clock::now()), ttl(64) {}
    
    NetMessage(const NetAddress& src, const NetAddress& dest, const std::string& msg_type, const std::string& data)
        : source(src), destination(dest), type(msg_type), payload(data), 
          timestamp(std::chrono::steady_clock::now()), ttl(64) {}
    
    std::string ToString() const {
        return "NetMessage{" + type + " from " + source.ToString() + " to " + destination.ToString() + 
               ": " + std::to_string(payload.size()) + " bytes}";
    }
};

// Network node base class
class NetNode {
public:
    enum class NodeType {
        CLIENT,
        SERVER,
        PEER,
        ROUTER
    };
    
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    };
    
private:
    std::string node_id;
    NetAddress local_address;
    NodeType node_type;
    std::atomic<ConnectionState> connection_state;
    
    // Threading
    std::thread network_thread;
    std::atomic<bool> running;
    std::mutex network_mutex;
    
    // Connections
    std::map<NetAddress, std::shared_ptr<class Connection>> connections;
    std::mutex connections_mutex;
    
    // Message queues
    std::queue<NetMessage> incoming_queue;
    std::queue<NetMessage> outgoing_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    // Callbacks
    std::function<void(const NetMessage&)> message_received_callback;
    std::function<void(const NetAddress&)> peer_connected_callback;
    std::function<void(const NetAddress&)> peer_disconnected_callback;
    
    // Routing table (for router nodes)
    std::map<NetAddress, NetAddress> routing_table;
    std::mutex routing_mutex;
    
    // Statistics
    std::atomic<uint64_t> messages_sent;
    std::atomic<uint64_t> messages_received;
    std::atomic<uint64_t> bytes_sent;
    std::atomic<uint64_t> bytes_received;
    
    void NetworkThreadFunc();
    void ProcessIncomingMessage(const NetMessage& message);
    void RouteMessage(const NetMessage& message);
    bool SendMessage(const NetMessage& message);
    
protected:
    virtual void OnMessageReceived(const NetMessage& message) {}
    virtual void OnPeerConnected(const NetAddress& peer) {}
    virtual void OnPeerDisconnected(const NetAddress& peer) {}
    virtual void OnConnectionStateChanged(ConnectionState old_state, ConnectionState new_state) {}
    
public:
    // Constructors
    NetNode(const std::string& id, NodeType type = NodeType::CLIENT);
    virtual ~NetNode();
    
    // Initialization
    bool Initialize(const NetAddress& address);
    void Shutdown();
    
    // Connection management
    bool ConnectToPeer(const NetAddress& peer_address);
    void DisconnectFromPeer(const NetAddress& peer_address);
    void DisconnectAllPeers();
    
    // Message sending
    bool Send(const NetAddress& destination, const std::string& message_type, const std::string& payload);
    bool Broadcast(const std::string& message_type, const std::string& payload);
    
    // Message receiving (blocking)
    bool Receive(NetMessage& message, int timeout_ms = -1);
    
    // Asynchronous message handling
    void SetMessageReceivedCallback(std::function<void(const NetMessage&)> callback);
    void SetPeerConnectedCallback(std::function<void(const NetAddress&)> callback);
    void SetPeerDisconnectedCallback(std::function<void(const NetAddress&)> callback);
    
    // Node information
    const std::string& GetNodeId() const { return node_id; }
    const NetAddress& GetLocalAddress() const { return local_address; }
    NodeType GetNodeType() const { return node_type; }
    ConnectionState GetConnectionState() const { return connection_state.load(); }
    
    std::vector<NetAddress> GetConnectedPeers() const;
    bool IsPeerConnected(const NetAddress& peer) const;
    
    // Routing (for router nodes)
    void AddRoute(const NetAddress& destination, const NetAddress& next_hop);
    void RemoveRoute(const NetAddress& destination);
    NetAddress FindRoute(const NetAddress& destination) const;
    
    // Statistics
    uint64_t GetMessagesSent() const { return messages_sent.load(); }
    uint64_t GetMessagesReceived() const { return messages_received.load(); }
    uint64_t GetBytesSent() const { return bytes_sent.load(); }
    uint64_t GetBytesReceived() const { return bytes_received.load(); }
    
    void ResetStatistics();
    
    // Utility functions
    static NetAddress ResolveHostname(const std::string& hostname, int port);
    static std::string GetLocalIPAddress();
    
    // Serialization support
    template<typename Stream>
    void Serialize(Stream& s) {
        s % node_id % local_address % static_cast<int&>(node_type);
    }
    
    // String representation
    std::string ToString() const;
};

// Connection class
class Connection {
private:
    NetAddress remote_address;
    std::atomic<NetNode::ConnectionState> state;
    
#ifdef _WIN32
    SOCKET socket_fd;
#else
    int socket_fd;
#endif
    
    std::thread io_thread;
    std::atomic<bool> running;
    std::mutex io_mutex;
    
    NetNode* parent_node;
    
    void IOThreadFunc();
    void ProcessIncomingData();
    
public:
    Connection(NetNode* parent, const NetAddress& address);
    ~Connection();
    
    bool Connect();
    void Disconnect();
    
    bool SendData(const std::string& data);
    std::string ReceiveData();
    
    const NetAddress& GetRemoteAddress() const { return remote_address; }
    NetNode::ConnectionState GetState() const { return state.load(); }
    
    bool IsConnected() const { 
        return state.load() == NetNode::ConnectionState::CONNECTED; 
    }
};

// Network service discovery
class NetDiscovery {
private:
    NetNode discovery_node;
    std::map<std::string, NetAddress> services;
    std::mutex services_mutex;
    
public:
    NetDiscovery();
    ~NetDiscovery();
    
    bool Start(const NetAddress& listen_address);
    void Stop();
    
    void RegisterService(const std::string& service_name, const NetAddress& service_address);
    void UnregisterService(const std::string& service_name);
    NetAddress DiscoverService(const std::string& service_name);
    std::vector<std::string> ListServices();
    
    // Callback for service discovery notifications
    std::function<void(const std::string&, const NetAddress&)> service_registered_callback;
    std::function<void(const std::string&)> service_unregistered_callback;
};

// Network message router
class NetRouter : public NetNode {
private:
    std::map<NetAddress, std::shared_ptr<Connection>> routes;
    
protected:
    void OnMessageReceived(const NetMessage& message) override;
    void OnPeerConnected(const NetAddress& peer) override;
    void OnPeerDisconnected(const NetAddress& peer) override;
    
public:
    NetRouter(const std::string& id);
    ~NetRouter();
    
    bool AddRoute(const NetAddress& destination, const NetAddress& next_hop);
    void RemoveRoute(const NetAddress& destination);
    std::vector<NetAddress> GetRoutes() const;
};

// Streaming operators
template<typename Stream>
void operator%(Stream& s, NetAddress& address) {
    s % const_cast<std::string&>(address.GetIP()) % const_cast<int&>(address.GetPort());
}

template<typename Stream>
void operator%(Stream& s, NetMessage& message) {
    s % message.source % message.destination % message.type % message.payload % message.ttl;
}

template<typename Stream>
void operator%(Stream& s, NetNode& node) {
    node.Serialize(s);
}

// String conversion
inline std::string AsString(const NetAddress& address) {
    return address.ToString();
}

inline std::string AsString(const NetMessage& message) {
    return message.ToString();
}

inline std::string AsString(const NetNode& node) {
    return node.ToString();
}

// Implementation details
inline NetNode::NetNode(const std::string& id, NodeType type) 
    : node_id(id), node_type(type), connection_state(ConnectionState::DISCONNECTED),
      running(false), messages_sent(0), messages_received(0), 
      bytes_sent(0), bytes_received(0) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

inline NetNode::~NetNode() {
    Shutdown();
#ifdef _WIN32
    WSACleanup();
#endif
}

inline void NetNode::NetworkThreadFunc() {
    while (running.load()) {
        // Process queued messages
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            while (!incoming_queue.empty()) {
                NetMessage message = incoming_queue.front();
                incoming_queue.pop();
                ProcessIncomingMessage(message);
            }
        }
        
        // Check for new connections and data
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

inline void NetNode::ProcessIncomingMessage(const NetMessage& message) {
    messages_received.fetch_add(1);
    bytes_received.fetch_add(message.payload.size());
    
    // Route if needed
    if (node_type == NodeType::ROUTER) {
        RouteMessage(message);
    } else {
        // Deliver to application
        if (message_received_callback) {
            message_received_callback(message);
        }
        OnMessageReceived(message);
    }
}

inline void NetNode::RouteMessage(const NetMessage& message) {
    // For routers, forward messages appropriately
    if (message.destination == local_address) {
        // Message is for us
        if (message_received_callback) {
            message_received_callback(message);
        }
        OnMessageReceived(message);
    } else {
        // Forward to appropriate peer
        NetAddress next_hop = FindRoute(message.destination);
        if (next_hop.IsValid()) {
            auto it = connections.find(next_hop);
            if (it != connections.end()) {
                it->second->SendData(message.ToString());
            }
        }
    }
}

inline bool NetNode::SendMessage(const NetMessage& message) {
    messages_sent.fetch_add(1);
    bytes_sent.fetch_add(message.payload.size());
    
    // Queue for sending
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        outgoing_queue.push(message);
    }
    queue_cv.notify_one();
    
    return true;
}

inline std::string NetNode::ToString() const {
    return "NetNode{id=" + node_id + ", address=" + local_address.ToString() + 
           ", type=" + std::to_string(static_cast<int>(node_type)) + 
           ", state=" + std::to_string(static_cast<int>(connection_state.load())) + "}";
}

inline NetDiscovery::NetDiscovery() : discovery_node("discovery", NetNode::NodeType::SERVER) {}

inline NetDiscovery::~NetDiscovery() {
    Stop();
}

inline Connection::Connection(NetNode* parent, const NetAddress& address) 
    : remote_address(address), state(NetNode::ConnectionState::DISCONNECTED),
      socket_fd(-1), running(false), parent_node(parent) {}

inline Connection::~Connection() {
    Disconnect();
}

#endif