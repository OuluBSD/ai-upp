#include "clicore.h"

using namespace Upp;

CoreIde::CoreIde() {
    // Initialize internal state
    current_editor_index = -1;

    // Initialize supervisor with strategy registry
    supervisor.SetStrategyRegistry(&strategy_registry);

    // Set default strategy (gracefully handle loading failure)
    String error;
    if (!InitializeStrategies("metadata/strategies_v1.json", error)) {
        // If we can't load strategy file, try with a default fallback
        // The supervisor will use built-in default weights
        supervisor.SetActiveStrategy("default", error);
    } else {
        // Set default strategy if initialization succeeds
        supervisor.SetActiveStrategy("default", error);
    }

    // Initialize ProjectMemory - load from default location if it exists
    String memory_file_path = GetWorkspaceRoot() + "/.aiupp/project_memory.json";
    memory.Load(memory_file_path);

    // Initialize GlobalKnowledge - load from user-level location
    String global_knowledge_path = GetHomeDirectory() + "/.aiupp/global_knowledge.json";
    global.Load(global_knowledge_path);

    // Load and register agent profiles from metadata
    if (!agent_registry.Load("metadata/agents_v1.json", error)) {
        // If we can't load agent file, log but don't fail
        console.AppendLine("Warning: Failed to load agent profiles: " + error);
        console.AppendLine("Using default empty agent configuration");
    } else {
        // Register all loaded agent profiles with the StrategicNavigator
        Vector<AgentProfile> profiles = agent_registry.GetAll();
        for(const AgentProfile& profile : profiles) {
            navigator.RegisterAgent(profile);
        }
    }

    // Initialize CoreEvolution - load from workspace-specific location
    String evolution_path = workspace_root + "/.aiupp/evolution.json";
    evolution.Load(evolution_path);

    // Learn from evolution history to adjust strategy weights
    EvolutionSummary summary = evolution.Summarize();
    supervisor.LearnFromEvolution(summary);

    // Initialize Playbook Engine - load from default location if it exists
    String playbook_error;
    if (!playbook_engine.Load("metadata/playbooks_v1.json", playbook_error)) {
        // If we can't load playbooks file, log but don't fail
        console.AppendLine("Warning: Failed to load playbooks: " + playbook_error);
        console.AppendLine("Using default empty playbooks configuration");
    }
}

CoreIde::~CoreIde() {
    // Clean up resources
}

// File operations
bool CoreIde::OpenFile(const String& path, String& error) {
    return OpenEditor(path, error) != nullptr;
}

bool CoreIde::SaveFile(const String& path, String& error) {
    if (path.IsEmpty()) {
        // Save the current editor if no path is specified
        if (current_editor_index >= 0 && current_editor_index < editors.GetCount()) {
            CoreEditor& editor = editors[current_editor_index];
            String old_content = editor.GetContent();
            int old_size = old_content.GetLength();
            bool success = editor.SaveFile(error);
            if (success) {
                String new_content = LoadFile(path);
                int new_size = new_content.GetLength();
                telemetry.RecordEdit(path, old_size, new_size);
            }
            return success;
        } else {
            error = "No file path specified and no current editor available";
            return false;
        }
    }

    // Find the editor for the specified path
    int index = editor_paths.Find(path);
    if (index >= 0) {
        // Editor exists, save it
        CoreEditor& editor = editors[index];
        String old_content = editor.GetContent();
        int old_size = old_content.GetLength();
        bool success = editor.SaveFile(error);
        if (success) {
            String new_content = LoadFile(path);
            int new_size = new_content.GetLength();
            telemetry.RecordEdit(path, old_size, new_size);
        }
        return success;
    } else {
        // Editor doesn't exist, so save the original file directly
        String content;
        if (FileExists(path)) {
            content = LoadFile(path);
            int old_size = content.GetLength();
            bool success = fileOps.SaveFile(path, content, error);
            if (success) {
                String new_content = LoadFile(path);
                int new_size = new_content.GetLength();
                telemetry.RecordEdit(path, old_size, new_size);
            }
            return success;
        }
        return fileOps.SaveFile(path, content, error);
    }
}

// Editor operations
CoreEditor* CoreIde::FindEditorForPath(const String& path) {
    int index = editor_paths.Find(path);
    if (index >= 0 && index < editors.GetCount()) {
        return &editors[index];
    }
    return nullptr;
}

CoreEditor* CoreIde::GetCurrentEditor() {
    if (current_editor_index >= 0 && current_editor_index < editors.GetCount()) {
        return &editors[current_editor_index];
    }
    return nullptr;
}

CoreEditor* CoreIde::OpenEditor(const String& path, String& error) {
    // Check if editor for this path already exists
    int index = editor_paths.Find(path);
    if (index >= 0) {
        // Editor already exists, set it as current and return it
        current_editor_index = index;
        return &editors[index];
    }

    // Editor doesn't exist, create a new one
    CoreEditor new_editor;
    if (!new_editor.LoadFile(path, error)) {
        return nullptr;  // Error already set by LoadFile
    }

    // Add the new editor
    int new_index = editors.GetCount();
    editors.Add(pick(new_editor)); // Proper One<T> creation
    editor_paths.Add(path);

    // Set as current editor
    current_editor_index = new_index;

    return &editors[new_index];
}

bool CoreIde::CloseFile(const String& path, String& error) {
    int index = editor_paths.Find(path);
    if (index < 0) {
        error = "File not open: " + path;
        return false;
    }

    // Remove the editor
    editors.Remove(index);
    editor_paths.Remove(index);

    // Update current_editor_index if needed
    if (current_editor_index == index) {
        current_editor_index = -1;  // No current editor
    } else if (current_editor_index > index) {
        current_editor_index--;  // Adjust index after removal
    }

    return true;
}

// Editor-specific operations
bool CoreIde::EditorInsert(const String& path, int pos, const String& text, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    // Record edit for telemetry (before modification)
    String old_content = editor->GetContent();
    int old_size = old_content.GetLength();
    bool success = editor->Insert(pos, text);
    if (success) {
        String new_content = editor->GetContent();
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);
    }
    return success;
}

bool CoreIde::EditorErase(const String& path, int pos, int count, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    // Record edit for telemetry (before modification)
    String old_content = editor->GetContent();
    int old_size = old_content.GetLength();
    bool success = editor->Erase(pos, count);
    if (success) {
        String new_content = editor->GetContent();
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);
    }
    return success;
}

bool CoreIde::EditorReplace(const String& path, int pos, int count, const String& replacement, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    // Record edit for telemetry (before modification)
    String old_content = editor->GetContent();
    int old_size = old_content.GetLength();
    bool success = editor->Replace(pos, count, replacement);
    if (success) {
        String new_content = editor->GetContent();
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);
    }
    return success;
}

bool CoreIde::EditorGotoLine(const String& path, int line, int& out_pos, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->GotoLine(line, out_pos);
}

bool CoreIde::EditorFindFirst(const String& path, const String& pattern, int start_pos,
                              bool case_sensitive, int& out_pos, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->FindFirst(pattern, start_pos, case_sensitive, out_pos);
}

bool CoreIde::EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                               bool case_sensitive, int& out_count, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->ReplaceAll(pattern, replacement, case_sensitive, out_count);
}

bool CoreIde::EditorUndo(const String& path, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Undo();
}

bool CoreIde::EditorRedo(const String& path, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Redo();
}

// Workspace management
bool CoreIde::SetWorkspaceRoot(const String& root, String& error) {
    if (!workspace.SetWorkspaceRoot(root, error)) {
        return false;
    }

    workspace_root = root;

    // Load ProjectMemory from workspace-specific location
    String memory_file_path = workspace_root + "/.aiupp/project_memory.json";
    memory.Load(memory_file_path);

    // Rebuild graph after workspace root changes
    String graph_error;
    if (!RebuildGraph(graph_error)) {
        // Log the error but don't fail the SetWorkspaceRoot operation
        console.AppendLine("Warning: Failed to rebuild dependency graph: " + graph_error);
    }

    return true;
}

