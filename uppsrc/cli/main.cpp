#include "cli.h"
#include "IdeSession.h"
#include "CommandRegistry.h"
#include "CommandExecutor.h"

// Parse just global arguments from Vector<String>
Upp::VectorMap<Upp::String, Upp::String> ParseGlobalArguments(const Vector<String>& args, bool& json_mode, bool& json_script_mode, String& json_script_path, int& end_index) {
    Upp::VectorMap<Upp::String, Upp::String> global_args;
    json_mode = false;
    json_script_mode = false;
    json_script_path = "";

    int i = 1; // Start after executable name
    while (i < args.GetCount() && args[i].StartsWith("--")) {
        Upp::String key = args[i].Mid(2);  // Remove '--' prefix

        if (key == "workspace-root" && i + 1 < args.GetCount()) {
            Upp::String value = args[i + 1];
            global_args.GetAdd(key, value);
            i += 2; // Skip key and value
        } else if (key == "script" && i + 1 < args.GetCount()) {
            Upp::String value = args[i + 1];
            global_args.GetAdd(key, value);
            i += 2; // Skip key and value
        } else if (key == "json-script" && i + 1 < args.GetCount()) {
            json_script_path = args[i + 1];
            json_script_mode = true;
            i += 2; // Skip key and value
        } else if (key == "json") {
            json_mode = true;
            i += 1; // Skip just the key
        } else {
            // If we encounter a global argument we don't know, stop parsing globals
            break;
        }
    }

    end_index = i; // Position after the last global argument
    return global_args;
}

// Helper function to convert InvocationResult to JSON string
String ToJson(const String& cmd_name, const InvocationResult& r) {
    ValueMap json_result;
    json_result.Add("command", cmd_name);
    json_result.Add("status_code", r.status_code);
    json_result.Add("message", r.message);
    json_result.Add("payload", r.payload);

    String json_str;
    try {
        json_str = AsJSON(json_result);
    } catch (...) {
        // Fallback if JSON serialization fails
        json_result.Add("payload", Value());  // Clear payload if it caused issues
        json_str = AsJSON(json_result);
    }

    return json_str;
}

// Parse command-line arguments from a script line
VectorMap<String, String> ParseCommandLineArgs(const String& line) {
    VectorMap<String, String> args;
    Vector<String> parts = Split(line, ' ');

    for (int i = 0; i < parts.GetCount(); i++) {
        if (parts[i].StartsWith("--") && i + 1 < parts.GetCount()) {
            String key = parts[i].Mid(2);  // Remove '--' prefix
            String value = parts[++i];     // Get the next part as the value
            args.Add(key, value);
        }
    }

    return args;
}

void PrintUsage() {
    RLOG("Usage: theide-cli [--json] [--workspace-root PATH] <command_name> [--arg value ...]");
    RLOG("Or: theide-cli [--json] --workspace-root PATH --script PATH");
    RLOG("Or: theide-cli [--json] --workspace-root PATH --json-script PATH");
    RLOG("Available commands: list_commands, open_file, save_file, find_in_files, build_project, clean_project, goto_line, search_code, show_console, show_errors, set_main_package, insert_text, erase_range, replace_all, undo, redo, find_definition, find_usages, get_build_order, detect_cycles, affected_packages, rename_symbol, remove_dead_includes, canonicalize_includes, describe_command, orchestrator_add_workspace, orchestrator_summaries, orchestrator_roadmap, list_agents, agent_plan, global_plan");
    RLOG("Global options: --workspace-root PATH, --json, --script PATH, --json-script PATH");
    RLOG("Example: theide-cli open_file --path /path/to/file.cpp");
    RLOG("Example: theide-cli --workspace-root . --json find_definition --symbol MyClass");
    RLOG("Example: theide-cli --workspace-root . --json describe_command --name build_project");
    RLOG("Example: theide-cli --workspace-root . --script commands.txt");
    RLOG("Example: theide-cli --workspace-root . --json --json-script plan.json");
    RLOG("Example: theide-cli orchestrator_add_workspace --path /project/path");
    RLOG("Example: theide-cli --json orchestrator_summaries");
    RLOG("Example: theide-cli --json orchestrator_roadmap --strategy stability-first");
}

