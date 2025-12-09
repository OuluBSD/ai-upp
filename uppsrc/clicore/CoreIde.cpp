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
    // For now, we'll implement a simpler approach for recording the edit
    // In a complete implementation, this would track all affected files during rename
    bool success = refactor.RenameSymbol(old, nw, *this, error);
    // Note: In a complete implementation, we would need to get the list of affected files from the refactor operation
    // and record the before/after sizes for each file
    return success;
}

bool CoreIde::RemoveDeadIncludes(const String& path, String& error, int* out_count) {
    String old_content = LoadFile(path);
    int old_size = old_content.GetLength();
    bool success = refactor.RemoveDeadIncludes(path, *this, error, out_count);
    if (success) {
        String new_content = LoadFile(path);
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);
    }
    return success;
}

bool CoreIde::CanonicalizeIncludes(const String& path, String& error, int* out_count) {
    String old_content = LoadFile(path);
    int old_size = old_content.GetLength();
    bool success = refactor.CanonicalizeIncludes(path, *this, error, out_count);
    if (success) {
        String new_content = LoadFile(path);
        int new_size = new_content.GetLength();
        telemetry.RecordEdit(path, old_size, new_size);
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
    CoreOptimize::LoopResult result = optimizer.RunOptimizationLoop(package, cfg, *this, error);

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

    // Convert the supervisor plan to a scenario plan
    CoreScenario::ScenarioPlan scenario_plan = scenario.BuildPlanFromSupervisor(sup_plan, max_actions);

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
    if (!Is<ValueMap>(plan_desc)) {
        error = "plan_desc must be a ValueMap";
        return Value();
    }

    const ValueMap& plan_map = plan_desc;
    CoreScenario::ScenarioPlan plan;
    plan.name = plan_map.Get("name", String("default_scenario"));

    // Convert actions from ValueArray to Vector<ScenarioAction>
    if (plan_map.Find("actions") >= 0) {
        ValueArray actions = plan_map.Get("actions");
        for (int i = 0; i < actions.GetCount(); i++) {
            if (Is<ValueMap>(actions[i])) {
                const ValueMap& action_map = actions[i];
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

Value CoreIde::ApplyScenario(const Value& plan_desc, String& error) {
    // Convert Value to ScenarioPlan (similar to SimulateScenario)
    if (!Is<ValueMap>(plan_desc)) {
        error = "plan_desc must be a ValueMap";
        return Value();
    }

    const ValueMap& plan_map = plan_desc;
    CoreScenario::ScenarioPlan plan;
    plan.name = plan_map.Get("name", String("default_scenario"));

    // Convert actions from ValueArray to Vector<ScenarioAction>
    if (plan_map.Find("actions") >= 0) {
        ValueArray actions = plan_map.Get("actions");
        for (int i = 0; i < actions.GetCount(); i++) {
            if (Is<ValueMap>(actions[i])) {
                const ValueMap& action_map = actions[i];
                CoreScenario::ScenarioAction action;
                action.type = action_map.Get("type", String("command"));
                action.target = action_map.Get("target", String(""));
                action.params = action_map.Get("params", ValueMap());
                plan.actions.Add(action);
            }
        }
    }

    // Apply the scenario
    CoreScenario::ScenarioResult result = scenario.Apply(plan, *this, error);

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
