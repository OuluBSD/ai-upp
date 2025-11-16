#include "QwenTCPServer.h"
#include "QwenClient.h"
#include "QwenProtocol.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <thread>
#include <chrono>
#include <arpa/inet.h>  // for inet_pton
#include <sys/poll.h>
#include <functional>
#include <vector>
#include <map>
#include <optional>
#include <cctype>
#include <iomanip>

namespace Qwen {

namespace {

std::string json_escape(const std::string& str) {
    std::ostringstream oss;
    for (unsigned char c : str) {
        switch (c) {
            case '\\': oss << "\\\\"; break;
            case '"': oss << "\\\""; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 32) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    oss << std::dec;
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string serialize_tool_group_message(const ToolGroup& group) {
    std::ostringstream oss;
    oss << "{\"type\":\"tool_group\",\"id\":" << group.id << ",\"tools\":[";

    for (size_t i = 0; i < group.tools.size(); ++i) {
        const auto& tool = group.tools[i];
        if (i > 0) {
            oss << ",";
        }

        oss << "{";
        oss << "\"type\":\"tool_call\",";
        oss << "\"tool_id\":\"" << json_escape(tool.tool_id) << "\",";
        oss << "\"tool_name\":\"" << json_escape(tool.tool_name) << "\",";
        oss << "\"status\":\"" << tool_status_to_string(tool.status) << "\",";
        oss << "\"args\":{";

        bool first_arg = true;
        for (const auto& [key, value] : tool.args) {
            if (!first_arg) {
                oss << ",";
            }
            first_arg = false;
            oss << "\"" << json_escape(key) << "\":\"" << json_escape(value) << "\"";
        }

        oss << "}";

        if (tool.result) {
            oss << ",\"result\":\"" << json_escape(*tool.result) << "\"";
        }
        if (tool.error) {
            oss << ",\"error\":\"" << json_escape(*tool.error) << "\"";
        }
        if (tool.confirmation_details) {
            oss << ",\"confirmation_details\":{";
            oss << "\"message\":\"" << json_escape(tool.confirmation_details->message) << "\",";
            oss << "\"requires_approval\":" << (tool.confirmation_details->requires_approval ? "true" : "false");
            oss << "}";
        }

        oss << "}";
    }

    oss << "]}\n";
    return oss.str();
}

std::optional<std::string> extract_json_string_field(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    pos += search.length();
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }

    if (pos >= json.size() || json[pos] != '"') {
        return std::nullopt;
    }

    ++pos; // skip opening quote
    std::string value;
    bool escaping = false;

    for (; pos < json.size(); ++pos) {
        char c = json[pos];
        if (escaping) {
            switch (c) {
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default: value += c; break;
            }
            escaping = false;
            continue;
        }

        if (c == '\\') {
            escaping = true;
            continue;
        }

        if (c == '"') {
            return value;
        }

        value += c;
    }

    return std::nullopt;
}

std::optional<bool> extract_json_bool_field(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\":";
    size_t pos = json.find(search);
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    pos += search.length();
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
        ++pos;
    }

    if (json.compare(pos, 4, "true") == 0) {
        return true;
    }
    if (json.compare(pos, 5, "false") == 0) {
        return false;
    }

    return std::nullopt;
}

}  // namespace


QwenTCPServer::QwenTCPServer(const std::string& qwen_code_path)
    : qwen_code_path_(qwen_code_path)
{
}

QwenTCPServer::~QwenTCPServer() {
    stop();
}

bool QwenTCPServer::start(int port, const std::string& host) {
    if (running_.load()) {
        return false; // Already running
    }

    port_ = port;
    host_ = host;
    
    // Initialize QwenClient to connect to qwen-code
    QwenClientConfig client_config;
    
    // Set the path to qwen-code executable
    // Default to project's wrapper script (relative to project root)
    std::string default_path = qwen_code_path_.empty() ? "script/qwen-code" : qwen_code_path_;

    client_config.qwen_executable = default_path;
    
    // Configure client to communicate via stdin/stdout (the default mode for qwen-code)
    client_config.mode = CommunicationMode::STDIN_STDOUT;
    client_config.auto_restart = true;
    client_config.verbose = true;  // Enable verbose logging to debug message flow
    
    // Initialize the Qwen client
    qwen_client_ = std::make_unique<QwenClient>(client_config);
    
    // Set up message handlers to handle responses from qwen-code and forward to TCP clients
    MessageHandlers handlers;
    
    // When we get a conversation response from qwen-code, forward it to the appropriate TCP client
    handlers.on_conversation = [this](const ConversationMessage& msg) {
        if (msg.role == MessageRole::ASSISTANT) {
            // For now, broadcast assistant responses to all connected clients
            // In a real implementation, we would route to the specific client that made the request
            std::string response = "{";
            response += "\"type\":\"assistant_response\",";
            response += "\"content\":" + std::string("\"") + msg.content + std::string("\"");
            response += "}\n";

            // Send to all connected clients
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (const auto& pair : active_clients_) {
                send_response(pair.first, response);
            }
        }
    };

    handlers.on_tool_group = [this](const ToolGroup& group) {
        std::string response = serialize_tool_group_message(group);
        std::cout << "[QwenTCPServer] Forwarding tool_group id " << group.id
                  << " to " << active_clients_.size() << " client(s)" << std::endl;

        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& pair : active_clients_) {
            send_response(pair.first, response);
        }
    };
    
