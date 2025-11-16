#ifndef _QwenTCPServer_h_
#define _QwenTCPServer_h_

#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <map>
#include <memory>
#include <chrono>

// Forward declaration for QwenClient
namespace Qwen {
    class QwenClient;
    struct QwenClientConfig;
}

namespace Qwen {

// Structure to track client connection context
struct ClientContext {
    int client_fd;
    std::chrono::steady_clock::time_point last_activity;
    std::string input_buffer;  // For accumulating partial messages
};

class QwenTCPServer {
public:
    explicit QwenTCPServer(const std::string& qwen_code_path = "");
    ~QwenTCPServer();

    // Start the TCP server on the specified port and host
    bool start(int port, const std::string& host = "0.0.0.0");
    
    // Stop the TCP server
    void stop();
    
    // Check if the server is currently running
    bool is_running() const;

private:
    // Main server thread function
    void server_thread();
    
    // Handle individual client messages
    void handle_client_message(int client_fd, const std::string& message);
    
    // Send response back to client
    bool send_response(int client_fd, const std::string& response);

    // Qwen client for communicating with qwen-code
    std::unique_ptr<QwenClient> qwen_client_;
    
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    
    int server_socket_ = -1;
    
    // Track active client connections
    std::map<int, ClientContext> active_clients_;
    mutable std::mutex clients_mutex_;
    
    int port_;
    std::string host_;
    std::string qwen_code_path_;
};

} // namespace Qwen

#endif // _QwenTCPServer_h_