const String& CoreIde::GetWorkspaceRoot() const {
    return workspace_root;
}

// Project/package operations
bool CoreIde::SetMainPackage(const String& package, String& error) {
    // First ensure workspace root is set
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use SetWorkspaceRoot first.";
        return false;
    }

    // Use the workspace helper to set the main package
    bool result = workspace.SetMainPackage(package, error);

    // Rebuild graph after setting main package
    if (result) {
        String graph_error;
        if (!RebuildGraph(graph_error)) {
            // Log the error but don't fail the SetMainPackage operation
            console.AppendLine("Warning: Failed to rebuild dependency graph: " + graph_error);
        }
    }

    return result;
}

bool CoreIde::GetMainPackage(CoreWorkspace::Package& out, String& error) const {
    return workspace.GetMainPackage(out, error);
}

// Build operations
bool CoreIde::BuildProject(const String& project, const String& config, String& log, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    // If no project specified, use main package
    if (project.IsEmpty() && !workspace.GetMainPackage().IsEmpty()) {
        CoreWorkspace::Package main_pkg;
        if (!workspace.GetMainPackage(main_pkg, error)) {
            return false;
        }
        return build.BuildProject(main_pkg, config, log, error);
    } else if (!project.IsEmpty()) {
        // If project is specified, load it and build
        CoreWorkspace::Package pkg;
        if (!workspace.LoadPackage(project, error)) {
            return false;
        }

        // Try to get the loaded package
        const CoreWorkspace::Package* loaded_pkg = workspace.GetPackage(project);
        if (!loaded_pkg) {
            error = "Could not load package: " + project;
            return false;
        }

        return build.BuildProject(*loaded_pkg, config, log, error);
    } else {
        error = "No project specified and no main package set.";
        return false;
    }
}

bool CoreIde::CleanProject(const String& project, String& log, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    // If no project specified, use main package
    if (project.IsEmpty() && !workspace.GetMainPackage().IsEmpty()) {
        CoreWorkspace::Package main_pkg;
        if (!workspace.GetMainPackage(main_pkg, error)) {
            return false;
        }
        return build.CleanProject(main_pkg, log, error);
    } else if (!project.IsEmpty()) {
        // If project is specified, load it and clean
        CoreWorkspace::Package pkg;
        if (!workspace.LoadPackage(project, error)) {
            return false;
        }

        // Try to get the loaded package
        const CoreWorkspace::Package* loaded_pkg = workspace.GetPackage(project);
        if (!loaded_pkg) {
            error = "Could not load package: " + project;
            return false;
        }

        return build.CleanProject(*loaded_pkg, log, error);
    } else {
        error = "No project specified and no main package set.";
        return false;
    }
}

// Navigation
bool CoreIde::GotoLine(const String& file, int line, String& error) {
    // TODO: Implement line navigation logic
    // For now, just verify the file exists and the line number is valid
    if (!FileExists(file)) {
        error = "File does not exist: " + file;
        return false;
    }

    if (line <= 0) {
        error = "Invalid line number: " + AsString(line);
        return false;
    }

    // In the future, this would navigate to the specific line in an internal editor
    return true;
}

// Search / code search
bool CoreIde::FindInFiles(const String& pattern, const String& directory, bool replace, String& result, String& error) {
    // If no directory is specified, use workspace root or main package directory
    String search_dir = directory;
    if (search_dir.IsEmpty()) {
        if (!workspace.GetMainPackage().IsEmpty()) {
            CoreWorkspace::Package main_pkg;
            if (!workspace.GetMainPackage(main_pkg, error)) {
                // If we can't get the main package, fall back to workspace root
                search_dir = workspace.GetWorkspaceRoot();
            } else {
                search_dir = main_pkg.path;
            }
        } else if (!workspace.GetWorkspaceRoot().IsEmpty()) {
            search_dir = workspace.GetWorkspaceRoot();
        } else {
            error = "No search directory specified and workspace context not established.";
            return false;
        }
    }

    // Delegate to the search helper
    Vector<String> results;
    bool success = search.FindInFiles(pattern, search_dir, replace, "", results, error);

    // Combine results into a single string
    for (const String& res : results) {
        result += res + "\n";
    }

    return success;
}

bool CoreIde::SearchCode(const String& query, String& result, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    if (workspace.GetMainPackage().IsEmpty()) {
        error = "No main package set. Use SetMainPackage first.";
        return false;
    }

    // Get the main package and search in its files
    CoreWorkspace::Package main_pkg;
    if (!workspace.GetMainPackage(main_pkg, error)) {
        return false;
    }

    // Use the workspace-aware search method
    Vector<String> results;
    bool success = search.SearchCodeInPackage(query, main_pkg, results, error);

    // Combine results into a single string
    for (const String& res : results) {
        result += res + "\n";
    }

    return success;
}

// Symbol assistance
bool CoreIde::IndexWorkspace(String& error) {
    return assist.IndexWorkspace(workspace, error);
}

bool CoreIde::FindSymbolDefinition(const String& symbol, String& file, int& line, String& error) {
    return assist.FindDefinition(symbol, file, line);
}

bool CoreIde::FindSymbolUsages(const String& symbol, Vector<String>& locs, String& error) {
    return assist.FindUsages(symbol, locs);
}

// Output
bool CoreIde::GetConsoleOutput(String& out, String& error) {
    // Delegate to the console helper
    out = console.GetConsoleOutput();
    return true; // This operation should always succeed
}

bool CoreIde::GetErrorsOutput(String& out, String& error) {
    // Delegate to the console helper
    out = console.GetErrorsOutput();
    return true; // This operation should always succeed
}

// Graph operations
bool CoreIde::RebuildGraph(String& error) {
    return graph.BuildPackageGraph(workspace, error);
}

bool CoreIde::GetBuildOrder(Vector<String>& out_order, String& error) {
    return graph.TopologicalSort(out_order, error);
}

bool CoreIde::GetCycles(Vector<Vector<String>>& out_cycles, String& error) {
    return graph.DetectCycles(out_cycles);
}

bool CoreIde::GetAffectedPackages(const String& filepath, Vector<String>& out_packages, String& error) {
    return graph.AffectedPackagesByFile(filepath, workspace, out_packages);
}

// Refactoring operations
bool CoreIde::RenameSymbol(const String& old, const String& nw, String& error) {
    // Collect metrics before rename
    ValueMap metrics_before = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return false; // Return early on error
    }

    // For now, we'll implement a simpler approach for recording the edit
    // In a complete implementation, this would track all affected files during rename
    bool success = refactor.RenameSymbol(old, nw, *this, error);

    if (success) {
        // Collect metrics after rename
        ValueMap metrics_after = GetTelemetryData("", error);
        if (!error.IsEmpty()) {
            return false; // Return early on error
        }

        // Create evolution event to record the change
        EvolutionEvent ev;
        ev.timestamp = GetSysTime();
        ev.id = AsString(GetSysTime().GetTickCount(), "%016x"); // Generate a unique ID
        ev.package = workspace.GetMainPackage().IsEmpty() ? "unknown" : workspace.GetMainPackage();
        ev.agent_name = "unknown"; // Would be set by caller
        ev.scenario_id = "direct_call"; // Direct call rather than through scenario
        ev.lifecycle_phase = GetCurrentLifecyclePhase().name; // Get current lifecycle phase
        ev.strategy = GetActiveStrategy() ? GetActiveStrategy()->name : "default"; // Get active strategy name
        ev.context.Set("stability", GetLifecycleStability(error));
        if (error.IsEmpty()) {
            ev.context.Set("phase", GetCurrentLifecyclePhase().name);
        }

        ev.change_kinds.Add("rename");
        ev.metrics_before = metrics_before;
        ev.metrics_after = metrics_after;

        // Calculate deltas
        for (int i = 0; i < metrics_before.GetCount(); i++) {
            String key = metrics_before.GetKey(i);
            Value b_val = metrics_before[i];
            if (metrics_after.Find(key) >= 0) {
                Value a_val = metrics_after[key];

                if (Is<double>(b_val) && Is<double>(a_val)) {
                    ev.deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if (Is<int>(b_val) && Is<int>(a_val)) {
                    ev.deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    ev.deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
                }
            }
        }

        ev.succeeded = success;
        ev.reverted = false; // This will be set to true if the change is later reverted

        // Record the event in the evolution engine
        String evolution_path = workspace_root + "/.aiupp/evolution.json";
        evolution.Load(evolution_path); // Load existing events
        evolution.RecordEvent(ev); // Record the new event
        evolution.Save(evolution_path); // Save back to file
    }

    // Note: In a complete implementation, we would need to get the list of affected files from the refactor operation
    // and record the before/after sizes for each file
    return success;
}

