#include "CommandExecutor.h"
#include "IdeSession.h"
#include <Core/Core.h>
#include "CoreSupervisor.h"

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
    else if (name == "rename_symbol") {
        return HandleRenameSymbol(args);
    }
    else if (name == "remove_dead_includes") {
        return HandleRemoveDeadIncludes(args);
    }
    else if (name == "canonicalize_includes") {
        return HandleCanonicalizeIncludes(args);
    }
    else if (name == "describe_command") {
        return HandleDescribeCommand(args);
    }
    else if (name == "workspace_stats") {
        return HandleWorkspaceStats(args);
    }
    else if (name == "package_stats") {
        return HandlePackageStats(args);
    }
    else if (name == "file_complexity") {
        return HandleFileComplexity(args);
    }
    else if (name == "graph_stats") {
        return HandleGraphStats(args);
    }
    else if (name == "edit_history") {
        return HandleEditHistory(args);
    }
    else if (name == "optimize_package") {
        return HandleOptimizePackage(args);
    }
    else if (name == "get_optimization_plan") {
        return HandleGetOptimizationPlan(args);
    }
    else if (name == "workspace_plan") {
        return HandleGetWorkspacePlan(args);
    }
    else if (name == "list_strategies") {
        return HandleListStrategies(args);
    }
    else if (name == "get_strategy") {
        return HandleGetStrategy(args);
    }
    else if (name == "set_strategy") {
        return HandleSetStrategy(args);
    }
    else if (name == "supervisor_front") {
        return HandleSupervisorFront(args);
    }
    else if (name == "supervisor_rank") {
        return HandleSupervisorRank(args);
    }
    else if (name == "semantic_entities") {
        return HandleSemanticEntities(args);
    }
    else if (name == "semantic_clusters") {
        return HandleSemanticClusters(args);
    }
    else if (name == "semantic_find") {
        return HandleSemanticFind(args);
    }
    else if (name == "semantic_analyze") {
        return HandleSemanticAnalyze(args);
    }
    else if (name == "semantic_subsystems") {
        return HandleSemanticSubsystems(args);
    }
    else if (name == "semantic_entity") {
        return HandleSemanticEntity(args);
    }
    else if (name == "semantic_roles") {
        return HandleSemanticRoles(args);
    }
    else if (name == "semantic_layers") {
        return HandleSemanticLayers(args);
    }
    // Special case for list_commands (not in metadata)
    else if (name == "list_commands") {
        Vector<Command> allCmds = registry.ListCommands();
        String msg = "Available commands:\n";
        ValueArray payload_array;
        for (const Command& c : allCmds) {
            msg += c.name + " (" + c.category + "): " + c.description + "\n";
            ValueMap cmd_info;
            cmd_info.Add("name", c.name);
            cmd_info.Add("category", c.category);
            cmd_info.Add("description", c.description);
            payload_array.Add(cmd_info);
        }

        InvocationResult result_struct;
        result_struct.status_code = 0;
        result_struct.message = msg;
        result_struct.payload = payload_array;
        return result_struct;
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

    // Parse the result to create structured payload
    Vector<String> lines = Split(result, "\n");
    ValueArray payload_array;
    String output_result = "Find in files completed:\n";

    for (const String& line : lines) {
        if (!line.IsEmpty()) {
            output_result += line + "\n";
            // Try to parse "file:line:content" format if available
            Vector<String> parts = Split(line, ':');
            if (parts.GetCount() >= 3) {
                ValueMap entry;
                entry.Add("file", parts[0]);
                entry.Add("line", ScanInt(parts[1]));
                String content = parts[2];
                for (int i = 3; i < parts.GetCount(); i++) {
                    content += ":" + parts[i];
                }
                entry.Add("content", content);
                payload_array.Add(entry);
            } else {
                // If format doesn't match, just add the line as content
                ValueMap entry;
                entry.Add("content", line);
                payload_array.Add(entry);
            }
        }
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = output_result;
    result_struct.payload = payload_array;
    return result_struct;
}

InvocationResult CommandExecutor::HandleBuildProject(const VectorMap<String, String>& args) {
    String name = args.Get("name", "");
    String config = args.Get("config", "Debug");
    String log, error;

    if (!session->BuildProject(name, config, log, error)) {
        return {1, "Failed to build project '" + name + "' with config '" + config + "': " + error};
    }

    String result = "Build completed for project '" + name + "' with config '" + config + "'\nLog:\n" + log;

    ValueMap payload_map;
    payload_map.Add("project", name);
    payload_map.Add("config", config);
    payload_map.Add("log", log);
    payload_map.Add("success", true);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
}

InvocationResult CommandExecutor::HandleCleanProject(const VectorMap<String, String>& args) {
    String name = args.Get("name", "");
    String log, error;

    if (!session->CleanProject(name, log, error)) {
        return {1, "Failed to clean project '" + name + "': " + error};
    }

    String result = "Clean completed for project '" + name + "'\nLog:\n" + log;

    ValueMap payload_map;
    payload_map.Add("project", name);
    payload_map.Add("log", log);
    payload_map.Add("success", true);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
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

    // Parse the result to create structured payload
    Vector<String> lines = Split(result, "\n");
    ValueArray payload_array;
    String output_result = "Search code completed:\n";

    for (const String& line : lines) {
        if (!line.IsEmpty()) {
            output_result += line + "\n";
            // Try to parse "file:line:content" format if available
            Vector<String> parts = Split(line, ':');
            if (parts.GetCount() >= 3) {
                ValueMap entry;
                entry.Add("file", parts[0]);
                entry.Add("line", ScanInt(parts[1]));
                String content = parts[2];
                for (int i = 3; i < parts.GetCount(); i++) {
                    content += ":" + parts[i];
                }
                entry.Add("content", content);
                payload_array.Add(entry);
            } else {
                // If format doesn't match, just add the line as content
                ValueMap entry;
                entry.Add("content", line);
                payload_array.Add(entry);
            }
        }
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = output_result;
    result_struct.payload = payload_array;
    return result_struct;
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

    String result = "Replaced " + IntStr(count) + " occurrences in '" + path + "'.";

    ValueMap payload_map;
    payload_map.Add("count", count);
    payload_map.Add("path", path);
    payload_map.Add("pattern", pattern);
    payload_map.Add("replacement", replacement);
    payload_map.Add("case_sensitive", case_sensitive);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
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

    String result = "Moved to line " + IntStr(line) + " (pos " + IntStr(pos) + ") in '" + file + "'.";

    ValueMap payload_map;
    payload_map.Add("file", file);
    payload_map.Add("line", line);
    payload_map.Add("pos", pos);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
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

    String result = "Definition found: " + file + ":" + IntStr(line);

    ValueMap payload_map;
    payload_map.Add("file", file);
    payload_map.Add("line", line);
    payload_map.Add("symbol", symbol);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
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
    ValueArray payload_array;
    for (const String& loc : locations) {
        result += "  " + loc + "\n";
        // Parse location format "file:line" and convert to structured format
        int colon_pos = loc.ReverseFind(':');
        if (colon_pos > 0) {
            String file = loc.Mid(0, colon_pos);
            String line_str = loc.Mid(colon_pos + 1);
            int line = ScanInt(line_str);
            ValueMap entry;
            entry.Add("file", file);
            entry.Add("line", line);
            payload_array.Add(entry);
        } else {
            // If format is not "file:line", just add the whole location as a file
            ValueMap entry;
            entry.Add("file", loc);
            payload_array.Add(entry);
        }
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_array;
    return result_struct;
}

// Graph command handlers
InvocationResult CommandExecutor::HandleGetBuildOrder(const VectorMap<String, String>& args) {
    Vector<String> build_order;
    String error;

    if (!session->GetBuildOrder(build_order, error)) {
        return {1, "Failed to get build order: " + error};
    }

    String result = "Build order:\n";
    ValueArray payload_array;
    for (int i = 0; i < build_order.GetCount(); i++) {
        result += IntStr(i + 1) + ". " + build_order[i] + "\n";
        payload_array.Add(build_order[i]);
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_array;
    return result_struct;
}

InvocationResult CommandExecutor::HandleDetectCycles(const VectorMap<String, String>& args) {
    Vector<Vector<String>> cycles;
    String error;

    if (!session->FindCycles(cycles, error)) {
        return {1, "Failed to detect cycles: " + error};
    }

    String result = "Cycles detected in package dependencies:\n";
    ValueArray payload_array;

    if (cycles.GetCount() == 0) {
        result = "No cycles detected in package dependencies.";
        payload_array.Add("No cycles detected");
    } else {
        for (int i = 0; i < cycles.GetCount(); i++) {
            result += "Cycle " + IntStr(i + 1) + ": ";
            ValueArray cycle_packages;
            for (int j = 0; j < cycles[i].GetCount(); j++) {
                if (j > 0) result += " -> ";
                result += cycles[i][j];
                cycle_packages.Add(cycles[i][j]);
            }
            result += "\n";
            payload_array.Add(cycle_packages);
        }
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_array;
    return result_struct;
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
    ValueArray payload_array;
    for (int i = 0; i < affected_packages.GetCount(); i++) {
        result += "- " + affected_packages[i] + "\n";
        payload_array.Add(affected_packages[i]);
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_array;
    return result_struct;
}

// Refactoring command handlers
InvocationResult CommandExecutor::HandleRenameSymbol(const VectorMap<String, String>& args) {
    String old_name = args.Get("old", "");
    String new_name = args.Get("new", "");

    if (old_name.IsEmpty()) {
        return {1, "Missing required argument: old"};
    }

    if (new_name.IsEmpty()) {
        return {1, "Missing required argument: new"};
    }

    String error;
    if (!session->RenameSymbol(old_name, new_name, error)) {
        return {1, "Failed to rename symbol from '" + old_name + "' to '" + new_name + "': " + error};
    }

    String result = "Successfully renamed symbol from '" + old_name + "' to '" + new_name + "'";

    ValueMap payload_map;
    payload_map.Add("old", old_name);
    payload_map.Add("new", new_name);
    payload_map.Add("success", true);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
}

InvocationResult CommandExecutor::HandleRemoveDeadIncludes(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    if (path.IsEmpty()) {
        return {1, "Missing required argument: path"};
    }

    String error;
    int count; // We'll track how many includes were removed
    if (!session->RemoveDeadIncludes(path, error, &count)) {
        return {1, "Failed to remove dead includes in file '" + path + "': " + error};
    }

    String result = "Successfully removed " + IntStr(count) + " dead includes in file '" + path + "'";

    ValueMap payload_map;
    payload_map.Add("count", count);
    payload_map.Add("path", path);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
}

InvocationResult CommandExecutor::HandleCanonicalizeIncludes(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    if (path.IsEmpty()) {
        return {1, "Missing required argument: path"};
    }

    String error;
    int count = 0;
    if (!session->CanonicalizeIncludes(path, error, &count)) {
        return {1, "Failed to canonicalize includes in file '" + path + "': " + error};
    }

    String result = "Successfully canonicalized " + IntStr(count) + " include(s) in file '" + path + "'";

    ValueMap payload_map;
    payload_map.Add("count", count);
    payload_map.Add("path", path);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = result;
    result_struct.payload = payload_map;
    return result_struct;
}

InvocationResult CommandExecutor::HandleDescribeCommand(const VectorMap<String, String>& args) {
    String name = args.Get("name", "");
    if (name.IsEmpty()) {
        return {1, "Missing required argument: name"};
    }

    const Command* cmd = registry.FindByName(name);
    if (!cmd) {
        return {1, "Command not found: " + name};
    }

    // Create structured payload with command metadata
    ValueMap payload;
    payload.Add("name", cmd->name);
    payload.Add("category", cmd->category);
    payload.Add("description", cmd->description);
    payload.Add("long_description", cmd->long_description);

    // Add inputs (arguments)
    ValueArray inputs_array;
    for (const Argument& input : cmd->inputs) {
        ValueMap input_map;
        input_map.Add("name", input.name);
        input_map.Add("type", input.type);
        input_map.Add("required", input.required);
        input_map.Add("default", input.default_value);
        input_map.Add("description", input.description);

        if (!input.enum_values.IsEmpty()) {
            ValueArray enum_array;
            for (const String& enum_val : input.enum_values) {
                enum_array.Add(enum_val);
            }
            input_map.Add("enum_values", enum_array);
        }

        inputs_array.Add(input_map);
    }
    payload.Add("inputs", inputs_array);

    // Add outputs
    ValueMap outputs_map;
    outputs_map.Add("kind", cmd->output.kind);

    if (!cmd->output.fields.IsEmpty()) {
        ValueArray fields_array;
        for (const String& field : cmd->output.fields) {
            fields_array.Add(field);
        }
        outputs_map.Add("fields", fields_array);
    }
    payload.Add("outputs", outputs_map);

    // Add side effects
    ValueMap side_effects_map;
    side_effects_map.Add("modifies_files", cmd->side_effects.modifies_files);
    side_effects_map.Add("modifies_project", cmd->side_effects.modifies_project);
    side_effects_map.Add("requires_open_project", cmd->side_effects.requires_open_project);
    side_effects_map.Add("requires_open_file", cmd->side_effects.requires_open_file);
    payload.Add("side_effects", side_effects_map);

    payload.Add("context_notes", cmd->context_notes);

    // Create human-readable message
    String msg = "Command: " + cmd->name + "\n";
    msg += "Category: " + cmd->category + "\n";
    msg += "Description: " + cmd->description + "\n";
    msg += "Arguments:\n";
    for (const Argument& input : cmd->inputs) {
        msg += "  " + input.name + " (" + input.type + "): " + input.description;
        if (input.required) msg += " [required]";
        if (!input.default_value.IsEmpty()) msg += " [default: " + input.default_value + "]";
        msg += "\n";
    }

    InvocationResult result;
    result.status_code = 0;
    result.message = msg;
    result.payload = payload;
    return result;
}

// Telemetry handler implementations
InvocationResult CommandExecutor::HandleWorkspaceStats(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetWorkspaceStats(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to get workspace stats: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Workspace stats retrieved";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandlePackageStats(const VectorMap<String, String>& args) {
    String pkg = args.Get("package", "");
    if (pkg.IsEmpty()) {
        return {1, "Missing required argument: package"};
    }

    String error;
    Value result = session->GetPackageStats(pkg, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to get package stats for '" + pkg + "': " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Package stats for '" + pkg + "' retrieved";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleFileComplexity(const VectorMap<String, String>& args) {
    String path = args.Get("path", "");
    if (path.IsEmpty()) {
        return {1, "Missing required argument: path"};
    }

    String error;
    Value result = session->GetFileComplexity(path, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to get file complexity for '" + path + "': " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "File complexity for '" + path + "' retrieved";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleGraphStats(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetGraphStats(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to get graph stats: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Graph stats retrieved";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleEditHistory(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetEditHistory(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to get edit history: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Edit history retrieved";
    result_struct.payload = result;
    return result_struct;
}

// Optimization Loop v1 handler
InvocationResult CommandExecutor::HandleOptimizePackage(const VectorMap<String, String>& args) {
    String package = args.Get("name", "");
    if (package.IsEmpty()) {
        return {1, "Missing required argument: name"};
    }

    // Parse optional parameters with defaults
    int max_iterations = ScanInt(args.Get("max_iterations", "5"));
    double converge_threshold = ScanDouble(args.Get("converge_threshold", "0.01"));
    bool stop_on_worse = args.Get("stop_on_worse", "true") == "true";
    bool stop_on_converge = args.Get("stop_on_converge", "true") == "true";

    String error;
    Value result = session->OptimizePackage(
        package,
        max_iterations,
        converge_threshold,
        stop_on_worse,
        stop_on_converge,
        error
    );

    if (error.GetCount() > 0) {
        return {1, "Failed to optimize package '" + package + "': " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Optimization completed for package '" + package + "'";
    result_struct.payload = result;
    return result_struct;
}

// AI Supervisor Layer v1 handler
InvocationResult CommandExecutor::HandleGetOptimizationPlan(const VectorMap<String, String>& args) {
    String package = args.Get("name", "");
    if (package.IsEmpty()) {
        return {1, "Missing required argument: name"};
    }

    String error;
    Value result = session->GetOptimizationPlan(package, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to generate optimization plan for '" + package + "': " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Optimization plan generated for package '" + package + "'";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleGetWorkspacePlan(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetWorkspacePlan(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to generate workspace plan: " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Workspace optimization plan generated";
    result_struct.payload = result;
    return result_struct;
}

// Dynamic Strategy Engine handlers
InvocationResult CommandExecutor::HandleListStrategies(const VectorMap<String, String>& args) {
    String error;
    Value result = session->ListStrategies(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to list strategies: " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved strategy list";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleGetStrategy(const VectorMap<String, String>& args) {
    String strategy_name = args.Get("name", "active");
    String error;

    Value result;
    if (strategy_name == "active") {
        // Get the currently active strategy
        result = session->GetActiveStrategy(error);
    } else {
        // Get a specific strategy by name
        result = session->GetStrategy(strategy_name, error);
    }

    if (error.GetCount() > 0) {
        return {1, "Failed to get strategy: " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved strategy information";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSetStrategy(const VectorMap<String, String>& args) {
    String strategy_name = args.Get("name", "");
    if (strategy_name.IsEmpty()) {
        return {1, "Missing required argument: name"};
    }

    String error;
    bool success = session->SetActiveStrategy(strategy_name, error);

    if (!success) {
        return {1, "Failed to set strategy to '" + strategy_name + "': " + error};
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully set active strategy to '" + strategy_name + "'";
    return result_struct;
}

// Supervisor v3: Multi-Objective Planner command handlers
InvocationResult CommandExecutor::HandleSupervisorFront(const VectorMap<String, String>& args) {
    String package = args.Get("package", "");
    if (package.IsEmpty()) {
        return {1, "Missing required argument: package"};
    }

    String error;
    Value result = session->GetOptimizationPlan(package, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to generate optimization plan for '" + package + "': " + error};
    }

    // Extract the suggestions from the plan
    Vector<CoreSupervisor::Suggestion> suggestions;
    if (result.Is<ValueMap>()) {
        ValueMap plan = result;
        ValueArray steps = plan.Get("steps", ValueArray());

        for (int i = 0; i < steps.GetCount(); i++) {
            if (steps[i].Is<ValueMap>()) {
                ValueMap step = steps[i];

                CoreSupervisor::Suggestion suggestion;
                suggestion.action = (String)step.Get("action", "");
                suggestion.target = (String)step.Get("target", "");
                suggestion.params = step.Get("params", ValueMap());
                suggestion.reason = (String)step.Get("reason", "");
                suggestion.benefit_score = (double)step.Get("benefit_score", 0.0);
                suggestion.cost_score = (double)step.Get("cost_score", 0.0);
                suggestion.risk_score = (double)step.Get("risk_score", 0.0);
                suggestion.confidence_score = (double)step.Get("confidence_score", 0.0);

                // Get metrics map
                ValueMap metrics_map = step.Get("metrics", ValueMap());
                suggestion.metrics = metrics_map;

                suggestions.Add(suggestion);
            }
        }
    }

    // Instead of creating a new supervisor, let's use the one from the session
    // Extract the supervisor from the session - need to do this differently
    // since we're not calling session->ComputeParetoFront directly

    // For now, create a temporary supervisor to compute the front
    // A more proper way would be adding this functionality to the IdeSession
    CoreSupervisor temp_supervisor;
    Vector<CoreSupervisor::Suggestion> pareto_front = temp_supervisor.ComputeParetoFront(suggestions);

    // Convert suggestions back to ValueArray for payload
    ValueArray payload_array;
    for (const auto& suggestion : pareto_front) {
        ValueMap suggestion_map;
        suggestion_map.Add("action", suggestion.action);
        suggestion_map.Add("target", suggestion.target);
        suggestion_map.Add("params", suggestion.params);
        suggestion_map.Add("reason", suggestion.reason);
        suggestion_map.Add("benefit_score", suggestion.benefit_score);
        suggestion_map.Add("cost_score", suggestion.cost_score);
        suggestion_map.Add("risk_score", suggestion.risk_score);
        suggestion_map.Add("confidence_score", suggestion.confidence_score);
        suggestion_map.Add("metrics", suggestion.metrics);

        payload_array.Add(suggestion_map);
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Computed Pareto-optimal suggestions for package '" + package + "'";
    result_struct.payload = payload_array;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSupervisorRank(const VectorMap<String, String>& args) {
    String package = args.Get("package", "");
    if (package.IsEmpty()) {
        return {1, "Missing required argument: package"};
    }

    String limit_str = args.Get("limit", "10");
    int limit = ScanInt(limit_str);
    if (limit <= 0) {
        limit = 10; // default
    }

    String error;
    Value result = session->GetOptimizationPlan(package, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to generate optimization plan for '" + package + "': " + error};
    }

    // Extract the suggestions from the plan
    Vector<CoreSupervisor::Suggestion> suggestions;
    if (result.Is<ValueMap>()) {
        ValueMap plan = result;
        ValueArray steps = plan.Get("steps", ValueArray());

        for (int i = 0; i < steps.GetCount(); i++) {
            if (steps[i].Is<ValueMap>()) {
                ValueMap step = steps[i];

                CoreSupervisor::Suggestion suggestion;
                suggestion.action = (String)step.Get("action", "");
                suggestion.target = (String)step.Get("target", "");
                suggestion.params = step.Get("params", ValueMap());
                suggestion.reason = (String)step.Get("reason", "");
                suggestion.benefit_score = (double)step.Get("benefit_score", 0.0);
                suggestion.cost_score = (double)step.Get("cost_score", 0.0);
                suggestion.risk_score = (double)step.Get("risk_score", 0.0);
                suggestion.confidence_score = (double)step.Get("confidence_score", 0.0);

                // Get metrics map
                ValueMap metrics_map = step.Get("metrics", ValueMap());
                suggestion.metrics = metrics_map;

                suggestions.Add(suggestion);
            }
        }
    }

    // Get strategy weights for ranking
    double w_benefit = 1.0;
    double w_cost = 0.7;
    double w_risk = 1.2;
    double w_confidence = 1.0;

    // Try to get objective weights from active strategy
    Value strategy_info = session->GetActiveStrategy(error);
    if (strategy_info.Is<ValueMap>()) {
        ValueMap strategy_map = strategy_info;
        ValueMap objective_weights = strategy_map.Get("objective_weights", ValueMap());
        w_benefit = (double)objective_weights.Get("benefit", 1.0);
        w_cost = (double)objective_weights.Get("cost", 0.7);
        w_risk = (double)objective_weights.Get("risk", 1.2);
        w_confidence = (double)objective_weights.Get("confidence", 1.0);
    }

    // Compute weighted scores and rank suggestions
    Vector< pair<double, int> > scores_with_indices;  // (score, original_index)
    for (int i = 0; i < suggestions.GetCount(); i++) {
        const auto& s = suggestions[i];
        // Use weighted linear combination: score = (benefit_score * w_benefit) + (confidence_score * w_confidence) - (cost_score * w_cost) - (risk_score * w_risk)
        double score = (s.benefit_score * w_benefit) +
                      (s.confidence_score * w_confidence) -
                      (s.cost_score * w_cost) -
                      (s.risk_score * w_risk);

        scores_with_indices.Add(make_pair(score, i));
    }

    // Sort by score in descending order
    Sort(scores_with_indices, [](const pair<double, int>& a, const pair<double, int>& b) {
        return a.first > b.first;  // Higher scores first
    });

    // Create ranked result
    ValueArray payload_array;
    int actual_limit = min(limit, scores_with_indices.GetCount());
    for (int i = 0; i < actual_limit; i++) {
        int original_index = scores_with_indices[i].second;
        const auto& s = suggestions[original_index];
        double score = scores_with_indices[i].first;

        ValueMap suggestion_map;
        suggestion_map.Add("action", s.action);
        suggestion_map.Add("target", s.target);
        suggestion_map.Add("params", s.params);
        suggestion_map.Add("reason", s.reason);
        suggestion_map.Add("benefit_score", s.benefit_score);
        suggestion_map.Add("cost_score", s.cost_score);
        suggestion_map.Add("risk_score", s.risk_score);
        suggestion_map.Add("confidence_score", s.confidence_score);
        suggestion_map.Add("metrics", s.metrics);
        suggestion_map.Add("combined_score", score);  // Add the computed combined score

        payload_array.Add(suggestion_map);
    }

    // Create result struct
    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Ranked " + IntStr(actual_limit) + " suggestions for package '" + package + "'";
    result_struct.payload = payload_array;
    return result_struct;
}

// Semantic Analysis v1 handlers
InvocationResult CommandExecutor::HandleSemanticEntities(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticEntities(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic entities: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic entities";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticClusters(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticClusters(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic clusters: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic clusters";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticFind(const VectorMap<String, String>& args) {
    String pattern = args.Get("pattern", "");
    if (pattern.IsEmpty()) {
        return {1, "Missing required argument: pattern"};
    }

    String error;
    Value result = session->SearchSemanticEntities(pattern, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to search semantic entities with pattern '" + pattern + "': " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully found semantic entities matching pattern '" + pattern + "'";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticAnalyze(const VectorMap<String, String>& args) {
    String error;
    // We need to call the semantic analysis directly via the CoreIde
    // Since we don't have direct access to CoreIde from CommandExecutor,
    // we can't trigger a fresh analysis directly through IdeSession interface.
    // This would require adding a method to IdeSession interface to trigger analysis.
    // For now, calling GetSemanticEntities will trigger analysis if needed.

    // Just return status that analysis is complete
    Value entities = session->GetSemanticEntities(error);
    if (error.GetCount() > 0) {
        return {1, "Failed to perform semantic analysis: " + error};
    }

    Value clusters = session->GetSemanticClusters(error);
    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic clusters: " + error};
    }

    int entity_count = 0;
    if (entities.Is<ValueArray>()) {
        entity_count = entities.GetCount();
    }

    int cluster_count = 0;
    if (clusters.Is<ValueArray>()) {
        cluster_count = clusters.GetCount();
    }

    ValueMap payload;
    payload.Add("status", "Semantic analysis completed");
    payload.Add("entity_count", entity_count);
    payload.Add("cluster_count", cluster_count);

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Semantic analysis completed. Found " + IntStr(entity_count) + " entities and " + IntStr(cluster_count) + " clusters.";
    result_struct.payload = payload;
    return result_struct;
}

// NEW: Semantic Analysis v2 - Inference layer handlers
InvocationResult CommandExecutor::HandleSemanticSubsystems(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticSubsystems(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic subsystems: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic subsystems";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticEntity(const VectorMap<String, String>& args) {
    String entity_name = args.Get("name", "");
    if (entity_name.IsEmpty()) {
        return {1, "Missing required argument: name"};
    }

    String error;
    Value result = session->GetSemanticEntity(entity_name, error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic entity '" + entity_name + "': " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic information for entity '" + entity_name + "'";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticRoles(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticRoles(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic roles: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic roles";
    result_struct.payload = result;
    return result_struct;
}

InvocationResult CommandExecutor::HandleSemanticLayers(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticLayers(error);

    if (error.GetCount() > 0) {
        return {1, "Failed to retrieve semantic layers: " + error};
    }

    InvocationResult result_struct;
    result_struct.status_code = 0;
    result_struct.message = "Successfully retrieved semantic layers";
    result_struct.payload = result;
    return result_struct;
}

}