#include "QwenProtocol.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace Qwen {

// Implementation of utility functions
const char* message_type_to_string(MessageType type) {
    switch (type) {
        case MessageType::INIT: return "INIT";
        case MessageType::CONVERSATION: return "CONVERSATION";
        case MessageType::TOOL_GROUP: return "TOOL_GROUP";
        case MessageType::STATUS: return "STATUS";
        case MessageType::INFO: return "INFO";
        case MessageType::ERROR: return "ERROR";
        case MessageType::COMPLETION_STATS: return "COMPLETION_STATS";
        default: return "UNKNOWN";
    }
}

const char* command_type_to_string(CommandType type) {
    switch (type) {
        case CommandType::USER_INPUT: return "USER_INPUT";
        case CommandType::TOOL_APPROVAL: return "TOOL_APPROVAL";
        case CommandType::INTERRUPT: return "INTERRUPT";
        case CommandType::MODEL_SWITCH: return "MODEL_SWITCH";
        default: return "UNKNOWN";
    }
}

const char* message_role_to_string(MessageRole role) {
    switch (role) {
        case MessageRole::USER: return "user";
        case MessageRole::ASSISTANT: return "assistant";
        case MessageRole::SYSTEM: return "system";
        default: return "unknown";
    }
}

const char* tool_status_to_string(ToolStatus status) {
    switch (status) {
        case ToolStatus::PENDING: return "pending";
        case ToolStatus::CONFIRMING: return "confirming";
        case ToolStatus::EXECUTING: return "executing";
        case ToolStatus::SUCCESS: return "success";
        case ToolStatus::ERROR: return "error";
        case ToolStatus::CANCELED: return "canceled";
        default: return "unknown";
    }
}

const char* app_state_to_string(AppState state) {
    switch (state) {
        case AppState::IDLE: return "idle";
        case AppState::RESPONDING: return "responding";
        case AppState::WAITING_FOR_CONFIRMATION: return "waiting_for_confirmation";
        default: return "unknown";
    }
}

// Static method implementations for ProtocolParser
std::unique_ptr<StateMessage> ProtocolParser::parse_message(const std::string& json_str) {
    // For now, return nullptr since full implementation requires a JSON parser
    // This would need to be properly implemented with JSON parsing
    return nullptr;
}

std::string ProtocolParser::serialize_command(const Command& cmd) {
    // For now, return an empty string
    // This would need to be properly implemented with JSON serialization
    return "";
}

Command ProtocolParser::create_user_input(const std::string& content) {
    Command cmd;
    cmd.type = CommandType::USER_INPUT;
    cmd.data = UserInputCommand{content};
    return cmd;
}

Command ProtocolParser::create_tool_approval(const std::string& tool_id, bool approved) {
    Command cmd;
    cmd.type = CommandType::TOOL_APPROVAL;
    cmd.data = ToolApprovalCommand{tool_id, approved};
    return cmd;
}

Command ProtocolParser::create_interrupt() {
    Command cmd;
    cmd.type = CommandType::INTERRUPT;
    cmd.data = InterruptCommand{};
    return cmd;
}

Command ProtocolParser::create_model_switch(const std::string& model_id) {
    Command cmd;
    cmd.type = CommandType::MODEL_SWITCH;
    cmd.data = ModelSwitchCommand{model_id};
    return cmd;
}

MessageRole ProtocolParser::parse_role(const std::string& role_str) {
    if (role_str == "user") return MessageRole::USER;
    else if (role_str == "assistant") return MessageRole::ASSISTANT;
    else if (role_str == "system") return MessageRole::SYSTEM;
    else return MessageRole::USER; // default
}

ToolStatus ProtocolParser::parse_tool_status(const std::string& status_str) {
    if (status_str == "pending") return ToolStatus::PENDING;
    else if (status_str == "confirming") return ToolStatus::CONFIRMING;
    else if (status_str == "executing") return ToolStatus::EXECUTING;
    else if (status_str == "success") return ToolStatus::SUCCESS;
    else if (status_str == "error") return ToolStatus::ERROR;
    else if (status_str == "canceled") return ToolStatus::CANCELED;
    else return ToolStatus::PENDING; // default
}

AppState ProtocolParser::parse_app_state(const std::string& state_str) {
    if (state_str == "idle") return AppState::IDLE;
    else if (state_str == "responding") return AppState::RESPONDING;
    else if (state_str == "waiting_for_confirmation") return AppState::WAITING_FOR_CONFIRMATION;
    else return AppState::IDLE; // default
}

std::string ProtocolParser::role_to_string(MessageRole role) {
    switch (role) {
        case MessageRole::USER: return "user";
        case MessageRole::ASSISTANT: return "assistant";
        case MessageRole::SYSTEM: return "system";
        default: return "unknown";
    }
}

std::string ProtocolParser::tool_status_to_string(ToolStatus status) {
    return tool_status_to_string(status); // delegate to the const char* version
}

std::string ProtocolParser::app_state_to_string(AppState state) {
    return app_state_to_string(state); // delegate to the const char* version
}

} // namespace Qwen