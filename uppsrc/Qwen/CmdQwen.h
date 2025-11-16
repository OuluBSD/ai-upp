#ifndef _VfsShell_cmd_qwen_h_
#define _VfsShell_cmd_qwen_h_

#include "QwenClient.h"
#include "QwenStateManager.h"
#include <string>
#include <vector>
#include <map>

// Include VFS definitions
#include <Core/VfsBase/VfsBase.h>
// We define Vfs as an alias to VFS to match U++ conventions
using Vfs = Upp::VFS;

// Use the Qwen namespace classes
using Qwen::QwenClient;
using Qwen::QwenStateManager;

namespace QwenCmd {

// Configuration for qwen command
struct QwenConfig {
    std::string model = "qwen-oauth";  // Default model - using qwen-oauth as recommended
    std::string workspace_root;
    std::string qwen_code_path;  // Empty = use script/qwen-code wrapper (default)
    bool auto_approve_tools = false;
    bool use_colors = true;
    int max_retries = 3;

    // Load from VFS (/env/qwen_config.json) or environment
    void load_from_env(const std::map<std::string, std::string>& env);

    // Load from VFS file
    bool load_from_file(const std::string& vfs_path, Vfs& vfs);
};

// Options parsed from command-line arguments
struct QwenOptions {
    bool attach = false;
    bool new_session = false;  // Force new session creation
    bool list_sessions = false;
    bool clear_sessions = false;  // Clear all sessions
    bool help = false;
    bool simple_mode = false;  // Force stdio mode instead of ncurses
    bool use_openai = false;   // Use OpenAI instead of default provider
    bool use_oauth = false;    // Use qwen-oauth instead of default provider
    bool manager_mode = false; // Enable manager mode
    bool auto_approve_tools = false; // Auto-approve all tools
    std::string eval_input;    // Evaluate single input and exit
    std::string session_id;
    std::string model;
    std::string workspace_root;
    std::string mode = "stdin";  // Connection mode: stdin, tcp, pipe
    int port = 7777;             // TCP port (for mode=tcp)
    std::string host = "localhost";  // TCP host (for mode=tcp)
};

// Parse command-line arguments
QwenOptions parse_args(const std::vector<std::string>& args);

// Show help text
void show_help();

// Main qwen command entry point
void cmd_qwen(const std::vector<std::string>& args,
              Vfs& vfs);

// List all sessions
void list_sessions(QwenStateManager& state_mgr);

}  // namespace QwenCmd

#endif
