#include "CommandExecutor.h"
#include "IdeSession.h"
#include <Core/Core.h>

namespace Upp {

CommandExecutor::CommandExecutor(const CommandRegistry& registry, One<IdeSession> session)
    : registry(registry), session(pick(session)) {}

InvocationResult CommandExecutor::Invoke(const String& name,
                                        const VectorMap<String, String>& args) {
    // Find the command in the registry
    const Command* cmd = registry.FindByName(name);
    if (!cmd) {
        return InvocationResult(1, "Unknown command: " + name);
    }

    // Validate arguments against command metadata
    for (const Argument& arg : cmd->inputs) {
        if (arg.required && !args.Find(arg.name)) {
            return InvocationResult(1, "Missing required argument: " + arg.name);
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
    else if (name == "insert_text") {
        return HandleInsertText(args);
    }
    else if (name == "erase_range") {
        return HandleEraseRange(args);
    }
    else if (name == "replace_all") {
        return HandleReplaceAll(args);
    }
    else if (name == "undo") {
        return HandleUndo(args);
    }
    else if (name == "redo") {
        return HandleRedo(args);
    }
    else if (name == "find_definition") {
        return HandleFindDefinition(args);
    }
    else if (name == "find_usages") {
        return HandleFindUsages(args);
    }
    else if (name == "get_build_order") {
        return HandleGetBuildOrder(args);
    }
    else if (name == "detect_cycles") {
        return HandleDetectCycles(args);
    }
    else if (name == "affected_packages") {
        return HandleAffectedPackages(args);
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

InvocationResult CommandExecutor::HandleInsertText(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String posStr = args.Get("pos", "0");
    String text = args.Get("text", "");

    int pos = ScanInt(posStr);
    if (pos < 0) {
        return {1, "Invalid position: " + posStr};
    }

    String error;
    if (!session->EditorInsert(path, pos, text, error)) {
        return {1, "Failed to insert text: " + error};
    }

    return {0, "Inserted text into '" + path + "' at position " + IntStr(pos) + "."};
}

InvocationResult CommandExecutor::HandleEraseRange(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String posStr = args.Get("pos", "0");
    String countStr = args.Get("count", "0");

    int pos = ScanInt(posStr);
    int count = ScanInt(countStr);
    if (pos < 0 || count < 0) {
        return {1, "Invalid position or count: " + posStr + ", " + countStr};
    }

    String error;
    if (!session->EditorErase(path, pos, count, error)) {
        return {1, "Failed to erase range: " + error};
    }

    return {0, "Erased " + IntStr(count) + " characters in '" + path + "' starting at position " + IntStr(pos) + "."};
}

InvocationResult CommandExecutor::HandleReplaceAll(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String pattern = args.Get("pattern", "");
    String replacement = args.Get("replacement", "");
    String caseSensitiveStr = args.Get("case_sensitive", "true");

    bool case_sensitive = true;
    if (caseSensitiveStr == "false" || caseSensitiveStr == "0") {
        case_sensitive = false;
    }

    String error;
    int count;
    if (!session->EditorReplaceAll(path, pattern, replacement, case_sensitive, count, error)) {
        return {1, "Failed to replace all: " + error};
    }

    return {0, "Replaced " + IntStr(count) + " occurrences in '" + path + "'."};
}

InvocationResult CommandExecutor::HandleGotoLine(const VectorMap<String, String>& args) {
    String lineStr = args.Get("line", "1");
    String file = args.Get("file", "");

    int line = ScanInt(lineStr);
    String error;
    int pos;
    if (!session->EditorGotoLine(file, line, pos, error)) {
        return {1, "Failed to go to line " + IntStr(line) + " in file '" + file + "': " + error};
    }

    return {0, "Moved to line " + IntStr(line) + " (pos " + IntStr(pos) + ") in '" + file + "'."};
}

InvocationResult CommandExecutor::HandleUndo(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String error;

    if (!session->EditorUndo(path, error)) {
        return {1, "Failed to undo: " + error};
    }

    return {0, "Undid last edit in '" + path + "'."};
}

InvocationResult CommandExecutor::HandleRedo(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    String error;

    if (!session->EditorRedo(path, error)) {
        return {1, "Failed to redo: " + error};
    }

    return {0, "Redid last undone edit in '" + path + "'."};
}

InvocationResult CommandExecutor::HandleFindDefinition(const VectorMap<String, String>& args) {
    String symbol = args.Get("symbol", "");
    if (symbol.IsEmpty()) {
        return {1, "Missing required argument: symbol"};
    }

    String file, error;
    int line = -1;
    if (!session->FindDefinition(symbol, file, line, error)) {
        return {1, "Failed to find definition for symbol '" + symbol + "': " + error};
    }

    return {0, "Definition found: " + file + ":" + IntStr(line)};
}

InvocationResult CommandExecutor::HandleFindUsages(const VectorMap<String, String>& args) {
    String symbol = args.Get("symbol", "");
    if (symbol.IsEmpty()) {
        return {1, "Missing required argument: symbol"};
    }

    Vector<String> locations;
    String error;
    if (!session->FindUsages(symbol, locations, error)) {
        return {1, "Failed to find usages for symbol '" + symbol + "': " + error};
    }

    String result = "Usages found for symbol '" + symbol + "':\n";
    for (const String& loc : locations) {
        result += "  " + loc + "\n";
    }

    return {0, result};
}

// Graph command handlers
InvocationResult CommandExecutor::HandleGetBuildOrder(const VectorMap<String, String>& args) {
    Vector<String> build_order;
    String error;

    if (!session->GetBuildOrder(build_order, error)) {
        return {1, "Failed to get build order: " + error};
    }

    String result = "Build order:\n";
    for (int i = 0; i < build_order.GetCount(); i++) {
        result += IntStr(i + 1) + ". " + build_order[i] + "\n";
    }

    return {0, result};
}

InvocationResult CommandExecutor::HandleDetectCycles(const VectorMap<String, String>& args) {
    Vector<Vector<String>> cycles;
    String error;

    if (!session->FindCycles(cycles, error)) {
        return {1, "Failed to detect cycles: " + error};
    }

    if (cycles.GetCount() == 0) {
        return {0, "No cycles detected in package dependencies."};
    }

    String result = "Cycles detected in package dependencies:\n";
    for (int i = 0; i < cycles.GetCount(); i++) {
        result += "Cycle " + IntStr(i + 1) + ": ";
        for (int j = 0; j < cycles[i].GetCount(); j++) {
            if (j > 0) result += " -> ";
            result += cycles[i][j];
        }
        result += "\n";
    }

    return {0, result};
}

InvocationResult CommandExecutor::HandleAffectedPackages(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    if (path.IsEmpty()) {
        return {1, "Missing required argument: path"};
    }

    Vector<String> affected_packages;
    String error;

    if (!session->AffectedPackages(path, affected_packages, error)) {
        return {1, "Failed to get affected packages for file '" + path + "': " + error};
    }

    String result = "Packages affected by changes to '" + path + "':\n";
    for (int i = 0; i < affected_packages.GetCount(); i++) {
        result += "- " + affected_packages[i] + "\n";
    }

    return {0, result};
}

}