    // Error handler
    handlers.on_error = [this](const ErrorMessage& msg) {
        std::string error_response = "{";
        error_response += "\"type\":\"error\",";
        error_response += "\"content\":" + std::string("\"") + msg.message + std::string("\"");
        error_response += "}\n";
        
        // Send error to all connected clients
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& pair : active_clients_) {
            send_response(pair.first, error_response);
        }
    };
    
    qwen_client_->set_handlers(handlers);
    
    // Start the qwen-code client
    if (!qwen_client_->start()) {
        std::cerr << "Warning: Failed to start qwen-code client: " << qwen_client_->get_last_error() << std::endl;
        std::cerr << "Continuing with TCP server functionality only." << std::endl;
        // For now, continue even if qwen-code isn't available for testing
    }
    
    // Create server socket for TCP connections to the Qwen frontend
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Error creating server socket\n";
        return false;
    }

    // Allow reuse of address
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options\n";
        close(server_socket_);
        return false;
    }

    // Set up server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Handle special addresses like "0.0.0.0" and "localhost" properly
    if (host == "0.0.0.0" || host == "localhost") {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported: " << host << "\n";
        close(server_socket_);
        return false;
    }

    // Bind the socket
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket\n";
        close(server_socket_);
        return false;
    }

    // Listen for connections
    if (listen(server_socket_, 10) < 0) {
        std::cerr << "Error listening\n";
        close(server_socket_);
        return false;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(server_socket_, F_GETFL, 0);
    fcntl(server_socket_, F_SETFL, flags | O_NONBLOCK);
    
    running_.store(true);
    
    // Start server thread
    server_thread_ = std::thread(&QwenTCPServer::server_thread, this);
    
    std::cout << "Qwen TCP server started on " << host << ":" << port << std::endl;
    std::cout << "Connected to qwen-code at " << client_config.qwen_executable << std::endl;
    return true;
}