bool CoreIde::RemoveDeadIncludes(const String& path, String& error, int* out_count) {
    // Collect metrics before removing dead includes
    ValueMap metrics_before = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return false; // Return early on error
    }

    String old_content = LoadFile(path);
    int old_size = old_content.GetLength();
    bool success = refactor.RemoveDeadIncludes(path, *this, error, out_count);
    if (success) {
        String new_content = LoadFile(path);
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);

        // Collect metrics after removing dead includes
        ValueMap metrics_after = GetTelemetryData("", error);
        if (!error.IsEmpty()) {
            return false; // Return early on error
        }

        // Create evolution event to record the change
        EvolutionEvent ev;
        ev.timestamp = GetSysTime();
        ev.id = AsString(GetSysTime().GetTickCount(), "%016x"); // Generate a unique ID
        ev.package = workspace.GetMainPackage().IsEmpty() ? "unknown" : workspace.GetMainPackage();
        ev.agent_name = "unknown"; // Would be set by caller
        ev.scenario_id = "direct_call"; // Direct call rather than through scenario
        ev.lifecycle_phase = GetCurrentLifecyclePhase().name; // Get current lifecycle phase
        ev.strategy = GetActiveStrategy() ? GetActiveStrategy()->name : "default"; // Get active strategy name
        ev.context.Set("stability", GetLifecycleStability(error));
        if (error.IsEmpty()) {
            ev.context.Set("phase", GetCurrentLifecyclePhase().name);
        }

        ev.change_kinds.Add("include_cleanup");
        ev.metrics_before = metrics_before;
        ev.metrics_after = metrics_after;

        // Calculate deltas
        for (int i = 0; i < metrics_before.GetCount(); i++) {
            String key = metrics_before.GetKey(i);
            Value b_val = metrics_before[i];
            if (metrics_after.Find(key) >= 0) {
                Value a_val = metrics_after[key];

                if (Is<double>(b_val) && Is<double>(a_val)) {
                    ev.deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if (Is<int>(b_val) && Is<int>(a_val)) {
                    ev.deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    ev.deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
                }
            }
        }

        ev.succeeded = success;
        ev.reverted = false; // This will be set to true if the change is later reverted

        // Record the event in the evolution engine
        String evolution_path = workspace_root + "/.aiupp/evolution.json";
        evolution.Load(evolution_path); // Load existing events
        evolution.RecordEvent(ev); // Record the new event
        evolution.Save(evolution_path); // Save back to file
    }
    return success;
}

bool CoreIde::CanonicalizeIncludes(const String& path, String& error, int* out_count) {
    // Collect metrics before canonicalizing includes
    ValueMap metrics_before = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return false; // Return early on error
    }

    String old_content = LoadFile(path);
    int old_size = old_content.GetLength();
    bool success = refactor.CanonicalizeIncludes(path, *this, error, out_count);
    if (success) {
        String new_content = LoadFile(path);
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);

        // Collect metrics after canonicalizing includes
        ValueMap metrics_after = GetTelemetryData("", error);
        if (!error.IsEmpty()) {
            return false; // Return early on error
        }

        // Create evolution event to record the change
        EvolutionEvent ev;
        ev.timestamp = GetSysTime();
        ev.id = AsString(GetSysTime().GetTickCount(), "%016x"); // Generate a unique ID
        ev.package = workspace.GetMainPackage().IsEmpty() ? "unknown" : workspace.GetMainPackage();
        ev.agent_name = "unknown"; // Would be set by caller
        ev.scenario_id = "direct_call"; // Direct call rather than through scenario
        ev.lifecycle_phase = GetCurrentLifecyclePhase().name; // Get current lifecycle phase
        ev.strategy = GetActiveStrategy() ? GetActiveStrategy()->name : "default"; // Get active strategy name
        ev.context.Set("stability", GetLifecycleStability(error));
        if (error.IsEmpty()) {
            ev.context.Set("phase", GetCurrentLifecyclePhase().name);
        }

        ev.change_kinds.Add("include_normalization");
        ev.metrics_before = metrics_before;
        ev.metrics_after = metrics_after;

        // Calculate deltas
        for (int i = 0; i < metrics_before.GetCount(); i++) {
            String key = metrics_before.GetKey(i);
            Value b_val = metrics_before[i];
            if (metrics_after.Find(key) >= 0) {
                Value a_val = metrics_after[key];

                if (Is<double>(b_val) && Is<double>(a_val)) {
                    ev.deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if (Is<int>(b_val) && Is<int>(a_val)) {
                    ev.deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    ev.deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
                }
            }
        }

        ev.succeeded = success;
        ev.reverted = false; // This will be set to true if the change is later reverted

        // Record the event in the evolution engine
        String evolution_path = workspace_root + "/.aiupp/evolution.json";
        evolution.Load(evolution_path); // Load existing events
        evolution.RecordEvent(ev); // Record the new event
        evolution.Save(evolution_path); // Save back to file
    }
    return success;
}

// Telemetry & Analytics v1
Value CoreIde::GetWorkspaceStats() {
    return telemetry.GetWorkspaceStats(workspace);
}

Value CoreIde::GetPackageStats(const String& pkg) {
    const CoreWorkspace::Package* pkg_ptr = workspace.GetPackage(pkg);
    if (pkg_ptr) {
        return telemetry.GetPackageStats(*pkg_ptr);
    }
    return Value();
}

Value CoreIde::GetPackageStats(const String& pkg, String& error) {
    const CoreWorkspace::Package* pkg_ptr = workspace.GetPackage(pkg);
    if (!pkg_ptr) {
        error = "Package not found: " + pkg;
        return Value();
    }
    try {
        return telemetry.GetPackageStats(*pkg_ptr);
    } catch (const Exc& e) {
        error = e;
        return Value();
    }
}

Value CoreIde::GetTelemetryData(const String& pkg, String& error) {
    try {
        // For now, return the same as package stats - in a real implementation,
        // this would return additional telemetry information
        return GetPackageStats(pkg, error);
    } catch (const Exc& e) {
        error = e;
        return Value();
    }
}

Value CoreIde::GetGraphStats(const String& pkg, String& error) {
    try {
        // Return overall graph statistics (in a real implementation, this would
        // compute statistics specific to the package's position in the graph)
        return telemetry.GetGraphStats(graph);
    } catch (const Exc& e) {
        error = e;
        return Value();
    }
}

Value CoreIde::GetFileComplexity(const String& path) {
    String contents = LoadFile(path);
    return telemetry.ComputeFileComplexity(path, contents);
}

Value CoreIde::GetGraphStats() {
    return telemetry.GetGraphStats(graph);
}

Value CoreIde::GetEditHistory() {
    return telemetry.GetEditHistory();
}

