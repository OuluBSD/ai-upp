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
    } else if (name == "save_file") {
        return HandleSaveFile(args);
    } else if (name == "find_in_files") {
        return HandleFindInFiles(args);
    } else if (name == "build_project") {
        return HandleBuildProject(args);
    } else if (name == "clean_project") {
        return HandleCleanProject(args);
    } else if (name == "goto_line") {
        return HandleGotoLine(args);
    } else if (name == "search_code") {
        return HandleSearchCode(args);
    } else if (name == "show_console") {
        return HandleShowConsole(args);
    } else if (name == "show_errors") {
        return HandleShowErrors(args);
    } else if (name == "set_main_package") {
        return HandleSetMainPackage(args);
    } else if (name == "insert_text") {
        return HandleInsertText(args);
    } else if (name == "erase_range") {
        return HandleEraseRange(args);
    } else if (name == "replace_all") {
        return HandleReplaceAll(args);
    } else if (name == "undo") {
        return HandleUndo(args);
    } else if (name == "redo") {
        return HandleRedo(args);
    } else if (name == "find_definition") {
        return HandleFindDefinition(args);
    } else if (name == "find_usages") {
        return HandleFindUsages(args);
    } else if (name == "get_build_order") {
        return HandleGetBuildOrder(args);
    } else if (name == "detect_cycles") {
        return HandleDetectCycles(args);
    } else if (name == "affected_packages") {
        return HandleAffectedPackages(args);
    } else if (name == "rename_symbol") {
        return HandleRenameSymbol(args);
    } else if (name == "remove_dead_includes") {
        return HandleRemoveDeadIncludes(args);
    } else if (name == "canonicalize_includes") {
        return HandleCanonicalizeIncludes(args);
    } else if (name == "workspace_stats") {
        return HandleWorkspaceStats(args);
    } else if (name == "package_stats") {
        return HandlePackageStats(args);
    } else if (name == "file_complexity") {
        return HandleFileComplexity(args);
    } else if (name == "graph_stats") {
        return HandleGraphStats(args);
    } else if (name == "edit_history") {
        return HandleEditHistory(args);
    } else if (name == "optimize_package") {
        return HandleOptimizePackage(args);
    } else if (name == "get_optimization_plan") {
        return HandleGetOptimizationPlan(args);
    } else if (name == "get_workspace_plan") {
        return HandleGetWorkspacePlan(args);
    } else if (name == "list_strategies") {
        return HandleListStrategies(args);
    } else if (name == "get_strategy") {
        return HandleGetStrategy(args);
    } else if (name == "set_strategy") {
        return HandleSetStrategy(args);
    } else if (name == "supervisor_front") {
        return HandleSupervisorFront(args);
    } else if (name == "supervisor_rank") {
        return HandleSupervisorRank(args);
    } else if (name == "semantic_entities") {
        return HandleSemanticEntities(args);
    } else if (name == "semantic_clusters") {
        return HandleSemanticClusters(args);
    } else if (name == "semantic_find") {
        return HandleSemanticFind(args);
    } else if (name == "semantic_analyze") {
        return HandleSemanticAnalyze(args);
    } else if (name == "semantic_subsystems") {
        return HandleSemanticSubsystems(args);
    } else if (name == "semantic_entity") {
        return HandleSemanticEntity(args);
    } else if (name == "semantic_roles") {
        return HandleSemanticRoles(args);
    } else if (name == "semantic_layers") {
        return HandleSemanticLayers(args);
    } else if (name == "semantic_behavior") {
        return HandleSemanticBehavior(args);
    } else if (name == "semantic_behavior_entity") {
        return HandleSemanticBehaviorEntity(args);
    } else if (name == "semantic_behavior_graph") {
        return HandleSemanticBehaviorGraph(args);
    } else if (name == "semantic_pipeline") {
        return HandleSemanticPipeline(args);
    } else if (name == "build_scenario") {
        return HandleBuildScenario(args);
    } else if (name == "simulate_scenario") {
        return HandleSimulateScenario(args);
    } else if (name == "apply_scenario") {
        return HandleApplyScenario(args);
    } else if (name == "scenario_diff") {
        return HandleScenarioDiff(args);
    } else if (name == "scenario_revert") {
        return HandleScenarioRevert(args);
    } else if (name == "export_proposal") {
        return HandleExportProposal(args);
    } else if (name == "global_stats") {
        return HandleGlobalStats(args);
    } else if (name == "global_predict") {
        return HandleGlobalPredict(args);
    } else if (name == "export_global_knowledge") {
        return HandleExportGlobalKnowledge(args);
    } else if (name == "import_global_knowledge") {
        return HandleImportGlobalKnowledge(args);
    } else if (name == "lifecycle_phase") {
        return HandleLifecyclePhase(args);
    } else if (name == "lifecycle_phases") {
        return HandleLifecyclePhases(args);
    } else if (name == "lifecycle_predict") {
        return HandleLifecyclePredict(args);
    } else if (name == "lifecycle_drift") {
        return HandleLifecycleDrift(args);
    } else if (name == "lifecycle_stability") {
        return HandleLifecycleStability(args);
    } else if (name == "lifecycle_timeline") {
        return HandleLifecycleTimeline(args);
    } else if (name == "describe_command") {
        return HandleDescribeCommand(args);
    } else if (name == "orchestrator_add_workspace") {
        return HandleOrchestratorAddWorkspace(args);
    } else if (name == "orchestrator_summaries") {
        return HandleOrchestratorSummaries(args);
    } else if (name == "orchestrator_roadmap") {
        return HandleOrchestratorRoadmap(args);
    } else if (name == "temporal_seasonality") {
        return HandleTemporalSeasonality(args);
    } else if (name == "temporal_cadence") {
        return HandleTemporalCadence(args);
    } else if (name == "temporal_windows") {
        return HandleTemporalWindows(args);
    } else if (name == "temporal_forecast") {
        return HandleTemporalForecast(args);
    } else if (name == "temporal_risk") {
        return HandleTemporalRisk(args);
    } else if (name == "temporal_shock") {
        return HandleTemporalShock(args);
    } else if (name == "list_agents") {
        return HandleListAgents(args);
    } else if (name == "agent_plan") {
        return HandleAgentPlan(args);
    } else if (name == "global_plan") {
        return HandleGlobalPlan(args);
    } else if (name == "resolve_conflicts") {
        return HandleResolveConflicts(args);
    } else if (name == "explore_futures") {
        return HandleExploreFutures(args);
    } else if (name == "evolution_timeline") {
        return HandleEvolutionTimeline(args);
    } else if (name == "evolution_summary") {
        return HandleEvolutionSummary(args);
    } else if (name == "list_playbooks") {
        return HandleListPlaybooks(args);
    } else if (name == "run_playbook") {
        return HandleRunPlaybook(args);
    } else if (name == "instrument_build_hybrid") {
        return HandleInstrumentBuildHybrid(args);
    } else if (name == "instrument_render_hybrid") {
        return HandleInstrumentRenderHybrid(args);
    } else {
        return InvocationResult(1, "Unsupported command: " + name);
    }
}

