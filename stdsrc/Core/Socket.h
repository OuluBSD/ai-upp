#pragma once
#ifndef _Core_Socket_h_
#define _Core_Socket_h_

#include <string>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
#endif

NAMESPACE_UPP

// Network utility functions
bool InitializeNetwork();
void CleanupNetwork();

// Base Socket class
class Socket {
protected:
    int sock;
    bool connected;

public:
    Socket();
    virtual ~Socket();
    
    // Create socket
    bool Create(int domain, int type, int protocol);
    
    // Bind to address and port
    bool Bind(const std::string& address, int port);
    
    // Connect to remote address and port
    bool Connect(const std::string& address, int port);
    
    // Send data
    int Send(const void* data, size_t size);
    
    // Receive data
    int Receive(void* buffer, size_t size);
    
    // Close the socket
    void Close();
    
    // Set blocking mode
    bool SetBlocking(bool blocking);
    
    // Check if socket is valid
    bool IsValid() const { return sock != -1; }
    
    // Check if connected
    bool IsConnected() const { return connected; }
};

// TCP socket
class TcpSocket : public Socket {
public:
    TcpSocket();
    virtual ~TcpSocket();
};

// UDP socket
class UdpSocket : public Socket {
public:
    UdpSocket();
    virtual ~UdpSocket();
};

END_UPP_NAMESPACE

#endif