// Execute commands from a script file
bool ExecuteScript(const String& scriptPath, const VectorMap<String, String>& globalArgs,
                   bool json_mode, CommandExecutor& executor, One<IdeSession>& session) {
    using namespace Upp;
    Stream& cout = Cout();

    FileIn in(scriptPath);
    if (!in.IsOpen()) {
        if (json_mode) {
            InvocationResult error_result;
            error_result.status_code = 1;
            error_result.message = "ERROR: Could not open script file: " + scriptPath;
            cout << ToJson("script", error_result); cout.PutEol();
        } else {
            cout << "ERROR: Could not open script file: " << scriptPath; cout.PutEol();
        }
        return false;
    }

    String error;
    // Set workspace root if provided globally
    if (globalArgs.Find("workspace-root") >= 0) {
        String workspace_root = globalArgs.Get("workspace-root");
        if (!session->SetWorkspaceRoot(workspace_root, error)) {
            if (json_mode) {
                InvocationResult error_result;
                error_result.status_code = 1;
                error_result.message = "ERROR setting workspace root: " + error;
                cout << ToJson("script", error_result); cout.PutEol();
            } else {
                cout << "ERROR setting workspace root: " + error << endl;
            }
            return false;
        }
    }

    // Read and execute the script line by line
    while (!in.IsEof()) {
        String line = in.GetLine();

        // Skip comments and empty lines
        if (line.IsEmpty() || line.StartsWith("#")) {
            continue;
        }

        // Show the command being executed (unless in JSON mode)
        if (!json_mode) {
            cout << ">>> " << line << endl;
        }

        // Split the line into parts to extract command name and arguments
        Vector<String> parts = Split(line, ' ');
        if (parts.IsEmpty()) {
            continue;
        }

        String commandName = parts[0];

        // Parse command line arguments from the script line
        VectorMap<String, String> args = ParseCommandLineArgs(line);

        // Execute the command
        InvocationResult result = executor.Invoke(commandName, args);

        if (json_mode) {
            cout << ToJson(commandName, result) << endl;
        } else {
            if (result.status_code == 0) {
                cout << "SUCCESS: " << result.message << endl;
            } else {
                cout << "ERROR: " << result.message << endl;
                return false; // Stop on error for v1 implementation
            }
        }
    }

    return true;
}