// Optimization Loop v1
Value CoreIde::RunOptimizationLoop(const String& package,
                                   const CoreOptimize::LoopConfig& cfg,
                                   String& error) {
    // Collect metrics before optimization loop
    ValueMap metrics_before = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return Value(); // Return early on error
    }

    CoreOptimize::LoopResult result = optimizer.RunOptimizationLoop(package, cfg, *this, error);

    // Collect metrics after optimization loop
    ValueMap metrics_after = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return Value(); // Return early on error
    }

    // Create evolution event to record the change
    EvolutionEvent ev;
    ev.timestamp = GetSysTime();
    ev.id = AsString(GetSysTime().GetTickCount(), "%016x"); // Generate a unique ID
    ev.package = package;
    ev.agent_name = "unknown"; // Would be set by caller
    ev.scenario_id = "direct_call"; // Direct call rather than through scenario
    ev.lifecycle_phase = GetCurrentLifecyclePhase().name; // Get current lifecycle phase
    ev.strategy = GetActiveStrategy() ? GetActiveStrategy()->name : "default"; // Get active strategy name
    ev.context.Set("stability", GetLifecycleStability(error));
    if (error.IsEmpty()) {
        ev.context.Set("phase", GetCurrentLifecyclePhase().name);
    }

    ev.change_kinds.Add("optimization");
    ev.metrics_before = metrics_before;
    ev.metrics_after = metrics_after;

    // Calculate deltas
    for (int i = 0; i < metrics_before.GetCount(); i++) {
        String key = metrics_before.GetKey(i);
        Value b_val = metrics_before[i];
        if (metrics_after.Find(key) >= 0) {
            Value a_val = metrics_after[key];

            if (Is<double>(b_val) && Is<double>(a_val)) {
                ev.deltas.Set(key, As<double>(a_val) - As<double>(b_val));
            } else if (Is<int>(b_val) && Is<int>(a_val)) {
                ev.deltas.Set(key, As<int>(a_val) - As<int>(b_val));
            } else {
                ev.deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
            }
        }
    }

    ev.succeeded = result.success;
    ev.reverted = false; // This will be set to true if the change is later reverted

    // Record the event in the evolution engine
    String evolution_path = workspace_root + "/.aiupp/evolution.json";
    evolution.Load(evolution_path); // Load existing events
    evolution.RecordEvent(ev); // Record the new event
    evolution.Save(evolution_path); // Save back to file

    // Convert the result to a ValueMap for return
    ValueMap vm;
    vm.Set("success", result.success);
    vm.Set("reason", result.reason);

    // Convert iterations to ValueArray
    ValueArray iterations;
    for (const auto& iter : result.iterations) {
        ValueMap iter_map;
        iter_map.Set("index", iter.index);
        iter_map.Set("before_stats", iter.before_stats);
        iter_map.Set("after_stats", iter.after_stats);
        iter_map.Set("changes", iter.changes);
        iter_map.Set("score_delta", iter.score_delta);
        iterations.Add(iter_map);
    }
    vm.Set("iterations", iterations);

    return vm;
}

// Strategy registry management for Supervisor v2
bool CoreIde::InitializeStrategies(const String& strategies_path, String& error) {
    return strategy_registry.Load(strategies_path, error);
}

bool CoreIde::SetActiveStrategy(const String& name, String& error) {
    return supervisor.SetActiveStrategy(name, error);
}

const StrategyProfile* CoreIde::GetActiveStrategy() const {
    return supervisor.GetActiveStrategy();
}

const Vector<StrategyProfile>& CoreIde::GetAllStrategies() const {
    return strategy_registry.GetAll();
}

// Supervisor v1 - Generate optimization plan for a package
Value CoreIde::GenerateOptimizationPlan(const String& package, String& error) {
    CoreSupervisor::Plan plan = supervisor.GenerateOptimizationPlan(package, *this, error);

    if (!error.IsEmpty()) {
        return Value(); // Return empty value on error
    }

    // Convert the plan to a ValueMap for return
    ValueMap vm;
    vm.Set("summary", plan.summary);

    // Convert steps to ValueArray
    ValueArray steps;
    for (const auto& step : plan.steps) {
        ValueMap step_map;
        step_map.Set("action", step.action);
        step_map.Set("target", step.target);
        step_map.Set("params", step.params);
        step_map.Set("reason", step.reason);
        steps.Add(step_map);
    }
    vm.Set("steps", steps);

    // Add strategy information used to generate the plan
    vm.Set("strategy", plan.strategy_info);

    return vm;
}

Value CoreIde::GenerateWorkspacePlan(String& error) {
    CoreSupervisor::Plan plan = supervisor.GenerateWorkspacePlan(*this, error);

    if (!error.IsEmpty()) {
        return Value(); // Return empty value on error
    }

    // Convert the plan to a ValueMap for return
    ValueMap vm;
    vm.Set("summary", plan.summary);

    // Convert steps to ValueArray
    ValueArray steps;
    for (const auto& step : plan.steps) {
        ValueMap step_map;
        step_map.Set("action", step.action);
        step_map.Set("target", step.target);
        step_map.Set("params", step.params);
        step_map.Set("reason", step.reason);
        steps.Add(step_map);
    }
    vm.Set("steps", steps);

    // Add strategy information used to generate the plan
    vm.Set("strategy", plan.strategy_info);

    return vm;
}

// Semantic analysis v1
bool CoreIde::AnalyzeSemantics(String& error) {
    return semantic.AnalyzeWorkspace(workspace, assist, graph, error);
}

const CoreSemantic& CoreIde::GetSemanticAnalyzer() const {
    return semantic;
}

CoreSemantic& CoreIde::GetSemanticAnalyzer() {
    return semantic;
}

// Scenario operations
Value CoreIde::BuildScenarioFromPlan(const String& package, int max_actions, String& error) {
    // Generate an optimization plan from the supervisor
    CoreSupervisor::Plan sup_plan = supervisor.GenerateOptimizationPlan(package, *this, error);

    if (!error.IsEmpty()) {
        return Value();
    }

    // Convert the supervisor plan to a Value to pass to BuildPlanFromSupervisor
    ValueMap sup_plan_value;
    sup_plan_value.Set("summary", sup_plan.summary);

    // Convert steps to ValueArray
    ValueArray steps_array;
    for (const auto& step : sup_plan.steps) {
        ValueMap step_map;
        step_map.Set("action", step.action);
        step_map.Set("target", step.target);
        step_map.Set("params", step.params);
        step_map.Set("reason", step.reason);
        steps_array.Add(step_map);
    }
    sup_plan_value.Set("steps", steps_array);
    sup_plan_value.Set("strategy", sup_plan.strategy_info);
    sup_plan_value.Set("semantic_snapshot", sup_plan.semantic_snapshot);

    CoreScenario::ScenarioPlan scenario_plan = scenario.BuildPlanFromSupervisor(sup_plan_value, max_actions);

    // Convert to ValueMap for return
    ValueMap result;
    result.Set("name", scenario_plan.name);

    ValueArray actions;
    for (const auto& action : scenario_plan.actions) {
        ValueMap action_map;
        action_map.Set("type", action.type);
        action_map.Set("target", action.target);
        action_map.Set("params", action.params);
        actions.Add(action_map);
    }
    result.Set("actions", actions);

    result.Set("metadata", scenario_plan.metadata);

    return result;
}

Value CoreIde::SimulateScenario(const Value& plan_desc, String& error) {
    // Convert Value to ScenarioPlan
    if (!IsValueMap(plan_desc)) {
        error = "plan_desc must be a ValueMap";
        return Value();
    }

    const ValueMap& plan_map = ValueTo<ValueMap>(plan_desc);
    CoreScenario::ScenarioPlan plan;
    plan.name = plan_map.Get("name", String("default_scenario"));

    // Convert actions from ValueArray to Vector<ScenarioAction>
    if (plan_map.Find("actions") >= 0) {
        ValueArray actions = plan_map.Get("actions");
        for (int i = 0; i < actions.GetCount(); i++) {
            if (IsValueMap(actions[i])) {
                const ValueMap& action_map = ValueTo<ValueMap>(actions[i]);
                CoreScenario::ScenarioAction action;
                action.type = action_map.Get("type", String("command"));
                action.target = action_map.Get("target", String(""));
                action.params = action_map.Get("params", ValueMap());
                plan.actions.Add(action);
            }
        }
    }

    // Perform simulation
    CoreScenario::ScenarioResult result = scenario.Simulate(plan, *this, error);

    if (!error.IsEmpty()) {
        return Value();
    }

    // Convert result to ValueMap for return
    ValueMap result_map;
    ValueMap plan_map_result;
    plan_map_result.Set("name", result.plan.name);

    ValueArray actions_result;
    for (const auto& action : result.plan.actions) {
        ValueMap action_map;
        action_map.Set("type", action.type);
        action_map.Set("target", action.target);
        action_map.Set("params", action.params);
        actions_result.Add(action_map);
    }
    plan_map_result.Set("actions", actions_result);

    result_map.Set("plan", plan_map_result);
    result_map.Set("before", result.before.telemetry); // Simplified for now
    result_map.Set("after", result.after.telemetry); // Simplified for now
    result_map.Set("deltas", result.deltas);
    result_map.Set("applied", result.applied);

    return result_map;
}

