#include "Socket.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    #include <fcntl.h>
#endif

NAMESPACE_UPP

// Initialize WSA if on Windows
static bool WSAInitialized = false;

bool InitializeNetwork() {
#ifdef _WIN32
    if (!WSAInitialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            return false;
        }
        WSAInitialized = true;
    }
#endif
    return true;
}

void CleanupNetwork() {
#ifdef _WIN32
    if (WSAInitialized) {
        WSACleanup();
        WSAInitialized = false;
    }
#endif
}

// Socket implementation
Socket::Socket() : sock(-1), connected(false) {
    InitializeNetwork();
}

Socket::~Socket() {
    Close();
}

bool Socket::Create(int domain, int type, int protocol) {
#ifdef _WIN32
    sock = ::socket(domain, type, protocol);
#else
    sock = ::socket(domain, type, protocol);
#endif
    return sock != -1;
}

bool Socket::Bind(const std::string& address, int port) {
    if (sock == -1) return false;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    return ::bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0;
}

bool Socket::Connect(const std::string& address, int port) {
    if (sock == -1) return false;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        connected = true;
        return true;
    }
    return false;
}

int Socket::Send(const void* data, size_t size) {
    if (!connected) return -1;
    return send(sock, static_cast<const char*>(data), size, 0);
}

int Socket::Receive(void* buffer, size_t size) {
    if (!connected) return -1;
    return recv(sock, static_cast<char*>(buffer), size, 0);
}

void Socket::Close() {
    if (sock != -1) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        sock = -1;
        connected = false;
    }
}

bool Socket::SetBlocking(bool blocking) {
#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(sock, F_SETFL, flags) == 0;
#endif
}

// TcpSocket implementation
TcpSocket::TcpSocket() : Socket() {
    Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

TcpSocket::~TcpSocket() {
    Close();
}

// UdpSocket implementation
UdpSocket::UdpSocket() : Socket() {
    Create(AF_INET, SOCK_DGRAM, 0);
}

UdpSocket::~UdpSocket() {
    Close();
}

END_UPP_NAMESPACE