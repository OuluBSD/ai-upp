#include "clicore.h"
#include "CoreSupervisor.h"

CoreSupervisor::CoreSupervisor() {
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestIncludeCleanup(const String& package,
                                                                const Value& pkg_stats) {
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "cleanup_includes_and_rebuild";
    suggestion.params = ValueMap().Set("package", package);
    suggestion.reason = "Many unused #includes detected.";
    
    // Check if unused includes are detected based on heuristic data
    // This is a placeholder implementation - would integrate with actual telemetry
    if (pkg_stats.IsType<V_UMAP>()) {
        const ValueMap& stats = pkg_stats;
        if (stats.Get("unused_includes", 0).GetInt() > 10) { // Heuristic threshold
            return suggestion;
        }
    }
    
    return Suggestion(); // Return empty suggestion if no cleanup needed
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRenameHotspot(const String& package,
                                                                const Value& telemetry,
                                                                CoreIde& ide) {
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "rename_symbol_safe";
    suggestion.params = ValueMap().Set("package", package);
    suggestion.reason = "Symbol appears thousands of times across workspace.";
    
    // Check if any symbol appears frequently enough to warrant renaming
    if (telemetry.IsType<V_UMAP>()) {
        const ValueMap& tel = telemetry;
        if (tel.Get("max_symbol_frequency", 0).GetInt() > 1000) { // Heuristic threshold
            String hot_symbol = tel.Get("hot_symbol", String()).ToString();
            suggestion.params.Set("symbol", hot_symbol);
            return suggestion;
        }
    }
    
    return Suggestion(); // Return empty suggestion if no hotspot found
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestGraphSimplification(const String& package,
                                                                      const Value& graph_stats) {
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "reorganize_includes";
    suggestion.params = ValueMap().Set("package", package);
    suggestion.reason = "Long dependency chains or near-cycles detected.";
    
    // Check for graph complexity issues
    if (graph_stats.IsType<V_UMAP>()) {
        const ValueMap& stats = graph_stats;
        if (stats.Get("max_dependency_chain", 0).GetInt() > 5 || // Heuristic threshold
            stats.Get("near_cycles", 0).GetInt() > 0) {
            return suggestion;
        }
    }
    
    return Suggestion(); // Return empty suggestion if no graph issues found
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRunOptimizationLoop(const String& package,
                                                                      const Value& pkg_stats) {
    Suggestion suggestion;
    suggestion.action = "optimize_package";
    suggestion.params = ValueMap()
                         .Set("name", package)
                         .Set("max_iterations", 5);
    suggestion.reason = "High complexity score & long dependency chains.";
    
    // Check if complexity exceeds thresholds
    if (pkg_stats.IsType<V_UMAP>()) {
        const ValueMap& stats = pkg_stats;
        double complexity = stats.Get("complexity_score", 0.0).GetDouble();
        double avg_complexity = stats.Get("avg_file_complexity", 0.0).GetDouble();
        
        if (complexity > 0.7 || avg_complexity > 0.6) { // Heuristic thresholds
            return suggestion;
        }
    }
    
    return Suggestion(); // Return empty suggestion if complexity is acceptable
}

double CoreSupervisor::ComputeRiskScore(const Value& pkg_stats,
                                        const Value& graph_stats,
                                        const Value& file_complexity) {
    double risk = 0.0;
    
    // Calculate risk based on various factors
    if (pkg_stats.IsType<V_UMAP>()) {
        const ValueMap& stats = pkg_stats;
        double avg_complexity = stats.Get("avg_file_complexity", 0.0).GetDouble();
        double include_density = stats.Get("include_density", 0.0).GetDouble();
        int total_bytes = stats.Get("total_bytes", 0).GetInt();
        
        // Normalize by typical values to get score between 0 and 1
        risk += (avg_complexity / 100.0) * 0.3; // 30% weight
        risk += (include_density / 0.5) * 0.3;  // 30% weight, assuming 50% is high
        risk += (min(double(total_bytes) / 100000.0, 1.0)) * 0.2; // 20% weight, assuming 100KB is large
    }
    
    if (graph_stats.IsType<V_UMAP>()) {
        const ValueMap& g_stats = graph_stats;
        int dependency_depth = g_stats.Get("dependency_depth", 0).GetInt();
        
        // Normalize and add to risk
        risk += (min(double(dependency_depth) / 10.0, 1.0)) * 0.2; // 20% weight
    }
    
    // Cap the risk score between 0 and 1
    return min(risk, 1.0);
}

CoreSupervisor::Plan CoreSupervisor::GenerateOptimizationPlan(const String& package,
                                                              CoreIde& ide,
                                                              String& error) {
    Plan plan;
    
    try {
        // Get various metrics from the IDE
        Value pkg_stats = ide.GetPackageStats(package, error);
        if (!error.IsEmpty()) {
            return plan;
        }
        
        Value telemetry = ide.GetTelemetryData(package, error);
        if (!error.IsEmpty()) {
            return plan;
        }
        
        Value graph_stats = ide.GetGraphStats(package, error);
        if (!error.IsEmpty()) {
            return plan;
        }
        
        // Generate suggestions based on heuristics
        Suggestion include_cleanup = SuggestIncludeCleanup(package, pkg_stats);
        if (!include_cleanup.action.IsEmpty()) {
            plan.steps.Add(include_cleanup);
        }
        
        Suggestion rename_hotspot = SuggestRenameHotspot(package, telemetry, ide);
        if (!rename_hotspot.action.IsEmpty()) {
            plan.steps.Add(rename_hotspot);
        }
        
        Suggestion graph_simplification = SuggestGraphSimplification(package, graph_stats);
        if (!graph_simplification.action.IsEmpty()) {
            plan.steps.Add(graph_simplification);
        }
        
        Suggestion optimization_loop = SuggestRunOptimizationLoop(package, pkg_stats);
        if (!optimization_loop.action.IsEmpty()) {
            plan.steps.Add(optimization_loop);
        }
        
        // Calculate risk score and create summary
        double risk_score = ComputeRiskScore(pkg_stats, graph_stats, Value());
        plan.summary = Format("Package '%s' shows structural issues: Risk score %.2f", package, risk_score);
        
        // Sort suggestions by priority (for now, just keep the order we added them)
        // In a more sophisticated implementation, we might have priorities
        
    } catch (const Exception& e) {
        error = e.ToString();
    } catch (...) {
        error = "Unknown error occurred generating optimization plan";
    }
    
    return plan;
}