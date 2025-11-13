#include "CmdQwen.h"

#ifdef flagMAIN
#include <Core/Core.h>

CONSOLE_APP_MAIN {
    // Command-line arguments for qwen command
    Upp::Vector<Upp::String> args;
    const Upp::Vector<Upp::String>& cmd_line = Upp::CommandLine();
    for(int i = 0; i < cmd_line.GetCount(); i++) {
        args.Add(cmd_line[i]);
    }

    // Convert to std::vector<std::string> for the function
    std::vector<std::string> std_args;
    for(int i = 0; i < args.GetCount(); i++) {
        std_args.push_back(std::string(args[i]));
    }

    // Call the qwen command implementation
    Upp::MountManager& mm = Upp::MountManager::System();
    QwenCmd::cmd_qwen(std_args, reinterpret_cast<Upp::VFS&>(mm));

    return; // Don't return an int from void function
}
#endif