// Execute commands from a JSON script file
bool ExecuteJsonScript(const String& jsonScriptPath, const VectorMap<String, String>& globalArgs,
                       bool json_mode, CommandExecutor& executor, One<IdeSession>& session) {
    using namespace Upp;
    Stream& cout = Cout();

    // Load the JSON file
    String jsonContent = LoadFile(jsonScriptPath);
    if (jsonContent.IsEmpty()) {
        if (json_mode) {
            InvocationResult error_result;
            error_result.status_code = 1;
            error_result.message = "ERROR: Could not load JSON script file: " + jsonScriptPath;
            cout << ToJson("json-script", error_result); cout.PutEol();
        } else {
            cout << "ERROR: Could not load JSON script file: " + jsonScriptPath << endl;
        }
        return false;
    }

    // Parse the JSON
    try {
        Value json = ParseJSON(jsonContent);

        // Get the commands array
        if (!json("commands").Is<ValueArray>()) {
            if (json_mode) {
                InvocationResult error_result;
                error_result.status_code = 1;
                error_result.message = "ERROR: Invalid JSON script format: missing 'commands' array";
                cout << ToJson("json-script", error_result); cout.PutEol();
            } else {
                cout << "ERROR: Invalid JSON script format: missing 'commands' array" << endl;
            }
            return false;
        }

        ValueArray commandsArray = json("commands");
        bool stopOnError = (bool)json("stop_on_error");

        String error;
        // Set workspace root if provided globally
        if (globalArgs.Find("workspace-root") >= 0) {
            String workspace_root = globalArgs.Get("workspace-root");
            if (!session->SetWorkspaceRoot(workspace_root, error)) {
                if (json_mode) {
                    InvocationResult error_result;
                    error_result.status_code = 1;
                    error_result.message = "ERROR setting workspace root: " + error;
                    cout << ToJson("json-script", error_result); cout.PutEol();
                } else {
                    cout << "ERROR setting workspace root: " + error << endl;
                }
                return false;
            }
        }

        // Execute each command in sequence
        for(int i = 0; i < commandsArray.GetCount(); i++) {
            Value cmdValue = commandsArray[i];
            String commandName = cmdValue("name");

            if (commandName.IsEmpty()) {
                if (json_mode) {
                    InvocationResult error_result;
                    error_result.status_code = 1;
                    error_result.message = "ERROR: Command at index " + AsString(i) + " has no name";
                    cout << ToJson("json-script", error_result); cout.PutEol();
                } else {
                    cout << "ERROR: Command at index " + AsString(i) + " has no name" << endl;
                }
                if (stopOnError) return false;
                continue;
            }

            // Get arguments for this command
            VectorMap<String, String> args;
            Value argsValue = cmdValue("args");
            if(argsValue.Is<ValueMap>()) {
                ValueMap argsMap = argsValue;
                for(int j = 0; j < argsMap.GetCount(); j++) {
                    args.GetAdd(argsMap.GetKey(j), argsMap[j]);
                }
            }

            // Execute the command
            InvocationResult result = executor.Invoke(commandName, args);

            // Output the result
            if (json_mode) {
                cout << ToJson(commandName, result) << endl;
            } else {
                if (result.status_code == 0) {
                    cout << "SUCCESS: " << result.message << endl;
                } else {
                    cout << "ERROR: " << result.message << endl;
                }
            }

            // Check if we should stop on error
            if (stopOnError && result.status_code != 0) {
                return false;
            }
        }
    } catch (const std::exception& e) {
        if (json_mode) {
            InvocationResult error_result;
            error_result.status_code = 1;
            error_result.message = "ERROR: Exception while processing JSON script: " + String(e.what());
            cout << ToJson("json-script", error_result); cout.PutEol();
        } else {
            cout << "ERROR: Exception while processing JSON script: " + String(e.what()) << endl;
        }
        return false;
    } catch (...) {
        if (json_mode) {
            InvocationResult error_result;
            error_result.status_code = 1;
            error_result.message = "ERROR: Unknown exception while processing JSON script";
            cout << ToJson("json-script", error_result); cout.PutEol();
        } else {
            cout << "ERROR: Unknown exception while processing JSON script" << endl;
        }
        return false;
    }

    return true;
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

    // Parse global arguments (like --workspace-root, --script, --json, --json-script) that might come before the command name
    int global_args_end_index = 1; // Start after program name
    bool json_mode = false;
    bool json_script_mode = false;
    String json_script_path;
    VectorMap<String, String> global_args = ParseGlobalArguments(commandLine, json_mode, json_script_mode, json_script_path, global_args_end_index);

    // Check if we're in script mode
    bool script_mode = (global_args.Find("script") >= 0);

    if (script_mode) {
        // Create session and handle global arguments
        One<IdeSession> session = CreateIdeSession();

        // Create registry and load commands
        CommandRegistry registry;
        registry.LoadCommands("../metadata/commands_v1.json");

        // Create executor with the session
        CommandExecutor executor(registry, pick(session));

        // Get the script path
        String scriptPath = global_args.Get("script");

        // Execute the script
        bool success = ExecuteScript(scriptPath, global_args, json_mode, executor, session);

        if (!success) {
            SetExitCode(1);
        }
    } else if (json_script_mode) {
        // JSON script mode
        // Create session and handle global arguments
        One<IdeSession> session = CreateIdeSession();

        // Create registry and load commands
        CommandRegistry registry;
        registry.LoadCommands("../metadata/commands_v1.json");

        // Create executor with the session
        CommandExecutor executor(registry, pick(session));

        // Execute the JSON script
        bool success = ExecuteJsonScript(json_script_path, global_args, json_mode, executor, session);

        if (!success) {
            SetExitCode(1);
        }
    } else {
        // Single command mode
        // Create session and handle global arguments
        One<IdeSession> session = CreateIdeSession();

        // Set workspace root if provided as a global argument
        if (global_args.Find("workspace-root") >= 0) {
            String workspace_root = global_args.Get("workspace-root");
            if (!session->SetWorkspaceRoot(workspace_root, error)) {
                if (json_mode) {
                    InvocationResult error_result;
                    error_result.status_code = 1;
                    error_result.message = "ERROR setting workspace root: " + error;
                    cout << ToJson("", error_result); cout.PutEol();
                } else {
                    cout << "ERROR setting workspace root: " << error; cout.PutEol();
                }
                SetExitCode(1);
                return;
            }
        }

        // Get command name - it should be at the global_args_end_index position
        if (global_args_end_index >= commandLine.GetCount()) {
            if (json_mode) {
                InvocationResult error_result;
                error_result.status_code = 1;
                error_result.message = "No command specified";
                cout << ToJson("", error_result); cout.PutEol();
            } else {
                PrintUsage();
            }
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

        if (json_mode) {
            cout << ToJson(commandName, result); cout.PutEol();
        } else {
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
    }
}