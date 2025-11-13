#include "CmdQwen.h"

#ifdef flagMAIN
#define CONSOLE_APP_MAIN
#include <Core/Core.h>

CONSOLE_APP_MAIN {
    // Command-line arguments for qwen command
    Vector<String> args;
    for(int i = 0; i < GetArgCount(); i++) {
        args.Add(GetArg(i));
    }
    
    // Call the qwen command implementation
    cmd_qwen(args, MountManager::System());
    
    return 0;
}
#endif