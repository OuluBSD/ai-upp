#pragma once
#ifndef _Core_Inet_h_
#define _Core_Inet_h_

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Core.h"
#include "Socket.h"

// Internet protocol constants
const int TCP_PROTOCOL = 6;
const int UDP_PROTOCOL = 17;

// Common port numbers
const int HTTP_PORT = 80;
const int HTTPS_PORT = 443;
const int FTP_PORT = 21;
const int SMTP_PORT = 25;
const int POP3_PORT = 110;
const int IMAP_PORT = 143;

// IP address representation
class IpAddress {
public:
    IpAddress();
    explicit IpAddress(const std::string& addr);
    IpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
    
    std::string ToString() const;
    uint32_t ToInt() const;
    bool IsValid() const { return valid; }
    
private:
    union {
        uint8_t bytes[4];
        uint32_t addr;
    };
    bool valid;
};

// URL parsing utilities
class Url {
public:
    Url();
    explicit Url(const std::string& url);
    
    std::string GetProtocol() const { return protocol; }
    std::string GetHost() const { return host; }
    int GetPort() const { return port; }
    std::string GetPath() const { return path; }
    std::string GetQuery() const { return query; }
    std::string GetFragment() const { return fragment; }
    
    bool IsValid() const { return valid; }
    
private:
    std::string protocol;
    std::string host;
    int port;
    std::string path;
    std::string query;
    std::string fragment;
    bool valid;
};

// HTTP client utilities
class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    // Make HTTP request
    bool Get(const std::string& url, std::string& response);
    bool Post(const std::string& url, const std::string& data, std::string& response);
    
    // Response headers
    const std::map<std::string, std::string>& GetHeaders() const { return headers; }
    
    // Set request headers
    void SetHeader(const std::string& name, const std::string& value);
    
private:
    std::map<std::string, std::string> headers;
    std::unique_ptr<TcpSocket> socket;
};

// DNS resolution utilities
std::vector<IpAddress> DnsLookup(const std::string& hostname);

#endif