Value CoreIde::ApplyScenario(const Value& plan_desc,
                             String& error) {
    // Convert Value to ScenarioPlan
    if (!IsValueMap(plan_desc)) {
        error = "plan_desc must be a ValueMap";
        return Value();
    }

    const ValueMap& plan_map = ValueTo<ValueMap>(plan_desc);
    CoreScenario::ScenarioPlan plan;
    plan.name = plan_map.Get("name", String("default_scenario"));

    // Convert actions from ValueArray to Vector<ScenarioAction>
    if (plan_map.Find("actions") >= 0) {
        ValueArray actions = plan_map.Get("actions");
        for (int i = 0; i < actions.GetCount(); i++) {
            if (IsValueMap(actions[i])) {
                const ValueMap& action_map = ValueTo<ValueMap>(actions[i]);
                CoreScenario::ScenarioAction action;
                action.type = action_map.Get("type", String("command"));
                action.target = action_map.Get("target", String(""));
                action.params = action_map.Get("params", ValueMap());
                plan.actions.Add(action);
            }
        }
    }

    // Collect metrics before applying scenario
    ValueMap metrics_before = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return Value(); // Return early on error
    }

    // Perform the actual application of the scenario
    CoreScenario::ScenarioResult result = scenario.Apply(plan, *this, error);

    if (!error.IsEmpty()) {
        return Value();
    }

    // Collect metrics after applying scenario
    ValueMap metrics_after = GetTelemetryData("", error);
    if (!error.IsEmpty()) {
        return Value(); // Return early on error
    }

    // Create evolution event to record the change
    EvolutionEvent ev;
    ev.timestamp = GetSysTime();
    ev.id = AsString(GetSysTime().GetTickCount(), "%016x"); // Generate a unique ID
    ev.package = plan_map.Get("target_package", String("unknown")); // Get target package from plan metadata
    ev.agent_name = plan_map.Get("initiator", String("unknown")); // Get agent name from plan metadata
    ev.scenario_id = plan_map.Get("scenario_id", String("unknown")); // Get scenario ID from plan metadata
    ev.lifecycle_phase = GetCurrentLifecyclePhase().name; // Get current lifecycle phase
    ev.strategy = GetActiveStrategy() ? GetActiveStrategy()->name : "default"; // Get active strategy name
    // Add context (drift, stability, seasonality snapshot)
    ev.context.Set("stability", GetLifecycleStability(error));
    if (error.IsEmpty()) {
        // Add more context if needed
        ev.context.Set("phase", GetCurrentLifecyclePhase().name);
    }
    ev.context.Set("telemetry_before", metrics_before);
    ev.context.Set("telemetry_after", metrics_after);

    // Determine change kinds based on scenario actions
    for (const auto& action : plan.actions) {
        if (action.type == "refactor") {
            if (action.target == "rename_symbol") {
                ev.change_kinds.Add("rename");
            } else if (action.target == "remove_dead_includes") {
                ev.change_kinds.Add("include_cleanup");
            } else if (action.target == "canonicalize_includes") {
                ev.change_kinds.Add("include_normalization");
            } else {
                ev.change_kinds.Add(action.target);
            }
        } else if (action.type == "command") {
            if (action.target == "optimize_package") {
                ev.change_kinds.Add("optimization");
            } else {
                ev.change_kinds.Add(action.target);
            }
        } else {
            ev.change_kinds.Add(action.type);
        }
    }

    ev.metrics_before = metrics_before;
    ev.metrics_after = metrics_after;

    // Calculate deltas
    for (int i = 0; i < metrics_before.GetCount(); i++) {
        String key = metrics_before.GetKey(i);
        Value b_val = metrics_before[i];
        if (metrics_after.Find(key) >= 0) {
            Value a_val = metrics_after[key];

            if (Is<double>(b_val) && Is<double>(a_val)) {
                ev.deltas.Set(key, As<double>(a_val) - As<double>(b_val));
            } else if (Is<int>(b_val) && Is<int>(a_val)) {
                ev.deltas.Set(key, As<int>(a_val) - As<int>(b_val));
            } else {
                ev.deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
            }
        }
    }

    ev.succeeded = result.applied; // If applied successfully, mark as succeeded
    ev.reverted = false; // This will be set to true if the change is later reverted

    // Record the event in the evolution engine
    String evolution_path = workspace_root + "/.aiupp/evolution.json";
    evolution.Load(evolution_path); // Load existing events
    evolution.RecordEvent(ev); // Record the new event
    evolution.Save(evolution_path); // Save back to file

    // Convert result to ValueMap for return
    ValueMap result_map;
    ValueMap plan_map_result;
    plan_map_result.Set("name", result.plan.name);

    ValueArray actions_result;
    for (const auto& action : result.plan.actions) {
        ValueMap action_map;
        action_map.Set("type", action.type);
        action_map.Set("target", action.target);
        action_map.Set("params", action.params);
        actions_result.Add(action_map);
    }
    plan_map_result.Set("actions", actions_result);

    result_map.Set("plan", plan_map_result);
    result_map.Set("before", result.before.telemetry); // Simplified for now
    result_map.Set("after", result.after.telemetry); // Simplified for now
    result_map.Set("deltas", result.deltas);
    result_map.Set("applied", result.applied);
    result_map.Set("unified_diff", result.unified_diff);

    return result_map;
}

Value CoreIde::BuildProposal(const String& package,
                             int max_actions,
                             String& error) {
    // Build the proposal using the CoreProposal class
    CoreProposal::Proposal proposal = this->proposal.BuildProposal(*this, package, max_actions, error);

    if (!error.IsEmpty()) {
        return Value(); // Return empty value on error
    }

    // Convert the proposal to Value using the CoreProposal's ToValue method
    return this->proposal.ToValue(proposal);
}



Value CoreIde::RevertPatch(const String& patch_text, String& error) {
    // Use the CoreScenario to perform the revert
    CoreScenario::ScenarioResult result = scenario.Revert(patch_text, *this, error);

    if (!error.IsEmpty()) {
        return Value();
    }

    // If the revert was successful, update ProjectMemory to mark any applicable changes as reverted
    if (result.applied) {
        // Mark the corresponding entries in ProjectMemory as reverted
        for (auto& entry : memory.history) {
            // This is a simple check - in a real implementation, you'd need a more sophisticated
            // way to match patches to memory entries
            if (result.unified_diff.Contains(entry.proposal_id)) {
                entry.reverted = true;
            }
        }
    }

    // Save the updated memory to file
    String memory_file_path = workspace_root + "/.aiupp/project_memory.json";
    memory.Save(memory_file_path);

    // Convert result to ValueMap for return
    ValueMap result_map;
    ValueMap plan_map_result;
    plan_map_result.Set("name", result.plan.name);

    ValueArray actions_result;
    for (const auto& action : result.plan.actions) {
        ValueMap action_map;
        action_map.Set("type", action.type);
        action_map.Set("target", action.target);
        action_map.Set("params", action.params);
        actions_result.Add(action_map);
    }
    plan_map_result.Set("actions", actions_result);

    result_map.Set("plan", plan_map_result);
    result_map.Set("before", result.before.telemetry); // Simplified for now
    result_map.Set("after", result.after.telemetry); // Simplified for now
    result_map.Set("deltas", result.deltas);
    result_map.Set("applied", result.applied);
    result_map.Set("unified_diff", result.unified_diff);

    return result_map;
}

