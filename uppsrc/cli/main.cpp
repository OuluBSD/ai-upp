#include "cli.h"
#include "IdeHost.h"

// Simple command-line argument parser
Upp::VectorMap<Upp::String, Upp::String> ParseArguments(int argc, const char* argv[]) {
    Upp::VectorMap<Upp::String, Upp::String> args;

    for (int i = 2; i < argc; i += 2) {  // Start from 2 to skip command name
        if (i + 1 < argc && Upp::String(argv[i]).StartsWith("--")) {
            Upp::String key = Upp::String(argv[i]).Mid(2);  // Remove '--' prefix
            Upp::String value = argv[i + 1];
            args.GetAdd(key, value);
        }
    }

    return args;
}

void PrintUsage() {
    RLOG("Usage: theide-cli <command_name> [--arg value ...]");
    RLOG("Available commands: list_commands, open_file, save_file, find_in_files, build_project, clean_project, goto_line, search_code, show_console, show_errors, set_main_package");
    RLOG("Example: theide-cli open_file --path /path/to/file.cpp");
}

GUI_APP_MAIN {
    if (commandLine.GetCount() < 1) {
        PrintUsage();
        return 1;
    }

    // Initialize the IDE host
    Upp::String error;
    if (!Upp::GetGlobalIdeHost().Init(error)) {
        cout << "ERROR initializing IDE host: " << error << std::endl;
        return 1;
    }

    // Get command name from first argument
    Upp::String commandName = commandLine[0];

    // Create registry and load commands
    Upp::CommandRegistry registry;
    registry.LoadCommands("../metadata/commands_v1.json");

    // Create executor
    Upp::CommandExecutor executor(registry);

    // Parse remaining arguments
    Upp::VectorMap<Upp::String, Upp::String> args = ParseArguments(commandLine.GetCount(), commandLine.Get());

    // Execute the command
    Upp::InvocationResult result = executor.Invoke(commandName, args);

    if (result.status_code == 0) {
        cout << "SUCCESS: " << result.message << std::endl;
    } else {
        cout << "ERROR: " << result.message << std::endl;

        // If the command is unknown, list available commands
        if (result.message.StartsWith("Unknown command")) {
            cout << "\nAvailable commands:" << std::endl;
            Upp::Vector<Upp::Command> commands = registry.ListCommands();
            for (const Upp::Command& cmd : commands) {
                cout << "  " << cmd.name << " - " << cmd.description << std::endl;
            }
        }
    }
}