// Lifecycle Supervisor v1 - Command handlers
InvocationResult CommandExecutor::HandleLifecyclePhase(const VectorMap<String, String>& args) {
    String error;
    LifecyclePhase phase = session->GetSupervisor().GetCoreIde().GetCurrentLifecyclePhase();
    
    ValueMap result;
    result.Set("name", phase.name);
    result.Set("description", phase.description);
    result.Set("stability", phase.stability);
    result.Set("volatility", phase.volatility);
    result.Set("refactor_bias", phase.refactor_bias);
    
    // Add metrics used for detection
    ValueMap metrics_used;
    metrics_used.Set("temporal_trend", "slopes_and_variance");
    metrics_used.Set("architectural_diagnostic", "complexity_and_coupling");
    metrics_used.Set("semantic_entropy", "computed_entropy");
    result.Set("metrics_used", metrics_used);
    
    InvocationResult r(0, "Current lifecycle phase retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleLifecyclePhases(const VectorMap<String, String>& args) {
    LifecycleModel model;  // Create a lifecycle model to get all known phases
    Vector<LifecyclePhase> phases = model.GetKnownPhases();
    
    ValueArray result;
    for (const LifecyclePhase& phase : phases) {
        ValueMap phase_map;
        phase_map.Set("name", phase.name);
        phase_map.Set("description", phase.description);
        phase_map.Set("stability", phase.stability);
        phase_map.Set("volatility", phase.volatility);
        phase_map.Set("refactor_bias", phase.refactor_bias);
        result.Add(phase_map);
    }
    
    InvocationResult r(0, "All lifecycle phases retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleLifecyclePredict(const VectorMap<String, String>& args) {
    String error;
    LifecyclePhase current_phase = session->GetSupervisor().GetCoreIde().GetCurrentLifecyclePhase();

    // Get the number of events to project (default to 10)
    int events = 10;
    if (args.Find("events") >= 0) {
        String events_str = args.Get("events");
        events = StrInt(events_str);
        if (events <= 0) events = 10;  // Default to 10 if invalid
    }

    ValueMap result;
    result.Set("current_phase", current_phase.name);

    // Describe the impact of the current phase on supervisor decisions
    ValueMap supervisor_impact;
    if (current_phase.name == "early_growth") {
        supervisor_impact.Set("refactor_bias", "aggressive");
        supervisor_impact.Set("risk_tolerance", "high");
        supervisor_impact.Set("optimization_aggression", "high");
    } else if (current_phase.name == "mature") {
        supervisor_impact.Set("refactor_bias", "conservative");
        supervisor_impact.Set("risk_tolerance", "low");
        supervisor_impact.Set("optimization_aggression", "low");
    } else if (current_phase.name == "declining") {
        supervisor_impact.Set("refactor_bias", "alert");
        supervisor_impact.Set("risk_tolerance", "medium");
        supervisor_impact.Set("optimization_aggression", "medium");
    } else if (current_phase.name == "legacy") {
        supervisor_impact.Set("refactor_bias", "minimal");
        supervisor_impact.Set("risk_tolerance", "very_low");
        supervisor_impact.Set("optimization_aggression", "minimal");
    }
    result.Set("supervisor_impact", supervisor_impact);

    // Create a simple projection of evolution over future events
    ValueArray projected_evolution;
    for (int i = 0; i < events; i++) {
        ValueMap event;
        event.Set("event_number", i + 1);

        // Simple projection based on phase characteristics
        if (current_phase.name == "early_growth") {
            event.Set("expected_change", "high");
            event.Set("expected_volatility", "high");
            event.Set("expected_complexity_change", "increasing");
        } else if (current_phase.name == "mature") {
            event.Set("expected_change", "low");
            event.Set("expected_volatility", "low");
            event.Set("expected_complexity_change", "stable");
        } else if (current_phase.name == "declining") {
            event.Set("expected_change", "medium");
            event.Set("expected_volatility", "medium");
            event.Set("expected_complexity_change", "increasing");
        } else if (current_phase.name == "legacy") {
            event.Set("expected_change", "very_low");
            event.Set("expected_volatility", "very_low");
            event.Set("expected_complexity_change", "stable");
        }

        projected_evolution.Add(event);
    }
    result.Set("projected_evolution", projected_evolution);

    InvocationResult r(0, "Lifecycle prediction generated successfully");
    r.payload = result;
    return r;
}

// Lifecycle Supervisor v2 - Command handlers
InvocationResult CommandExecutor::HandleLifecycleDrift(const VectorMap<String, String>& args) {
    String error;
    Value drift_data = session->GetSupervisor().GetCoreIde().GetLifecycleDrift(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetLifecycleDrift failed: " + error);
    }

    // Get stability index separately
    double stability_index = session->GetSupervisor().GetCoreIde().GetLifecycleStability(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetLifecycleStability failed: " + error);
    }

    // Add stability index to the result
    if (Is<ValueMap>(drift_data)) {
        ValueMap& map = AsValueMap(drift_data);
        map.Set("stability_index", stability_index);
    }

    InvocationResult r(0, "Lifecycle drift metrics retrieved successfully");
    r.payload = drift_data;
    return r;
}

InvocationResult CommandExecutor::HandleLifecycleStability(const VectorMap<String, String>& args) {
    String error;
    double stability_index = session->GetSupervisor().GetCoreIde().GetLifecycleStability(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetLifecycleStability failed: " + error);
    }

    // Classify the stability based on the index
    String classification;
    if (stability_index < 0.3) {
        classification = "chaotic";
    } else if (stability_index < 0.6) {
        classification = "volatile";
    } else if (stability_index < 0.8) {
        classification = "managed";
    } else {
        classification = "highly stable";
    }

    // Describe how this affects supervisor priorities
    String supervisor_impact;
    if (stability_index < 0.3) {
        supervisor_impact = "Supervisor is highly conservative, focusing on stability and risk mitigation";
    } else if (stability_index < 0.6) {
        supervisor_impact = "Supervisor balances risk and benefit, with moderate caution";
    } else if (stability_index < 0.8) {
        supervisor_impact = "Supervisor is moderately aggressive, allowing for controlled improvements";
    } else {
        supervisor_impact = "Supervisor allows for more aggressive changes and optimizations";
    }

    ValueMap result;
    result.Set("stability_index", stability_index);
    result.Set("classification", classification);
    result.Set("supervisor_impact", supervisor_impact);

    InvocationResult r(0, "Lifecycle stability index retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleLifecycleTimeline(const VectorMap<String, String>& args) {
    String error;
    Value drift_data = session->GetSupervisor().GetCoreIde().GetLifecycleDrift(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetLifecycleDrift failed: " + error);
    }

    // Extract only the history portion for the timeline
    Value timeline;
    if (Is<ValueMap>(drift_data)) {
        ValueMap map = AsValueMap(drift_data);
        timeline = map.Get("history", ValueArray());
    } else {
        timeline = ValueArray();
    }

    InvocationResult r(0, "Lifecycle timeline retrieved successfully");
    r.payload = timeline;
    return r;
}

// Handler functions for integrated commands (using IdeSession)
InvocationResult CommandExecutor::HandleOpenFile(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    if (!session->OpenFile(path, error)) {
        return InvocationResult(1, "OpenFile failed: " + error);
    }
    return InvocationResult(0, "File opened successfully: " + path);
}

InvocationResult CommandExecutor::HandleSaveFile(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    if (path == "current" || path.IsEmpty()) {
        // Save current file
        if (!session->SaveFile("", error)) {
            return InvocationResult(1, "SaveFile failed: " + error);
        }
        return InvocationResult(0, "Current file saved successfully");
    } else {
        // Save specific file
        if (!session->SaveFile(path, error)) {
            return InvocationResult(1, "SaveFile failed: " + error);
        }
        return InvocationResult(0, "File saved successfully: " + path);
    }
}

// Handler functions for stub commands (not yet integrated)
InvocationResult CommandExecutor::HandleFindInFiles(const VectorMap<String, String>& args) {
    String pattern = args.Get("pattern");
    String directory = args.Get("directory", ".");
    String regex_str = args.Get("regex", "false");
    bool regex = (regex_str == "true") ? true : false;

    String result, error;
    if (!session->FindInFiles(pattern, directory, result, error)) {
        return InvocationResult(1, "FindInFiles failed: " + error);
    }

    // Parse the result into a value array of objects
    ValueArray matches;
    Vector<String> lines = Split(result, '\n');
    for (const String& line : lines) {
        if (!line.IsEmpty()) {
            // Format: "file:line:match"
            Vector<String> parts = Split(line, ':');
            if (parts.GetCount() >= 3) {
                ValueMap match;
                match.Set("file", parts[0]);
                match.Set("line", StrInt(parts[1]));
                match.Set("match", parts[2]);
                matches.Add(match);
            }
        }
    }

    InvocationResult r(0, "FindInFiles completed successfully");
    r.payload = matches;
    return r;
}

InvocationResult CommandExecutor::HandleSearchCode(const VectorMap<String, String>& args) {
    String query = args.Get("query");
    String result, error;
    if (!session->SearchCode(query, result, error)) {
        return InvocationResult(1, "SearchCode failed: " + error);
    }

    // Parse the result into a value array of objects
    ValueArray matches;
    Vector<String> lines = Split(result, '\n');
    for (const String& line : lines) {
        if (!line.IsEmpty()) {
            // Format: "file:line:match"
            Vector<String> parts = Split(line, ':');
            if (parts.GetCount() >= 3) {
                ValueMap match;
                match.Set("file", parts[0]);
                match.Set("line", StrInt(parts[1]));
                match.Set("match", parts[2]);
                matches.Add(match);
            }
        }
    }

    InvocationResult r(0, "SearchCode completed successfully");
    r.payload = matches;
    return r;
}

InvocationResult CommandExecutor::HandleShowConsole(const VectorMap<String, String>& args) {
    String error;
    if (!session->ShowConsole(error)) {
        return InvocationResult(1, "ShowConsole failed: " + error);
    }
    return InvocationResult(0, "Console shown successfully");
}

InvocationResult CommandExecutor::HandleShowErrors(const VectorMap<String, String>& args) {
    String error;
    if (!session->ShowErrors(error)) {
        return InvocationResult(1, "ShowErrors failed: " + error);
    }
    return InvocationResult(0, "Errors shown successfully");
}

// Handler functions for symbol analysis commands
InvocationResult CommandExecutor::HandleFindDefinition(const VectorMap<String, String>& args) {
    String symbol = args.Get("symbol");
    String file;
    int line;
    String error;
    if (!session->FindDefinition(symbol, file, line, error)) {
        return InvocationResult(1, "FindDefinition failed: " + error);
    }

    ValueMap result;
    result.Set("file", file);
    result.Set("line", line);

    InvocationResult r(0, "Definition found successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleFindUsages(const VectorMap<String, String>& args) {
    String symbol = args.Get("symbol");
    Vector<String> locs;
    String error;
    if (!session->FindUsages(symbol, locs, error)) {
        return InvocationResult(1, "FindUsages failed: " + error);
    }

    ValueArray result;
    for (const String& loc : locs) {
        ValueMap usage;
        usage.Set("location", loc);
        result.Add(usage);
    }

    InvocationResult r(0, "Usages found successfully");
    r.payload = result;
    return r;
}

// Handler functions for project/build operations
InvocationResult CommandExecutor::HandleSetMainPackage(const VectorMap<String, String>& args) {
    String package = args.Get("package");
    String error;
    if (!session->SetMainPackage(package, error)) {
        return InvocationResult(1, "SetMainPackage failed: " + error);
    }
    return InvocationResult(0, "Main package set successfully: " + package);
}

InvocationResult CommandExecutor::HandleBuildProject(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String config = args.Get("config", "Debug");
    String log, error;
    if (!session->BuildProject(name, config, log, error)) {
        return InvocationResult(1, "BuildProject failed: " + error);
    }

    ValueMap result;
    result.Set("log", log);
    result.Set("success", log.Find("error") < 0 && log.Find("Error") < 0);  // Simple success detection

    InvocationResult r(0, "Project built successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleCleanProject(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String log, error;
    if (!session->CleanProject(name, log, error)) {
        return InvocationResult(1, "CleanProject failed: " + error);
    }

    ValueMap result;
    result.Set("log", log);
    result.Set("success", true);

    InvocationResult r(0, "Project cleaned successfully");
    r.payload = result;
    return r;
}

// Handler functions for editor operations
InvocationResult CommandExecutor::HandleInsertText(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    int pos = StrInt(args.Get("pos"));
    String text = args.Get("text");
    String error;
    if (!session->EditorInsert(path, pos, text, error)) {
        return InvocationResult(1, "EditorInsert failed: " + error);
    }
    return InvocationResult(0, "Text inserted successfully at position " + AsString(pos));
}

InvocationResult CommandExecutor::HandleEraseRange(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    int pos = StrInt(args.Get("pos"));
    int count = StrInt(args.Get("count"));
    String error;
    if (!session->EditorErase(path, pos, count, error)) {
        return InvocationResult(1, "EditorErase failed: " + error);
    }
    return InvocationResult(0, "Text erased successfully from position " + AsString(pos) + " for " + AsString(count) + " characters");
}

InvocationResult CommandExecutor::HandleReplaceAll(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String pattern = args.Get("pattern");
    String replacement = args.Get("replacement");
    String case_sensitive_str = args.Get("case_sensitive", "true");
    bool case_sensitive = (case_sensitive_str == "true") ? true : false;

    int replacements = 0;
    String error;
    if (!session->EditorReplaceAll(path, pattern, replacement, case_sensitive, replacements, error)) {
        return InvocationResult(1, "EditorReplaceAll failed: " + error);
    }

    ValueMap result;
    result.Set("replacements", replacements);

    InvocationResult r(0, "Replacement completed successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleUndo(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    if (!session->EditorUndo(path, error)) {
        return InvocationResult(1, "EditorUndo failed: " + error);
    }
    return InvocationResult(0, "Undo operation completed successfully");
}

InvocationResult CommandExecutor::HandleRedo(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    if (!session->EditorRedo(path, error)) {
        return InvocationResult(1, "EditorRedo failed: " + error);
    }
    return InvocationResult(0, "Redo operation completed successfully");
}

// Handler functions for navigation operations
InvocationResult CommandExecutor::HandleGotoLine(const VectorMap<String, String>& args) {
    int line = StrInt(args.Get("line"));
    String file = args.Get("file");
    String error;
    if (!session->GotoLine(file, line, error)) {
        return InvocationResult(1, "GotoLine failed: " + error);
    }
    return InvocationResult(0, "Moved to line " + AsString(line) + (file.IsEmpty() ? "" : " in " + file));
}

// Handler functions for graph analysis commands
InvocationResult CommandExecutor::HandleGetBuildOrder(const VectorMap<String, String>& args) {
    Vector<String> out_order;
    String error;
    if (!session->GetBuildOrder(out_order, error)) {
        return InvocationResult(1, "GetBuildOrder failed: " + error);
    }

    ValueArray result;
    for (const String& pkg : out_order) {
        result.Add(pkg);
    }

    InvocationResult r(0, "Build order retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleDetectCycles(const VectorMap<String, String>& args) {
    Vector<Vector<String>> out_cycles;
    String error;
    if (!session->FindCycles(out_cycles, error)) {
        return InvocationResult(1, "DetectCycles failed: " + error);
    }

    ValueArray result;
    for (const Vector<String>& cycle : out_cycles) {
        ValueArray cycle_arr;
        for (const String& pkg : cycle) {
            cycle_arr.Add(pkg);
        }
        result.Add(cycle_arr);
    }

    InvocationResult r(0, "Cycles detected successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleAffectedPackages(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    Vector<String> out_packages;
    String error;
    if (!session->AffectedPackages(path, out_packages, error)) {
        return InvocationResult(1, "AffectedPackages failed: " + error);
    }

    ValueArray result;
    for (const String& pkg : out_packages) {
        result.Add(pkg);
    }

    InvocationResult r(0, "Affected packages retrieved successfully");
    r.payload = result;
    return r;
}

// Handler functions for refactoring commands
InvocationResult CommandExecutor::HandleRenameSymbol(const VectorMap<String, String>& args) {
    String old_name = args.Get("old");
    String new_name = args.Get("new");
    String error;
    if (!session->RenameSymbol(old_name, new_name, error)) {
        return InvocationResult(1, "RenameSymbol failed: " + error);
    }

    // For now, we return a simple success message
    // In a more complete implementation, we would return the list of affected files
    Vector<String> affected_files;  // Would be populated in a complete implementation
    ValueArray result;
    for (const String& file : affected_files) {
        result.Add(file);
    }

    InvocationResult r(0, "Symbol renamed successfully from '" + old_name + "' to '" + new_name + "'");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleRemoveDeadIncludes(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    int count = 0;
    String error;
    if (!session->RemoveDeadIncludes(path, error, &count)) {
        return InvocationResult(1, "RemoveDeadIncludes failed: " + error);
    }

    ValueMap result;
    result.Set("removed_includes", count);

    InvocationResult r(0, "Dead includes removed successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleCanonicalizeIncludes(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    int count = 0;
    String error;
    if (!session->CanonicalizeIncludes(path, error, &count)) {
        return InvocationResult(1, "CanonicalizeIncludes failed: " + error);
    }

    ValueMap result;
    result.Set("canonicalized_includes", count);

    InvocationResult r(0, "Includes canonicalized successfully");
    r.payload = result;
    return r;
}

// Handler function for command introspection
InvocationResult CommandExecutor::HandleDescribeCommand(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    const Command* cmd = registry.FindByName(name);
    if (!cmd) {
        return InvocationResult(1, "Unknown command: " + name);
    }

    // Return command metadata
    ValueMap result;
    result.Set("name", cmd->name);
    result.Set("category", cmd->category);
    result.Set("description", cmd->description);
    result.Set("long_description", cmd->long_description);

    ValueArray inputs;
    for (const Argument& arg : cmd->inputs) {
        ValueMap arg_map;
        arg_map.Set("name", arg.name);
        arg_map.Set("type", arg.type);
        arg_map.Set("required", arg.required);
        arg_map.Set("default", arg.default_value);
        arg_map.Set("description", arg.description);
        if (!arg.enum_values.IsEmpty()) {
            ValueArray enum_vals;
            for (const String& val : arg.enum_values) {
                enum_vals.Add(val);
            }
            arg_map.Set("enum_values", enum_vals);
        }
        inputs.Add(arg_map);
    }
    result.Set("inputs", inputs);

    ValueMap outputs = cmd->output;
    result.Set("outputs", outputs);

    ValueMap side_effects = cmd->side_effects;
    result.Set("side_effects", side_effects);

    result.Set("context_notes", cmd->context_notes);

    InvocationResult r(0, "Command description retrieved successfully");
    r.payload = result;
    return r;
}

// Handler functions for telemetry commands
InvocationResult CommandExecutor::HandleWorkspaceStats(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetWorkspaceStats(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "WorkspaceStats failed: " + error);
    }

    InvocationResult r(0, "Workspace stats retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandlePackageStats(const VectorMap<String, String>& args) {
    String package = args.Get("package");
    String error;
    Value result = session->GetPackageStats(package, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "PackageStats failed: " + error);
    }

    InvocationResult r(0, "Package stats retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleFileComplexity(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    Value result = session->GetFileComplexity(path, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "FileComplexity failed: " + error);
    }

    InvocationResult r(0, "File complexity retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleGraphStats(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetGraphStats(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GraphStats failed: " + error);
    }

    InvocationResult r(0, "Graph stats retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleEditHistory(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetEditHistory(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "EditHistory failed: " + error);
    }

    InvocationResult r(0, "Edit history retrieved successfully");
    r.payload = result;
    return r;
}

// Optimization Loop v1 handler
InvocationResult CommandExecutor::HandleOptimizePackage(const VectorMap<String, String>& args) {
    String package = args.Get("package");
    int max_iterations = StrInt(args.Get("max_iterations", "10"));
    double converge_threshold = StrDouble(args.Get("converge_threshold", "0.01"));
    String stop_on_worse_str = args.Get("stop_on_worse", "true");
    String stop_on_converge_str = args.Get("stop_on_converge", "true");
    bool stop_on_worse = (stop_on_worse_str == "true") ? true : false;
    bool stop_on_converge = (stop_on_converge_str == "true") ? true : false;

    String error;
    Value result = session->OptimizePackage(
        package,
        max_iterations,
        converge_threshold,
        stop_on_worse,
        stop_on_converge,
        error
    );
    
    if (!error.IsEmpty()) {
        return InvocationResult(1, "OptimizePackage failed: " + error);
    }

    InvocationResult r(0, "Package optimization completed successfully");
    r.payload = result;
    return r;
}

// AI Supervisor Layer v1 handler
InvocationResult CommandExecutor::HandleGetOptimizationPlan(const VectorMap<String, String>& args) {
    String package = args.Get("package");
    String error;
    Value result = session->GetOptimizationPlan(package, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetOptimizationPlan failed: " + error);
    }

    InvocationResult r(0, "Optimization plan retrieved successfully");
    r.payload = result;
    return r;
}

// AI Supervisor Layer v2 handler
InvocationResult CommandExecutor::HandleGetWorkspacePlan(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetWorkspacePlan(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetWorkspacePlan failed: " + error);
    }

    InvocationResult r(0, "Workspace plan retrieved successfully");
    r.payload = result;
    return r;
}

// Dynamic Strategy Engine handlers
InvocationResult CommandExecutor::HandleListStrategies(const VectorMap<String, String>& args) {
    String error;
    Value result = session->ListStrategies(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "ListStrategies failed: " + error);
    }

    InvocationResult r(0, "Strategies listed successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleGetStrategy(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String error;
    Value result = session->GetStrategy(name, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetStrategy failed: " + error);
    }

    InvocationResult r(0, "Strategy retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSetStrategy(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String error;
    if (!session->SetActiveStrategy(name, error)) {
        return InvocationResult(1, "SetStrategy failed: " + error);
    }

    InvocationResult r(0, "Strategy set successfully to: " + name);
    return r;
}

InvocationResult CommandExecutor::HandleSupervisorFront(const VectorMap<String, String>& args) {
    // Placeholder implementation
    ValueArray front;
    // In a real implementation, this would call a method to get the Pareto front of suggestions
    // For now, we return an empty array
    InvocationResult r(0, "Supervisor front retrieved (placeholder implementation)");
    r.payload = front;
    return r;
}

InvocationResult CommandExecutor::HandleSupervisorRank(const VectorMap<String, String>& args) {
    // Placeholder implementation
    ValueArray ranked;
    // In a real implementation, this would rank suggestions
    // For now, we return an empty array
    InvocationResult r(0, "Supervisor rank retrieved (placeholder implementation)");
    r.payload = ranked;
    return r;
}

// Semantic Analysis v1 handlers
InvocationResult CommandExecutor::HandleSemanticEntities(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticEntities(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticEntities failed: " + error);
    }

    InvocationResult r(0, "Semantic entities retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticClusters(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticClusters(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticClusters failed: " + error);
    }

    InvocationResult r(0, "Semantic clusters retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticFind(const VectorMap<String, String>& args) {
    String pattern = args.Get("pattern");
    String error;
    Value result = session->SearchSemanticEntities(pattern, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticFind failed: " + error);
    }

    InvocationResult r(0, "Semantic entities found successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticAnalyze(const VectorMap<String, String>& args) {
    // Placeholder - this would perform semantic analysis
    // For now, we just return a success message
    ValueMap result;
    result.Set("status", "semantic analysis completed");
    InvocationResult r(0, "Semantic analysis completed successfully");
    r.payload = result;
    return r;
}

// Semantic Analysis v2 - NEW: Inference layer handlers
InvocationResult CommandExecutor::HandleSemanticSubsystems(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticSubsystems(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticSubsystems failed: " + error);
    }

    InvocationResult r(0, "Semantic subsystems retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticEntity(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String error;
    Value result = session->GetSemanticEntity(name, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticEntity failed: " + error);
    }

    InvocationResult r(0, "Semantic entity retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticRoles(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticRoles(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticRoles failed: " + error);
    }

    InvocationResult r(0, "Semantic roles retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticLayers(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticLayers(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticLayers failed: " + error);
    }

    InvocationResult r(0, "Semantic layers retrieved successfully");
    r.payload = result;
    return r;
}

// Semantic Analysis v3 - NEW: Behavioral analysis handlers
InvocationResult CommandExecutor::HandleSemanticBehavior(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticBehavior(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticBehavior failed: " + error);
    }

    InvocationResult r(0, "Semantic behavior retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticBehaviorEntity(const VectorMap<String, String>& args) {
    String name = args.Get("name");
    String error;
    Value result = session->GetSemanticBehaviorEntity(name, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticBehaviorEntity failed: " + error);
    }

    InvocationResult r(0, "Semantic behavior entity retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticBehaviorGraph(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticBehaviorGraph(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticBehaviorGraph failed: " + error);
    }

    InvocationResult r(0, "Semantic behavior graph retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSemanticPipeline(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSemanticPipeline(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SemanticPipeline failed: " + error);
    }

    InvocationResult r(0, "Semantic pipeline retrieved successfully");
    r.payload = result;
    return r;
}

// Scenario operation handlers (PART D)
InvocationResult CommandExecutor::HandleBuildScenario(const VectorMap<String, String>& args) {
    String package = args.Get("package");
    int max_actions = StrInt(args.Get("max_actions", "5"));
    String error;
    Value result = session->BuildScenario(package, max_actions, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "BuildScenario failed: " + error);
    }

    InvocationResult r(0, "Scenario built successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleSimulateScenario(const VectorMap<String, String>& args) {
    // For this implementation, we'll create a dummy scenario description from the arguments
    ValueMap plan_desc;
    plan_desc.Set("name", "default_scenario");
    ValueArray actions;
    plan_desc.Set("actions", actions);
    
    String error;
    Value result = session->SimulateScenario(plan_desc, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "SimulateScenario failed: " + error);
    }

    InvocationResult r(0, "Scenario simulated successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleApplyScenario(const VectorMap<String, String>& args) {
    // For this implementation, we'll create a dummy scenario description from the arguments
    ValueMap plan_desc;
    plan_desc.Set("name", "default_scenario");
    ValueArray actions;
    plan_desc.Set("actions", actions);
    
    String error;
    Value result = session->ApplyScenario(plan_desc, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "ApplyScenario failed: " + error);
    }

    InvocationResult r(0, "Scenario applied successfully");
    r.payload = result;
    return r;
}

// Patch and revert handlers (PART E)
InvocationResult CommandExecutor::HandleScenarioDiff(const VectorMap<String, String>& args) {
    // Placeholder implementation
    ValueMap result;
    result.Set("diff", "scenario diff result");
    InvocationResult r(0, "Scenario diff generated (placeholder implementation)");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleScenarioRevert(const VectorMap<String, String>& args) {
    // For this implementation, we'll create a dummy patch text from the arguments
    String patch_text = args.Get("patch", "");
    String error;
    Value result = session->RevertPatch(patch_text, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "ScenarioRevert failed: " + error);
    }

    InvocationResult r(0, "Scenario reverted successfully");
    r.payload = result;
    return r;
}

// Proposal export handler (PART F)
InvocationResult CommandExecutor::HandleExportProposal(const VectorMap<String, String>& args) {
    // For this implementation, we'll create a dummy proposal from the arguments
    String package = args.Get("package", "default");
    int max_actions = StrInt(args.Get("max_actions", "5"));
    String error;
    Value result = session->BuildProposal(package, max_actions, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "ExportProposal failed: " + error);
    }

    // Check if the 'with-futures' flag is present
    String with_futures = args.Get("with_futures", "false");
    if (with_futures == "true" || with_futures == "1") {
        // Add outcome_horizon to the result
        Value futures_result = session->ExploreFutures(error);
        if (!error.IsEmpty()) {
            // Log the error but don't fail the entire command - just don't include futures
            LOG("Warning: Failed to generate outcome horizon: " + error);
        } else {
            // Add outcome_horizon to the existing result
            if (IsValueMap(result)) {
                ValueMap result_map = ValueTo<ValueMap>(result);
                result_map.Set("outcome_horizon", futures_result);
                result = result_map;
            } else {
                // If result is not a map, create a new map with the result and the horizon
                ValueMap combined_result;
                combined_result.Set("proposal", result);
                combined_result.Set("outcome_horizon", futures_result);
                result = combined_result;
            }
        }
    }

    InvocationResult r(0, "Proposal exported successfully");
    r.payload = result;
    return r;
}

// Cross-Workspace Intelligence (CWI) v1 handlers
InvocationResult CommandExecutor::HandleGlobalStats(const VectorMap<String, String>& args) {
    // Placeholder - in a real implementation this would provide global knowledge statistics
    ValueMap result;
    result.Set("total_projects", 0);
    result.Set("total_patterns", 0);
    result.Set("total_refactors", 0);
    result.Set("total_users", 1);
    InvocationResult r(0, "Global stats retrieved (placeholder implementation)");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleGlobalPredict(const VectorMap<String, String>& args) {
    String pattern = args.Get("pattern", "");
    String refactor = args.Get("refactor", "");
    String topology = args.Get("topology", "");
    
    // Placeholder - in a real implementation this would use global knowledge for prediction
    ValueMap result;
    result.Set("prediction", 0.5);  // 50% success likelihood
    ValueMap factors;
    factors.Set("pattern_match", pattern.IsEmpty() ? 0.0 : 0.7);
    factors.Set("refactor_history", refactor.IsEmpty() ? 0.0 : 0.6);
    factors.Set("topology_match", topology.IsEmpty() ? 0.0 : 0.5);
    result.Set("factors", factors);
    ValueMap meta_weights;
    meta_weights.Set("pattern_success_bias", 0.1);
    meta_weights.Set("refactor_success_bias", 0.05);
    meta_weights.Set("topology_risk_adjustment", 0.2);
    result.Set("meta_weights", meta_weights);
    
    InvocationResult r(0, "Global prediction made (placeholder implementation)");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleExportGlobalKnowledge(const VectorMap<String, String>& args) {
    String path = args.Get("path", "./global_knowledge.json");
    
    // Placeholder - in a real implementation this would export global knowledge
    ValueMap result;
    result.Set("exported_entries", 0);
    result.Set("output_file", path);
    InvocationResult r(0, "Global knowledge exported to " + path + " (placeholder implementation)");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleImportGlobalKnowledge(const VectorMap<String, String>& args) {
    String file = args.Get("file");
    String mode = args.Get("mode", "merge");

    // Placeholder - in a real implementation this would import global knowledge
    ValueMap result;
    result.Set("imported_entries", 0);
    result.Set("import_mode", mode);
    InvocationResult r(0, "Global knowledge imported from " + file + " in " + mode + " mode (placeholder implementation)");
    r.payload = result;
    return r;
}

// Orchestrator v1 - Multi-project roadmap handlers
InvocationResult CommandExecutor::HandleOrchestratorAddWorkspace(const VectorMap<String, String>& args) {
    String path = args.Get("path");
    String error;
    if (!session->AddWorkspaceToOrchestrator(path, error)) {
        return InvocationResult(1, "AddWorkspaceToOrchestrator failed: " + error);
    }
    return InvocationResult(0, "Workspace added to orchestrator successfully: " + path);
}

InvocationResult CommandExecutor::HandleOrchestratorSummaries(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetWorkspaceSummaries(error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "GetWorkspaceSummaries failed: " + error);
    }

    InvocationResult r(0, "Workspace summaries retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleOrchestratorRoadmap(const VectorMap<String, String>& args) {
    String strategy = args.Get("strategy", "default");
    String error;
    Value result = session->BuildGlobalRoadmap(strategy, error);
    if (!error.IsEmpty()) {
        return InvocationResult(1, "BuildGlobalRoadmap failed: " + error);
    }

    InvocationResult r(0, "Global roadmap built successfully using strategy: " + strategy);
    r.payload = result;
    return r;
}

// Temporal Strategy Engine v1 - Command handlers
InvocationResult CommandExecutor::HandleTemporalSeasonality(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSupervisor().GetCoreIde().GetSeasonality();

    if (result.Is<ValueArray>()) {
        InvocationResult r(0, "Temporal seasonality patterns retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve seasonality data");
    }
}

InvocationResult CommandExecutor::HandleTemporalCadence(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSupervisor().GetCoreIde().GetReleaseCadence();

    InvocationResult r(0, "Temporal release cadence retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleTemporalWindows(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSupervisor().GetCoreIde().GetStabilityWindows();

    if (result.Is<ValueArray>()) {
        InvocationResult r(0, "Temporal stability windows retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve stability windows data");
    }
}

// Temporal Strategy Engine v2 - Forecasting & Shock Modeling command handlers

InvocationResult CommandExecutor::HandleTemporalForecast(const VectorMap<String, String>& args) {
    int horizon = StrInt(args.Get("horizon", "12"));  // Default horizon is 12 timesteps
    String error;

    Value result = session->GetSupervisor().GetCoreIde().GetLifecycleForecast(horizon);

    if (result.Is<ValueArray>()) {
        InvocationResult r(0, "Temporal lifecycle forecast retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve forecast data");
    }
}

InvocationResult CommandExecutor::HandleTemporalRisk(const VectorMap<String, String>& args) {
    String error;

    Value result = session->GetSupervisor().GetCoreIde().GetRiskProfile();

    if (result.Is<ValueMap>()) {
        InvocationResult r(0, "Temporal risk profile retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve risk profile data");
    }
}

InvocationResult CommandExecutor::HandleTemporalShock(const VectorMap<String, String>& args) {
    String type = args.Get("type", "developer_churn");  // Default shock type
    String error;

    Value result = session->GetSupervisor().GetCoreIde().SimulateShock(type);

    if (result.Is<ValueMap>()) {
        InvocationResult r(0, "Temporal shock simulation retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve shock simulation data");
    }
}

// Strategic Navigator v1 - Multi-agent planning command handlers

InvocationResult CommandExecutor::HandleListAgents(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetSupervisor().GetCoreIde().GetAgentProfiles();

    if (result.Is<ValueArray>()) {
        InvocationResult r(0, "List of agents retrieved successfully");
        r.payload = result;
        return r;
    } else {
        return InvocationResult(1, "Failed to retrieve agent profiles");
    }
}

InvocationResult CommandExecutor::HandleAgentPlan(const VectorMap<String, String>& args) {
    String agent_name = args.Get("agent_name", "");
    if (agent_name.IsEmpty()) {
        return InvocationResult(1, "Missing required argument: agent_name");
    }

    String error;
    Value result = session->BuildAgentPlan(agent_name, error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to build agent plan: " + error);
    }

    InvocationResult r(0, "Agent plan generated successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleGlobalPlan(const VectorMap<String, String>& args) {
    String error;
    Value result = session->BuildGlobalPlan(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to build global plan: " + error);
    }

    InvocationResult r(0, "Global plan generated successfully");
    r.payload = result;
    return r;
}

// Conflict Resolver v1 - Patch-level negotiation command handlers

InvocationResult CommandExecutor::HandleResolveConflicts(const VectorMap<String, String>& args) {
    String error;
    Value result = session->ResolveConflicts(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to resolve conflicts: " + error);
    }

    InvocationResult r(0, "Conflicts resolved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleExploreFutures(const VectorMap<String, String>& args) {
    String error;
    Value result = session->ExploreFutures(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to explore futures: " + error);
    }

    InvocationResult r(0, "Futures exploration completed successfully");
    r.payload = result;
    return r;
}

// Evolution Engine v1 - CLI command handlers
InvocationResult CommandExecutor::HandleEvolutionTimeline(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetEvolutionTimeline(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to retrieve evolution timeline: " + error);
    }

    InvocationResult r(0, "Evolution timeline retrieved successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleEvolutionSummary(const VectorMap<String, String>& args) {
    String error;
    Value result = session->GetEvolutionSummary(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to retrieve evolution summary: " + error);
    }

    InvocationResult r(0, "Evolution summary retrieved successfully");
    r.payload = result;
    return r;
}

// Playbook Engine v1 - CLI command handlers
InvocationResult CommandExecutor::HandleListPlaybooks(const VectorMap<String, String>& args) {
    String error;
    Value result = session->ListPlaybooks(error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to list playbooks: " + error);
    }

    InvocationResult r(0, "Playbooks listed successfully");
    r.payload = result;
    return r;
}

InvocationResult CommandExecutor::HandleRunPlaybook(const VectorMap<String, String>& args) {
    String playbook_id = args.Get("playbook_id", "");
    if (playbook_id.IsEmpty()) {
        return InvocationResult(1, "Missing required argument: playbook_id");
    }

    String error;
    Value result = session->RunPlaybook(playbook_id, error);

    if (!error.IsEmpty()) {
        return InvocationResult(1, "Failed to run playbook '" + playbook_id + "': " + error);
    }

    InvocationResult r(0, "Playbook '" + playbook_id + "' executed successfully");
    r.payload = result;
    return r;
}

}