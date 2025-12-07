#include "CommandExecutor.h"
#include "IdeSession.h"
#include <Core/Core.h>

namespace Upp {

CommandExecutor::CommandExecutor(const CommandRegistry& registry)
    : registry(registry), session(CreateIdeSession()) {}

InvocationResult CommandExecutor::Invoke(const String& name,
                                        const VectorMap<String, String>& args) {
    // Find the command in the registry
    const Command* cmd = registry.FindByName(name);
    if (!cmd) {
        return {1, "Unknown command: " + name};
    }

    // Validate arguments against command metadata
    for (const Argument& arg : cmd->inputs) {
        if (arg.required && !args.Find(arg.name)) {
            return {1, "Missing required argument: " + arg.name};
        }
    }

    // Dispatch to appropriate handler based on command name
    if (name == "open_file") {
        return HandleOpenFile(args);
    }
    else if (name == "save_file") {
        return HandleSaveFile(args);
    }
    else if (name == "find_in_files") {
        return HandleFindInFiles(args);
    }
    else if (name == "build_project") {
        return HandleBuildProject(args);
    }
    else if (name == "clean_project") {
        return HandleCleanProject(args);
    }
    else if (name == "goto_line") {
        return HandleGotoLine(args);
    }
    else if (name == "search_code") {
        return HandleSearchCode(args);
    }
    else if (name == "show_console") {
        return HandleShowConsole(args);
    }
    else if (name == "show_errors") {
        return HandleShowErrors(args);
    }
    else if (name == "set_main_package") {
        return HandleSetMainPackage(args);
    }
    // Special case for list_commands (not in metadata)
    else if (name == "list_commands") {
        Vector<Command> allCmds = registry.ListCommands();
        String msg = "Available commands:\n";
        for (const Command& c : allCmds) {
            msg += c.name + " (" + c.category + "): " + c.description + "\n";
        }
        return {0, msg};
    }

    return {1, "Unimplemented command: " + name};
}

InvocationResult CommandExecutor::HandleOpenFile(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String error;

    if (!session->OpenFile(path, error)) {
        return {1, "Failed to open file '" + path + "': " + error};
    }

    return {0, "Opened file '" + path + "'"};
}

InvocationResult CommandExecutor::HandleSaveFile(const VectorMap<String, String>& args) {
    String path = args.Get("path", "current");
    String error;

    if (path == "current") {
        // For now, we can't save "current" file from CLI - require explicit path
        return {1, "Saving without explicit path is not yet supported from CLI"};
    }

    if (!session->SaveFile(path, error)) {
        return {1, "Failed to save file '" + path + "': " + error};
    }

    return {0, "Saved file '" + path + "'"};
}

InvocationResult CommandExecutor::HandleFindInFiles(const VectorMap<String, String>& args) {
    String pattern = args.Get("pattern", "");
    String directory = args.Get("directory", ".");

    String result, error;
    if (!session->FindInFiles(pattern, directory, result, error)) {
        return {1, "Failed to find in files: " + error};
    }

    return {0, "Find in files completed:\n" + result};
}

InvocationResult CommandExecutor::HandleBuildProject(const VectorMap<String, String>& args) {
    String name = args.Get("name", "");
    String config = args.Get("config", "Debug");
    String log, error;

    if (!session->BuildProject(name, config, log, error)) {
        return {1, "Failed to build project '" + name + "' with config '" + config + "': " + error};
    }

    return {0, "Build completed for project '" + name + "' with config '" + config + "'\nLog:\n" + log};
}

InvocationResult CommandExecutor::HandleCleanProject(const VectorMap<String, String>& args) {
    String name = args.Get("name", "");
    String log, error;

    if (!session->CleanProject(name, log, error)) {
        return {1, "Failed to clean project '" + name + "': " + error};
    }

    return {0, "Clean completed for project '" + name + "'\nLog:\n" + log};
}

InvocationResult CommandExecutor::HandleGotoLine(const VectorMap<String, String>& args) {
    String lineStr = args.Get("line", "1");
    String file = args.Get("file", "");

    int line = ScanInt(lineStr);  // Convert string to integer
    String error;

    if (!session->GotoLine(file, line, error)) {
        return {1, "Failed to go to line " + IntStr(line) + " in file '" + file + "': " + error};
    }

    return {0, "Navigated to line " + IntStr(line) + " in file '" + file + "'"};
}

InvocationResult CommandExecutor::HandleSearchCode(const VectorMap<String, String>& args) {
    String query = args.Get("query", "");

    String result, error;
    if (!session->SearchCode(query, result, error)) {
        return {1, "Failed to search code: " + error};
    }

    return {0, "Search code completed:\n" + result};
}

InvocationResult CommandExecutor::HandleShowConsole(const VectorMap<String, String>& args) {
    String error;
    if (!session->ShowConsole(error)) {
        return {1, "Failed to access console: " + error};
    }

    // Get the console output
    String output;
    if (!session->GetConsoleOutput(output, error)) {
        return {1, "Failed to retrieve console output: " + error};
    }

    return {0, "Console output:\n" + output};
}

InvocationResult CommandExecutor::HandleShowErrors(const VectorMap<String, String>& args) {
    String error;
    if (!session->ShowErrors(error)) {
        return {1, "Failed to access errors: " + error};
    }

    // Get the errors output
    String output;
    if (!session->GetErrorsOutput(output, error)) {
        return {1, "Failed to retrieve errors output: " + error};
    }

    return {0, "Errors output:\n" + output};
}

InvocationResult CommandExecutor::HandleSetMainPackage(const VectorMap<String, String>& args) {
    String pkg = args.Get("package", "");
    String error;

    if (!session->SetMainPackage(pkg, error)) {
        return {1, "Failed to set main package to '" + pkg + "': " + error};
    }

    return {0, "Set main package to '" + pkg + "'"};
}

}