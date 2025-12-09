#include "clicore.h"
#include "CoreSupervisor.h"

CoreSupervisor::CoreSupervisor() {
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestIncludeCleanup(const String& package,
                                                                const Value& pkg_stats) {
	ValueMap m;
	m.Set("package", package);
	
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "cleanup_includes_and_rebuild";
    suggestion.params = m;
    suggestion.reason = "Many unused #includes detected.";
    
    // Check if unused includes are detected based on heuristic data
    // This is a placeholder implementation - would integrate with actual telemetry
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats;
        if ((int)stats.Get("unused_includes", 0) > 10) { // Heuristic threshold
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
	ValueMap m;
	m.Set("package", package);
    suggestion.params = m;
    suggestion.reason = "Symbol appears thousands of times across workspace.";

    // Check if any symbol appears frequently enough to warrant renaming
    if (telemetry.Is<ValueMap>()) {
        ValueMap tel = telemetry;  // Copy to non-const to access Get method
        if ((int)tel.Get("max_symbol_frequency", 0) > 1000) { // Heuristic threshold
			ValueMap m2;
			m2.Set("package", package);
			m2.Set("symbol", (String)tel.Get("hot_symbol", String()));
            suggestion.params = m2;
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
	ValueMap m;
	m.Set("package", package);
    suggestion.params = m;
    suggestion.reason = "Long dependency chains or near-cycles detected.";

    // Check for graph complexity issues
    if (graph_stats.Is<ValueMap>()) {
        ValueMap stats = graph_stats;  // Copy to non-const to access Get method
        if ((int)stats.Get("max_dependency_chain", 0) > 5 || // Heuristic threshold
            (int)stats.Get("near_cycles", 0) > 0) {
            return suggestion;
        }
    }

    return Suggestion(); // Return empty suggestion if no graph issues found
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRunOptimizationLoop(const String& package,
                                                                      const Value& pkg_stats) {
    Suggestion suggestion;
    suggestion.action = "optimize_package";
	ValueMap m;
	m.Set("name", package);
	m.Set("max_iterations", 5);
    suggestion.params = m;
    suggestion.reason = "High complexity score & long dependency chains.";

    // Check if complexity exceeds thresholds
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats;  // Copy to non-const to access Get method
        double complexity = (double)stats.Get("complexity_score", 0.0);
        double avg_complexity = (double)stats.Get("avg_file_complexity", 0.0);

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
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats; // Copy to non-const to access Get method
        double avg_complexity = (double)stats.Get("avg_file_complexity", 0.0);
        double include_density = (double)stats.Get("include_density", 0.0);
        int total_bytes = (int)stats.Get("total_bytes", 0);

        // Normalize by typical values to get score between 0 and 1
        risk += (avg_complexity / 100.0) * 0.3; // 30% weight
        risk += (include_density / 0.5) * 0.3;  // 30% weight, assuming 50% is high
        risk += (min(double(total_bytes) / 100000.0, 1.0)) * 0.2; // 20% weight, assuming 100KB is large
    }

    if (graph_stats.Is<ValueMap>()) {
        ValueMap g_stats = graph_stats; // Copy to non-const to access Get method
        int dependency_depth = (int)g_stats.Get("dependency_depth", 0);

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
        
    } catch (const Exc& e) {
        error = e;
    } catch (...) {
        error = "Unknown error occurred generating optimization plan";
    }
    
    return plan;
}