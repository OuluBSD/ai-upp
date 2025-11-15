#include "CmdQwen.h"

// Only include main function when building as a standalone application
// (not when being included as part of VfsShell build with its own main)
// The issue is that flagMAIN is defined by the build system when building as executable
// but if VfsShell is also included, there will be a main conflict.
// So we'll use the flag to determine if we need a main function.
#ifdef flagMAIN

#include <Core/Core.h>
#include <Core/VfsBase/VfsBase.h>
#include <Core/VfsBase/Mount.h>

using namespace Upp;

// Initialize MountManager with system filesystem mounted at root
void InitializeMountSystem() {
    MountManager& mm = MountManager::System();

    // Mount system filesystem at root "/"
    SystemFS* sysfs = new SystemFS();
    mm.Mount("/", sysfs, "sysfs");

    // This makes the system filesystem available at the root path
}

CONSOLE_APP_MAIN {
    // Initialize the mount system before any VFS operations
    InitializeMountSystem();

    // Get command-line arguments
    const Vector<String>& cmd_line = CommandLine();

    // Convert to std::vector<std::string> for QwenCmd::cmd_qwen
    std::vector<std::string> std_args;
    for(int i = 0; i < cmd_line.GetCount(); i++) {
        std_args.push_back(std::string(cmd_line[i].Begin(), cmd_line[i].GetLength()));
    }

    // Parse options to check for help flag
    QwenCmd::QwenOptions opts = QwenCmd::parse_args(std_args);

    // If no arguments or --help flag, show help
    if (std_args.empty() || opts.help) {
        QwenCmd::show_help();
        return;
    }

    // Get the root VFS from MountManager
    // We need to find the root mount point "/" which should be SystemFS
    MountManager& mm = MountManager::System();
    MountManager::MountPoint* mp = mm.Find("/");

    if (!mp || !mp->vfs) {
        Cerr() << "Error: Could not access VFS. Root filesystem not mounted.\n";
        Cerr() << "Please ensure SystemFS is mounted at '/' before running Qwen.\n";
        SetExitCode(1);
        return;
    }

    // Call the qwen command implementation with the VFS instance
    QwenCmd::cmd_qwen(std_args, *mp->vfs);
}

#endif