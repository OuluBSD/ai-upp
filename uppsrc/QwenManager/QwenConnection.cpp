#include "QwenConnection.h"
#include <Qwen/QwenClient.h>

NAMESPACE_UPP

QwenConnection::QwenConnection() {
    // Initialize with default values
    connected = false;
    healthy = false;
}

void QwenConnection::Init(QwenServerConnectionConf& srv_config) {
    config = &srv_config;  // Store pointer to the configuration

    // Set up the client configuration based on the server configuration
    if (config->connection_type == "tcp") {
        client_config.mode = Qwen::CommunicationMode::TCP;
        client_config.tcp_host = config->host.ToStd();
        client_config.tcp_port = config->port;
    } else if (config->connection_type == "stdin") {
        client_config.mode = Qwen::CommunicationMode::STDIN_STDOUT;
        client_config.qwen_executable = config->directory.ToStd();
    } else if (config->connection_type == "pipe") {
        client_config.mode = Qwen::CommunicationMode::NAMED_PIPE;
        client_config.pipe_path = config->directory.ToStd();
    }

    // Set up message handlers
    client_config.handlers.on_init = [this](const Qwen::InitMessage& msg) {
        LOG("Qwen init message received: " + String().Cat() << msg.version << " - " << msg.workspace_root);
        healthy = true;
    };

    client_config.handlers.on_conversation = [this](const Qwen::ConversationMessage& msg) {
        LOG("Qwen conversation message: " + String().Cat() << msg.content);
        healthy = true;
    };

    client_config.handlers.on_tool_group = [this](const Qwen::ToolGroup& msg) {
        LOG("Qwen tool group message received with " << msg.tools.size() << " tools");
        healthy = true;
    };

    client_config.handlers.on_status = [this](const Qwen::StatusUpdate& msg) {
        String status_msg = msg.message ? String().Cat() << *msg.message : "No message";
        LOG("Qwen status update: " << status_msg);
        healthy = true;
    };

    client_config.handlers.on_info = [this](const Qwen::InfoMessage& msg) {
        LOG("Qwen info message: " << msg.message);
        healthy = true;
    };

    client_config.handlers.on_error = [this](const Qwen::ErrorMessage& msg) {
        LOG("Qwen error: " << msg.message);
        healthy = false;
    };

    client_config.handlers.on_completion_stats = [this](const Qwen::CompletionStats& msg) {
        LOG("Qwen completion stats received");
        healthy = true;
    };

    client_config.auto_restart = true;
    client_config.max_restarts = 3;
    client_config.verbose = true;
}

QwenConnection::~QwenConnection() {
    Disconnect();
}

bool QwenConnection::Connect() {
    if (!config || connected) {
        return false; // Configuration must be set and not already connected
    }

    client = std::make_unique<Qwen::QwenClient>(client_config);
    connected = client->start();
    healthy = connected;

    // Update the configuration's connection status
    config->is_connected = connected;
    config->is_healthy = healthy;

    return connected;
}

void QwenConnection::Disconnect() {
    if (client) {
        client->stop();
        client.reset();
    }
    connected = false;
    healthy = false;

    // Update the configuration's connection status if config is valid
    if (config) {
        config->is_connected = connected;
        config->is_healthy = healthy;
    }
}

bool QwenConnection::SendUserInput(const String& content) {
    if (!connected || !client) {
        return false;
    }

    return client->send_user_input(content.ToStd());
}

String QwenConnection::GetStatus() const {
    if (!connected) {
        return "Disconnected";
    }

    return healthy ? "Connected" : "Connected (Unhealthy)";
}

int QwenConnection::PollMessages(int timeout_ms) {
    if (!connected || !client) {
        return -1;
    }

    return client->poll_messages(timeout_ms);
}

bool QwenConnection::CreateSession() {
    if (!connected || !client) {
        return false;
    }

    // For now, we'll just return true to indicate a new session
    // In a real implementation, we'd create a new session via the Qwen API
    current_session_id = "session_" + Upp::AsString(GetTickCount());  // Use timestamp as unique ID

    // Add the new session ID to the project's session list if we have a project reference
    // This would require additional parameters to link back to the project
    return true;
}

bool QwenConnection::AttachToSession(const String& sessionId) {
    if (!connected || !client) {
        return false;
    }

    current_session_id = sessionId;
    return true;
}

bool QwenConnection::ListSessions(Vector<String>& sessionList) {
    if (!connected || !client) {
        return false;
    }

    // In a real implementation, this would call Qwen API to get sessions
    // For now, we'll just return the current session if it exists
    if (!current_session_id.IsEmpty()) {
        sessionList.Add(current_session_id);
    }

    return true;
}

bool QwenConnection::DeleteSession(const String& sessionId) {
    if (!connected || !client) {
        return false;
    }

    // If we're deleting the current session, clear it
    if (current_session_id == sessionId) {
        current_session_id.Clear();
    }

    return true;
}

String QwenConnection::GetCurrentSessionId() const {
    return current_session_id;
}

END_UPP_NAMESPACE