// Lifecycle Analysis v1 - Get the current lifecycle phase with history tracking
LifecyclePhase CoreIde::GetCurrentLifecyclePhase() const {
    // Get current metrics needed for lifecycle detection
    Vector<CoreTelemetry::PackageStats> pkg_stats = telemetry.GetPackageStats(workspace);

    // Aggregate metrics for workspace-level analysis
    int total_complexity = 0;
    int total_coupling = 0;
    int total_files = 0;
    int total_packages = pkg_stats.GetCount();

    for (const auto& stats : pkg_stats) {
        total_complexity += stats.complexity_score;
        total_coupling += stats.coupling_score;
        total_files += stats.files;
    }

    // Calculate average complexities
    double avg_complexity = total_packages > 0 ? (double)total_complexity / total_packages : 0.0;
    double avg_coupling = total_packages > 0 ? (double)total_coupling / total_packages : 0.0;

    // Calculate semantic entropy based on package distribution
    double semantic_entropy = 0.0;  // Placeholder - in real implementation this would be computed from semantic analysis

    // Get temporal dynamics trend for lifecycle detection
    TemporalDynamics::Trend trend = telemetry.GetTemporalTrend();

    // Get architecture diagnostic metrics
    ArchitectureDiagnostic diag;
    diag.complexity_index = avg_complexity;
    diag.coupling_score = avg_coupling;
    diag.structural_entropy = semantic_entropy;

    // Determine the lifecycle phase using the model
    LifecyclePhase phase = lifecycle.DetectPhase(trend, diag, semantic_entropy);

    // Record this phase in the history
    const_cast<CoreIde*>(this)->RecordCurrentPhase(phase);

    return phase;
}

// Helper method to record the current phase in the lifecycle model
void CoreIde::RecordCurrentPhase(const LifecyclePhase& phase) {
    // Record the phase in the lifecycle model
    lifecycle.RecordPhase(phase);

    // Set up storage path for lifecycle history (e.g., .aiupp/lifecycle.json in workspace root)
    String lifecycle_path = workspace_root + "/.aiupp/lifecycle.json";
    lifecycle.SetStoragePath(lifecycle_path);

    // Save the updated history to persist it
    lifecycle.Save(lifecycle_path);
}

Value CoreIde::GetLifecycleDrift(String& error) const {
    DriftMetrics drift = lifecycle.ComputeDrift();

    // Create a ValueMap with the drift metrics
    ValueMap result;
    result.Set("transitions", drift.transitions);
    result.Set("back_and_forth", drift.back_and_forth);
    result.Set("avg_phase_duration", drift.avg_phase_duration);

    // Include the history for complete information
    Vector<PhaseSample> history = lifecycle.GetHistory();
    ValueArray history_array;
    for (const PhaseSample& sample : history) {
        ValueMap sample_map;
        sample_map.Set("timestamp", (int64)sample.timestamp.GetUnix());
        sample_map.Set("phase", sample.phase_name);
        history_array.Add(sample_map);
    }
    result.Set("history", history_array);

    return result;
}

double CoreIde::GetLifecycleStability(String& error) const {
    // Get drift metrics
    DriftMetrics drift = lifecycle.ComputeDrift();

    // Get temporal dynamics trend
    TemporalDynamics::Trend trend = telemetry.GetTemporalTrend();

    // Calculate stability index using the lifecycle model
    double stability = lifecycle.ComputeStabilityIndex(drift, trend);

    return stability;
}

// Orchestrator v1 - Multi-project roadmap management
void CoreIde::AddWorkspaceToOrchestrator(const String& path) {
    orchestrator.AddWorkspace(path);
}

Value CoreIde::GetWorkspaceSummaries() {
    Vector<ProjectSummary> summaries = orchestrator.Summaries();
    ValueArray result;

    for (const ProjectSummary& summary : summaries) {
        ValueMap summary_map;
        summary_map.Set("name", summary.name);
        summary_map.Set("path", summary.path);
        summary_map.Set("stability", summary.stability);
        summary_map.Set("lifecycle_phase", summary.lifecycle_phase);
        summary_map.Set("entropy", summary.entropy);
        summary_map.Set("size_loc", summary.size_loc);
        summary_map.Set("packages", summary.packages);

        result.Add(summary_map);
    }

    return result;
}

Value CoreIde::BuildGlobalRoadmap(const String& strategy) {
    CrossWorkspacePlan plan = orchestrator.BuildGlobalRoadmap(strategy);

    ValueMap result;
    result.Set("strategy_name", plan.strategy_name);
    result.Set("proposals", plan.proposals);
    result.Set("global_metrics", plan.global_metrics);

    return result;
}

// Temporal Strategy Engine v1 - Seasonality, Release Cadence & Stability Windows

Value CoreIde::GetSeasonality() {
    // Load lifecycle history
    Vector<PhaseSample> history = lifecycle.GetHistory();

    if (history.GetCount() == 0) {
        // If no history, return an empty array
        return ValueArray();
    }

    // Detect seasonality patterns
    Vector<SeasonalityPattern> patterns = seasonality.DetectSeasonality(history);

    // Convert to Value
    ValueArray result;
    for (const auto& pattern : patterns) {
        ValueMap pattern_map;
        pattern_map.Set("name", pattern.name);
        pattern_map.Set("intensity", pattern.intensity);
        pattern_map.Set("confidence", pattern.confidence);

        ValueArray peaks_array;
        for (int peak : pattern.peaks) {
            peaks_array.Add(peak);
        }
        pattern_map.Set("peaks", peaks_array);

        result.Add(pattern_map);
    }

    return result;
}

Value CoreIde::GetReleaseCadence() {
    // Load lifecycle history
    Vector<PhaseSample> history = lifecycle.GetHistory();

    if (history.GetCount() == 0) {
        // If no history, return an empty map
        ValueMap result;
        result.Set("average_interval", 0);
        result.Set("confidence", 0.0);
        return result;
    }

    // Infer release cadence
    ReleaseCadence cadence = seasonality.InferReleaseCadence(history);

    // Convert to Value
    ValueMap result;
    result.Set("average_interval", cadence.average_interval);
    result.Set("confidence", cadence.confidence);

    return result;
}

Value CoreIde::GetStabilityWindows() {
    // Load lifecycle history
    Vector<PhaseSample> history = lifecycle.GetHistory();

    if (history.GetCount() == 0) {
        // If no history, return an empty array
        return ValueArray();
    }

    // Get release cadence to inform window prediction
    ReleaseCadence cadence = seasonality.InferReleaseCadence(history);

    // Predict stability windows
    Vector<StabilityWindow> windows = seasonality.PredictStabilityWindows(history, cadence);

    // Convert to Value
    ValueArray result;
    for (const auto& window : windows) {
        ValueMap window_map;
        window_map.Set("start", window.start);
        window_map.Set("end", window.end);
        window_map.Set("predicted_safety", window.predicted_safety);

        result.Add(window_map);
    }

    return result;
}

// Temporal Strategy Engine v2 - Forecasting & Shock Modeling

Value CoreIde::GetLifecycleForecast(int horizon) {
    // Load lifecycle history to perform forecasting
    Vector<PhaseSample> history = lifecycle.GetHistory();

    if (history.GetCount() == 0) {
        // If no history, return an empty array
        return ValueArray();
    }

    // Get the forecast using the forecasting engine
    Vector<ForecastPoint> forecast_points = forecast.ForecastLifecycle(history, horizon);

    // Convert forecast points to Value for return
    ValueArray result;
    for (const auto& point : forecast_points) {
        ValueMap point_map;
        point_map.Set("t", point.t);
        point_map.Set("predicted_phase", point.predicted_phase);
        point_map.Set("predicted_entropy", point.predicted_entropy);
        point_map.Set("confidence", point.confidence);

        result.Add(point_map);
    }

    return result;
}

