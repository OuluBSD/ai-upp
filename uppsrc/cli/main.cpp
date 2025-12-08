#include "cli.h"
#include "IdeSession.h"
#include "CommandRegistry.h"
#include "CommandExecutor.h"

// Simple command-line argument parser that handles global options before command
Upp::VectorMap<Upp::String, Upp::String> ParseArguments(int argc, const char* argv[], int& start_index) {
    Upp::VectorMap<Upp::String, Upp::String> args;

    // First check for global arguments that affect the entire session
    for (int i = 1; i < argc; i += 2) {  // Start from 1, looking for global options before command
        if (i + 1 < argc && Upp::String(argv[i]).StartsWith("--")) {
            Upp::String key = Upp::String(argv[i]).Mid(2);  // Remove '--' prefix
            Upp::String value = argv[i + 1];

            if (key == "workspace-root") {
                // This is a global argument, so we'll handle it separately
                // For now, just store it in args but also determine where to start parsing
                args.GetAdd(key, value);
                start_index = i + 2;  // Start command arguments after this global option
                break;  // For now, just handle one global option
            }
        }
    }

    // If no global options were found, start parsing from index 1 (command name)
    if (start_index == 0) {
        start_index = 1;
    }

    // Now parse command-specific arguments
    for (int i = start_index + 1; i < argc; i += 2) {  // Skip command name at start_index
        if (i + 1 < argc && Upp::String(argv[i]).StartsWith("--")) {
            Upp::String key = Upp::String(argv[i]).Mid(2);  // Remove '--' prefix
            Upp::String value = argv[i + 1];
            args.GetAdd(key, value);
        }
    }

    return args;
}

// Parse just global arguments from Vector<String>
Upp::VectorMap<Upp::String, Upp::String> ParseGlobalArguments(const Vector<String>& args, int& end_index) {
    Upp::VectorMap<Upp::String, Upp::String> global_args;

    int i = 1; // Start after executable name
    while (i < args.GetCount() && args[i].StartsWith("--")) {
        if (i + 1 < args.GetCount()) {
            Upp::String key = args[i].Mid(2);  // Remove '--' prefix
            Upp::String value = args[i + 1];

            if (key == "workspace-root") {
                global_args.GetAdd(key, value);
                i += 2; // Skip key and value
            } else {
                // If we encounter a global argument we don't know, stop parsing globals
                break;
            }
        } else {
            break;
        }
    }

    end_index = i; // Position after the last global argument
    return global_args;
}

void PrintUsage() {
    RLOG("Usage: theide-cli <command_name> [--arg value ...]");
    RLOG("Available commands: list_commands, open_file, save_file, find_in_files, build_project, clean_project, goto_line, search_code, show_console, show_errors, set_main_package");
    RLOG("Example: theide-cli open_file --path /path/to/file.cpp");
}

CONSOLE_APP_MAIN {
	using namespace Upp;
	Stream& cout = Cout();
	const Vector<String>& commandLine = CommandLine();
	
    if (commandLine.GetCount() < 1) {
        PrintUsage();
        SetExitCode(1);
        return;
    }

    // No IDE host initialization needed for CLI core
    String error;

    // Parse global arguments (like --workspace-root) that might come before the command name
    int global_args_end_index = 1; // Start after program name
    VectorMap<String, String> global_args = ParseGlobalArguments(commandLine, global_args_end_index);

    // Create session and handle global arguments
    One<IdeSession> session = CreateIdeSession();

    // Set workspace root if provided as a global argument
    if (global_args.Find("workspace-root") >= 0) {
        String workspace_root = global_args.Get("workspace-root");
        if (!session->SetWorkspaceRoot(workspace_root, error)) {
            cout << "ERROR setting workspace root: " << error; cout.PutEol();
	        SetExitCode(1);
	        return;
        }
    }

    // Get command name - it should be at the global_args_end_index position
    if (global_args_end_index >= commandLine.GetCount()) {
        PrintUsage();
        SetExitCode(1);
        return;
    }

    String commandName = commandLine[global_args_end_index];

    // Create registry and load commands
    CommandRegistry registry;
    registry.LoadCommands("../metadata/commands_v1.json");

    // Create executor with the session
    CommandExecutor executor(registry, pick(session));

    // Parse command-specific arguments (starting after the command name)
    int cmd_args_start = global_args_end_index + 1;
    VectorMap<String, String> args;
    for (int i = cmd_args_start; i < commandLine.GetCount(); i += 2) {
        if (i + 1 < commandLine.GetCount() && String(commandLine[i]).StartsWith("--")) {
            String key = String(commandLine[i]).Mid(2);  // Remove '--' prefix
            String value = commandLine[i + 1];
            args.GetAdd(key, value);
        }
        // Note: We ignore arguments that don't start with -- as they're malformed
    }

    // Execute the command
    InvocationResult result = executor.Invoke(commandName, args);

    if (result.status_code == 0) {
        cout << "SUCCESS: " << result.message; cout.PutEol();
    } else {
        cout << "ERROR: " << result.message; cout.PutEol();

        // If the command is unknown, list available commands
        if (result.message.StartsWith("Unknown command")) {
            cout << "\nAvailable commands:"; cout.PutEol();
            Vector<Command> commands = registry.ListCommands();
            for (const Command& cmd : commands) {
                cout << "  " << cmd.name << " - " << cmd.description; cout.PutEol();
            }
        }
    }
}