void QwenTCPServer::server_thread() {
    std::vector<struct pollfd> poll_fds;
    struct pollfd server_pollfd;
    server_pollfd.fd = server_socket_;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);

    int loop_count = 0;
    while (running_.load()) {
        loop_count++;

        // Poll for events (with 100ms timeout)
        int ret = poll(poll_fds.data(), poll_fds.size(), 100);

        if (ret < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, continue
            }
            std::cerr << "[QwenTCPServer] Poll error: " << strerror(errno) << "\n";
            break;
        }

        if (ret == 0) {
            // Timeout - no TCP activity, but still poll qwen-code subprocess!
            // (AI responses arrive asynchronously during these quiet periods)
            // Don't continue here - fall through to poll_messages() below
        }

        // Check for new connections on server socket
        if (poll_fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_socket = accept(server_socket_,
                                      (struct sockaddr*)&client_addr,
                                      &client_len);

            if (client_socket >= 0) {
                // Mark client socket as non-blocking
                int flags = fcntl(client_socket, F_GETFL, 0);
                fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

                // Add client socket to poll_fds for monitoring
                struct pollfd client_pollfd;
                client_pollfd.fd = client_socket;
                client_pollfd.events = POLLIN;
                poll_fds.push_back(client_pollfd);

                // Add to our active clients tracking
                {
                    std::lock_guard<std::mutex> lock(clients_mutex_);
                    active_clients_[client_socket] = {
                        client_socket,
                        std::chrono::steady_clock::now(),
                        std::string()
                    };
                }

                std::cout << "[QwenTCPServer] New TCP connection from "
                         << inet_ntoa(client_addr.sin_addr) << "\n";
            }
        }

        // Check for data from existing clients (skip server socket at index 0)
        for (size_t i = 1; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                char buffer[4096];
                ssize_t bytes_read = read(poll_fds[i].fd, buffer, sizeof(buffer) - 1);

                if (bytes_read > 0) {
                    // Null-terminate the buffer
                    buffer[bytes_read] = '\0';

                    // Process the received data
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex_);
                        auto it = active_clients_.find(poll_fds[i].fd);
                        if (it != active_clients_.end()) {
                            it->second.input_buffer += std::string(buffer);

                            // Process complete lines (JSON messages are separated by newlines)
                            std::string& input_buffer = it->second.input_buffer;
                            size_t pos;
                            while ((pos = input_buffer.find('\n')) != std::string::npos) {
                                std::string line = input_buffer.substr(0, pos);
                                input_buffer.erase(0, pos + 1);

                                if (!line.empty()) {
                                    handle_client_message(poll_fds[i].fd, line);
                                }
                            }
                        }
                    }
                } else if (bytes_read == 0) {
                    // EOF on read - client closed write side but may still be listening
                    // Keep the connection alive for sending responses (half-duplex)
                    std::cout << "[QwenTCPServer] Client " << poll_fds[i].fd
                              << " closed write side (EOF), keeping connection for responses\n";
                    // Don't close the socket - just note that we won't get more input
                } else if (poll_fds[i].revents & POLLERR) {
                    // Actual error occurred - close the connection
                    int client_fd = poll_fds[i].fd;

                    // Remove from our tracking
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex_);
                        active_clients_.erase(client_fd);
                    }

                    close(client_fd);

                    // Remove from poll_fds
                    poll_fds.erase(poll_fds.begin() + i);
                    --i;  // Adjust index after removal

                    std::cout << "[QwenTCPServer] Client disconnected (POLLERR): " << client_fd << "\n";
                } else if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    // Error occurred
                    std::cerr << "[QwenTCPServer] Read error: " << strerror(errno) << "\n";

                    // Close the socket
                    int client_fd = poll_fds[i].fd;
                    
                    // Remove from our tracking
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex_);
                        active_clients_.erase(client_fd);
                    }

                    close(client_fd);

                    // Remove from poll_fds
                    poll_fds.erase(poll_fds.begin() + i);
                    --i;  // Adjust index after removal
                }
            }
        }
        
        // Also poll messages from qwen-code if connected
        // Keep polling until no more messages (important for async responses!)
        if (qwen_client_ && qwen_client_->is_running()) {
            int msg_count;
            // Poll repeatedly to catch all available messages
            while ((msg_count = qwen_client_->poll_messages(0)) > 0) {
                // Messages processed via handlers
            }
        }
    }

    // Close all remaining client sockets
    for (size_t i = 1; i < poll_fds.size(); ++i) {
        close(poll_fds[i].fd);
    }

    std::cout << "[QwenTCPServer] Server thread exiting\n";
}