Value CoreIde::GetRiskProfile() {
    // Load lifecycle history and release cadence to compute risk profile
    Vector<PhaseSample> history = lifecycle.GetHistory();
    ReleaseCadence cadence = seasonality.InferReleaseCadence(history);

    // Compute the risk profile based on history and cadence
    RiskProfile profile = forecast.ComputeRiskProfile(history, cadence);

    // Convert risk profile to Value for return
    ValueMap result;
    result.Set("long_term_risk", profile.long_term_risk);
    result.Set("volatility_risk", profile.volatility_risk);
    result.Set("schedule_risk", profile.schedule_risk);
    result.Set("architectural_risk", profile.architectural_risk);

    // Convert possible shocks to ValueArray
    ValueArray shocks_array;
    for (const auto& shock : profile.possible_shocks) {
        ValueMap shock_map;
        shock_map.Set("type", shock.type);
        shock_map.Set("severity", shock.severity);
        shock_map.Set("probability", shock.probability);
        shocks_array.Add(shock_map);
    }
    result.Set("possible_shocks", shocks_array);

    return result;
}

Value CoreIde::SimulateShock(const String& type) {
    // Load lifecycle history to perform shock simulation
    Vector<PhaseSample> history = lifecycle.GetHistory();

    if (history.GetCount() == 0) {
        // If no history, return an empty map
        ValueMap result;
        result.Set("type", type);
        result.Set("severity", 0.0);
        result.Set("probability", 0.0);
        return result;
    }

    // Simulate the shock of the specified type
    ShockScenario scenario = forecast.SimulateShock(history, type);

    // Convert shock scenario to Value for return
    ValueMap result;
    result.Set("type", scenario.type);
    result.Set("severity", scenario.severity);
    result.Set("probability", scenario.probability);

    return result;
}

// Strategic Navigator v1 - Multi-agent goal-oriented planning methods
void CoreIde::RegisterAgentProfile(const AgentProfile& profile) {
    navigator.RegisterAgent(profile);
}

Value CoreIde::GetAgentProfiles() const {
    Vector<AgentProfile> agents = navigator.GetAgents();

    ValueArray result;
    for(const AgentProfile& profile : agents) {
        ValueMap agent_map;
        agent_map.Set("name", profile.name);
        agent_map.Set("preferences", profile.preferences);

        ValueArray goals_array;
        for(const Goal& goal : profile.goals) {
            ValueMap goal_map;
            goal_map.Set("id", goal.id);
            goal_map.Set("description", goal.description);
            goal_map.Set("weights", goal.weights);
            goal_map.Set("priority", goal.priority);
            goals_array.Add(goal_map);
        }
        agent_map.Set("goals", goals_array);

        result.Add(agent_map);
    }

    return result;
}

Value CoreIde::BuildAgentPlan(const String& agent_name, String& error) {
    Vector<AgentProfile> agents = navigator.GetAgents();

    // Find the agent profile by name
    AgentProfile* target_profile = nullptr;
    for(int i = 0; i < agents.GetCount(); i++) {
        if(agents[i].name == agent_name) {
            target_profile = &agents[i];
            break;
        }
    }

    if(!target_profile) {
        error = "Agent profile not found: " + agent_name;
        return Value();
    }

    // Build the agent plan
    AgentPlan plan = navigator.BuildAgentPlan(*this, *target_profile, error);
    if(!error.IsEmpty()) {
        return Value();
    }

    // Convert to ValueMap for return
    ValueMap result;
    result.Set("agent_name", plan.agent_name);
    result.Set("metadata", plan.metadata);
    result.Set("proposal", plan.proposal);

    return result;
}

Value CoreIde::BuildGlobalPlan(String& error) {
    Vector<AgentProfile> agents = navigator.GetAgents();

    GlobalPlan global_plan = navigator.BuildGlobalPlan(*this, agents, error);
    if(!error.IsEmpty()) {
        return Value();
    }

    // Convert to ValueMap for return
    ValueMap result;
    result.Set("conflicts", global_plan.conflicts);
    result.Set("merged", global_plan.merged);

    // Convert agent_plans to ValueArray
    ValueArray agent_plans_array;
    for(int i = 0; i < global_plan.agent_plans.GetCount(); i++) {
        const ValueMap& agent_plan = global_plan.agent_plans[i];
        agent_plans_array.Add(agent_plan);
    }
    result.Set("agent_plans", agent_plans_array);

    return result;
}

// Conflict Resolver v1 - Patch-level negotiation methods
Value CoreIde::ResolveConflicts(String& error) {
    // First, get the global plan from the Strategic Navigator
    Vector<AgentProfile> agents = navigator.GetAgents();

    GlobalPlan global_plan = navigator.BuildGlobalPlan(*this, agents, error);
    if (!error.IsEmpty()) {
        return Value();
    }

    // Convert agent plans from ValueArray to Vector<AgentPlan>
    Vector<AgentPlan> agent_plans;
    for (int i = 0; i < global_plan.agent_plans.GetCount(); i++) {
        if (IsValueMap(global_plan.agent_plans[i])) {
            const ValueMap& plan_map = ValueTo<ValueMap>(global_plan.agent_plans[i]);

            AgentPlan agent_plan;
            agent_plan.agent_name = plan_map.Get("agent_name", String(""));
            agent_plan.metadata = plan_map.Get("metadata", ValueMap());
            agent_plan.proposal = plan_map.Get("proposal", Value());

            agent_plans.Add(agent_plan);
        }
    }

    // Run the conflict resolution
    NegotiatedResult negotiated_result = resolver.Negotiate(agent_plans, agents);

    // Convert ConflictDetail objects to Values
    ValueArray conflicts_array;
    Vector<ConflictDetail> conflicts = resolver.DetectConflicts(agent_plans);
    for (const ConflictDetail& conflict : conflicts) {
        ValueMap conflict_map;
        conflict_map.Set("file", conflict.file);
        conflict_map.Set("line", conflict.line);
        conflict_map.Set("type", conflict.type);
        conflict_map.Set("agents", conflict.agents);
        conflict_map.Set("metadata", conflict.metadata);
        conflicts_array.Add(conflict_map);
    }

    // Convert TradeOff objects to Values
    ValueArray tradeoffs_array;
    Vector<TradeOff> tradeoffs_list = resolver.EvaluateTradeOffs(conflicts, agent_plans, agents);
    for (const TradeOff& tradeoff : tradeoffs_list) {
        ValueMap tradeoff_map;
        tradeoff_map.Set("id", tradeoff.id);
        tradeoff_map.Set("description", tradeoff.description);
        tradeoff_map.Set("score", tradeoff.score);
        tradeoff_map.Set("rationale", tradeoff.rationale);
        tradeoffs_array.Add(tradeoff_map);
    }

    // Create the final result
    ValueMap result;
    result.Set("conflicts", conflicts_array);
    result.Set("tradeoffs", tradeoffs_array);

    ValueMap result_map;
    result_map.Set("final_actions", negotiated_result.final_actions);
    result_map.Set("discarded_actions", negotiated_result.discarded_actions);
    result_map.Set("tradeoffs", tradeoffs_array);

    result.Set("result", result_map);

    return result;
}

Value CoreIde::ExploreFutures(String& error) {
    try {
        // Get the latest negotiated scenario:
        // Use StrategicNavigator + CoreConflictResolver to obtain:
        // - ScenarioPlan base_plan
        // - final_actions from NegotiatedResult
        Vector<AgentProfile> agents = navigator.GetAgents();

        GlobalPlan global_plan = navigator.BuildGlobalPlan(*this, agents, error);
        if (!error.IsEmpty()) {
            return Value();
        }

        // Convert agent plans from ValueArray to Vector<AgentPlan>
        Vector<AgentPlan> agent_plans;
        for (int i = 0; i < global_plan.agent_plans.GetCount(); i++) {
            if (IsValueMap(global_plan.agent_plans[i])) {
                const ValueMap& plan_map = ValueTo<ValueMap>(global_plan.agent_plans[i]);

                AgentPlan agent_plan;
                agent_plan.agent_name = plan_map.Get("agent_name", String(""));
                agent_plan.metadata = plan_map.Get("metadata", ValueMap());
                agent_plan.proposal = plan_map.Get("proposal", Value());

                agent_plans.Add(agent_plan);
            }
        }

        // Run the conflict resolution to get final actions
        NegotiatedResult negotiated_result = resolver.Negotiate(agent_plans, agents);

        // Convert final actions to ScenarioAction vector
        Vector<CoreScenario::ScenarioAction> final_actions;
        if (IsValueArray(negotiated_result.final_actions)) {
            ValueArray final_actions_array = negotiated_result.final_actions;
            for (int i = 0; i < final_actions_array.GetCount(); i++) {
                if (IsValueMap(final_actions_array[i])) {
                    const ValueMap& action_map = ValueTo<ValueMap>(final_actions_array[i]);
                    CoreScenario::ScenarioAction action;
                    action.type = action_map.Get("type", String("command"));
                    action.target = action_map.Get("target", String(""));
                    action.params = action_map.Get("params", ValueMap());
                    final_actions.Add(action);
                }
            }
        }

        // Create a base scenario plan with a default name
        CoreScenario::ScenarioPlan base_plan;
        base_plan.name = "negotiated_base_plan";
        base_plan.actions = final_actions;
        base_plan.metadata.Set("source", "negotiated_result");

        // Compute:
        // - current lifecycle phase (GetCurrentLifecyclePhase)
        LifecyclePhase phase = GetCurrentLifecyclePhase();

        // - current temporal trend (TemporalDynamics::ComputeTrend)
        TemporalDynamics::Trend trend = telemetry.GetTemporalTrend();

        // - an appropriate stability window (from TemporalSeasonality)
        Vector<PhaseSample> history = lifecycle.GetHistory();
        ReleaseCadence cadence = seasonality.InferReleaseCadence(history);
        Vector<TemporalSeasonality::StabilityWindow> windows = seasonality.PredictStabilityWindows(history, cadence);
        TemporalSeasonality::StabilityWindow window;
        if (windows.GetCount() > 0) {
            // Use the first stability window, or compute average if multiple exist
            window = windows[0];
        } else {
            // Default window if none available
            window.lower_bound = 0.0;
            window.upper_bound = 1.0;
            window.confidence = 0.5;
        }

        // Call: OutcomeHorizon horizon = future_sim.Explore(base_plan, final_actions, phase, trend, window);
        CoreFutureSimulator::OutcomeHorizon horizon = future_sim.Explore(base_plan, final_actions, phase, trend, window);

        // Convert OutcomeHorizon into a ValueMap:
        ValueMap result;

        // Convert branches
        ValueArray branches_array;
        for (int i = 0; i < horizon.branches.GetCount(); i++) {
            const CoreFutureSimulator::FutureBranch& branch =
                static_cast<const CoreFutureSimulator::FutureBranch&>(horizon.branches[i]);

            ValueMap branch_map;
            branch_map.Set("id", branch.id);
            branch_map.Set("starting_point", branch.starting_point);

            // Convert actions array
            ValueArray actions_array;
            for (int j = 0; j < branch.actions.GetCount(); j++) {
                actions_array.Add(branch.actions[j]);
            }
            branch_map.Set("actions", actions_array);

            branch_map.Set("projected_metrics", branch.projected_metrics);
            branch_map.Set("terminal_state", branch.terminal_state);
            branch_map.Set("score", branch.score);

            branches_array.Add(branch_map);
        }
        result.Set("branches", branches_array);

        // Best branch
        result.Set("best_branch", horizon.best_branch);

        // Stats
        result.Set("stats", horizon.stats);

        return result;
    }
    catch (const Exc& e) {
        error = e;
        return Value();
    }
    catch (...) {
        error = "Unknown error occurred in ExploreFutures";
        return Value();
    }
}

Value CoreIde::GetEvolutionTimeline(String& error) const {
    // Load evolution data from file
    String evolution_path = workspace_root + "/.aiupp/evolution.json";
    const_cast<CoreIde*>(this)->evolution.Load(evolution_path); // Load the evolution data

    Vector<EvolutionEvent> events = evolution.GetTimeline();

    ValueArray result;

    for (const EvolutionEvent& ev : events) {
        ValueMap event_map;
        event_map.Set("timestamp", ev.timestamp.ToString());
        event_map.Set("id", ev.id);
        event_map.Set("package", ev.package);
        event_map.Set("agent_name", ev.agent_name);
        event_map.Set("scenario_id", ev.scenario_id);
        event_map.Set("lifecycle_phase", ev.lifecycle_phase);
        event_map.Set("strategy", ev.strategy);
        event_map.Set("succeeded", ev.succeeded);
        event_map.Set("reverted", ev.reverted);

        // Convert change_kinds to ValueArray
        ValueArray change_kinds_array;
        for (const String& kind : ev.change_kinds) {
            change_kinds_array.Add(kind);
        }
        event_map.Set("change_kinds", change_kinds_array);

        // Add metrics and deltas
        event_map.Set("metrics_before", ev.metrics_before);
        event_map.Set("metrics_after", ev.metrics_after);
        event_map.Set("deltas", ev.deltas);
        event_map.Set("context", ev.context);

        result.Add(event_map);
    }

    return result;
}

Value CoreIde::GetEvolutionSummary(String& error) const {
    // Load evolution data from file
    String evolution_path = workspace_root + "/.aiupp/evolution.json";
    const_cast<CoreIde*>(this)->evolution.Load(evolution_path); // Load the evolution data

    EvolutionSummary summary = evolution.Summarize();

    ValueMap result;
    result.Set("total_events", summary.total_events);
    result.Set("successful", summary.successful);
    result.Set("reverted_count", summary.reverted_count);

    // Convert by_change_kind map to ValueMap
    ValueMap by_change_kind_map;
    for (int i = 0; i < summary.by_change_kind.GetCount(); i++) {
        String key = summary.by_change_kind.GetKey(i);
        Value value = summary.by_change_kind[i];
        by_change_kind_map.Set(key, value);
    }
    result.Set("by_change_kind", by_change_kind_map);

    // Convert by_strategy map to ValueMap
    ValueMap by_strategy_map;
    for (int i = 0; i < summary.by_strategy.GetCount(); i++) {
        String key = summary.by_strategy.GetKey(i);
        Value value = summary.by_strategy[i];
        by_strategy_map.Set(key, value);
    }
    result.Set("by_strategy", by_strategy_map);

    // Convert by_phase map to ValueMap
    ValueMap by_phase_map;
    for (int i = 0; i < summary.by_phase.GetCount(); i++) {
        String key = summary.by_phase.GetKey(i);
        Value value = summary.by_phase[i];
        by_phase_map.Set(key, value);
    }
    result.Set("by_phase", by_phase_map);

    return result;
}

// Playbook Engine v1 - High-level workflow automation
Value CoreIde::ListPlaybooks(String& error) const {
    ValueArray result;
    Vector<Playbook> playbooks = playbook_engine.GetAll();

    for (int i = 0; i < playbooks.GetCount(); i++) {
        const Playbook& pb = playbooks[i];

        ValueMap pb_info;
        pb_info.GetAdd("id") = pb.id;
        pb_info.GetAdd("description") = pb.description;
        pb_info.GetAdd("safety_level") = pb.safety_level;

        // Add constraints
        ValueMap constraints_map;
        for(int j = 0; j < pb.constraints.GetCount(); j++) {
            String key = pb.constraints.GetKey(j);
            Value value = pb.constraints[j];
            constraints_map.Set(key, value);
        }
        pb_info.GetAdd("constraints") = constraints_map;

        result.Add(pb_info);
    }

    return result;
}

Value CoreIde::RunPlaybook(const String& id, String& error) {
    const Playbook* pb = playbook_engine.Find(id);
    if (!pb) {
        error = "Playbook not found: " + id;
        return Value();
    }

    return playbook_engine.Run(*pb, *this, error);
}