void QwenTCPServer::handle_client_message(int client_fd, const std::string& message) {
    std::cout << "[QwenTCPServer] Processing message from client " << client_fd << ": " << message << std::endl;

    auto type = extract_json_string_field(message, "type");
    if (!type) {
        std::string response = "{\"type\":\"error\",\"content\":\"Missing type field\"}\n";
        std::cout << "[QwenTCPServer] Missing type field in client request\n";
        send_response(client_fd, response);
        return;
    }

    if (*type == "user_input") {
        auto content = extract_json_string_field(message, "content");
        if (!content) {
            std::string response = "{\"type\":\"error\",\"content\":\"Content field not found in JSON\"}\n";
            std::cout << "[QwenTCPServer] Sending field error\n";
            send_response(client_fd, response);
            return;
        }

        std::cout << "[QwenTCPServer] Extracted user content: " << *content << std::endl;
        
        if (qwen_client_ && qwen_client_->is_running()) {
            std::cout << "[QwenTCPServer] Sending to qwen-code: " << *content << std::endl;
            bool sent = qwen_client_->send_user_input(*content);

            if (!sent) {
                // If send failed, send error response
                std::string response = "{\"type\":\"error\",\"content\":\"Failed to send request to qwen-code\"}\n";
                std::cout << "[QwenTCPServer] Sending error response\n";
                send_response(client_fd, response);
            }
            // Response will come asynchronously via the message handlers
            // set up in start() - no need to send anything here
        } else {
            // qwen-code client is not available or not running
            std::string response = "{\"type\":\"error\",\"content\":\"qwen-code client not available\"}\n";
            std::cout << "[QwenTCPServer] qwen-code not available\n";
            send_response(client_fd, response);
        }
    } else if (*type == "tool_approval") {
        auto tool_id = extract_json_string_field(message, "tool_id");
        auto approved = extract_json_bool_field(message, "approved");

        if (!tool_id || !approved) {
            std::string response = "{\"type\":\"error\",\"content\":\"Invalid tool approval payload\"}\n";
            std::cout << "[QwenTCPServer] Invalid tool approval payload\n";
            send_response(client_fd, response);
            return;
        }

        if (qwen_client_ && qwen_client_->is_running()) {
            std::cout << "[QwenTCPServer] Forwarding tool approval for " << *tool_id
                      << " (approved=" << (*approved ? "true" : "false") << ")\n";
            bool sent = qwen_client_->send_tool_approval(*tool_id, *approved);

            if (!sent) {
                std::string response = "{\"type\":\"error\",\"content\":\"Failed to forward tool approval to qwen-code\"}\n";
                std::cout << "[QwenTCPServer] Failed to forward tool approval\n";
                send_response(client_fd, response);
            }
        } else {
            std::string response = "{\"type\":\"error\",\"content\":\"qwen-code client not available\"}\n";
            std::cout << "[QwenTCPServer] qwen-code not available for tool approval\n";
            send_response(client_fd, response);
        }
    } else {
        // Unknown message type
        std::string response = "{\"type\":\"error\",\"content\":\"Unknown message type\"}\n";
        std::cout << "[QwenTCPServer] Sending unknown type error\n";
        send_response(client_fd, response);
    }
}

bool QwenTCPServer::send_response(int client_fd, const std::string& response) {
    if (client_fd < 0) return false;

    ssize_t sent = write(client_fd, response.c_str(), response.size());
    if (sent != static_cast<ssize_t>(response.size())) {
        std::cerr << "[QwenTCPServer] Failed to send complete response. Expected: "
                  << response.size() << ", Sent: " << (sent >= 0 ? sent : 0) << std::endl;
        return false;
    }

    return true;
}

void QwenTCPServer::stop() {
    running_.store(false);
    
    if (server_socket_ != -1) {
        // Close the server socket to break the poll() call in the server thread
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
    
    // Stop the qwen client
    if (qwen_client_ && qwen_client_->is_running()) {
        qwen_client_->stop();
    }
    qwen_client_.reset();
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

bool QwenTCPServer::is_running() const {
    return running_.load();
}

} // namespace Qwen
