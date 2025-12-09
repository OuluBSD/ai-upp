#include "clicore.h"
#include "CoreSupervisor.h"
#include "CoreIde.h"

CoreSupervisor::CoreSupervisor() {
}

void CoreSupervisor::SetStrategyRegistry(const StrategyRegistry* reg) {
    registry = reg;
}

bool CoreSupervisor::SetActiveStrategy(const String& name, String& error) {
    if (!registry) {
        error = "No strategy registry available";
        return false;
    }

    const StrategyProfile* profile = registry->Find(name);
    if (!profile) {
        error = "Strategy not found: " + name;
        return false;
    }

    active = profile;
    return true;
}

const StrategyProfile* CoreSupervisor::GetActiveStrategy() const {
    return active;
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestIncludeCleanupStrategic(const String& package,
                                                                         const Value& pkg_stats) const {
    ValueMap m;
    m.Set("package", package);

    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "cleanup_includes_and_rebuild";
    suggestion.params = m;
    suggestion.reason = "Many unused #includes detected.";
    suggestion.benefit_score = 0.0;
    suggestion.cost_score = 0.0;
    suggestion.risk_score = 0.0;
    suggestion.confidence_score = 0.0;

    // Get threshold from active strategy or use default
    double min_include_density = 0.3; // Default threshold
    if (active) {
        ValueMap thresholds = active->thresholds;
        ValueMap cleanup_thresholds = thresholds.Get("cleanup_includes_and_rebuild", ValueMap());
        if (!cleanup_thresholds.IsEmpty()) {
            min_include_density = (double)cleanup_thresholds.Get("min_include_density", 0.3);
        }
    }

    // Check if unused includes are detected based on heuristic data with strategy threshold
    // This is a placeholder implementation - would integrate with actual telemetry
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats;
        if ((int)stats.Get("unused_includes", 0) > 10) { // Heuristic threshold
            // Additional check based on include density if available
            double include_density = (double)stats.Get("include_density", 1.0);
            if (include_density >= min_include_density) {
                return suggestion;
            }
        }
    }

    return Suggestion(); // Return empty suggestion if no cleanup needed
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestIncludeCleanup(const String& package,
                                                                const Value& pkg_stats) const {
    // Use the strategic version by default
    return SuggestIncludeCleanupStrategic(package, pkg_stats);
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRenameHotspotStrategic(const String& package,
                                                                        const Value& telemetry,
                                                                        CoreIde& ide) const {
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "rename_symbol_safe";
    ValueMap m;
    m.Set("package", package);
    suggestion.params = m;
    suggestion.reason = "Symbol appears frequently across workspace.";
    suggestion.benefit_score = 0.0;
    suggestion.cost_score = 0.0;
    suggestion.risk_score = 0.0;
    suggestion.confidence_score = 0.0;

    // Get threshold from active strategy or use default
    int min_symbol_occurrences = 100; // Default threshold
    if (active) {
        ValueMap thresholds = active->thresholds;
        ValueMap rename_thresholds = thresholds.Get("rename_symbol_safe", ValueMap());
        if (!rename_thresholds.IsEmpty()) {
            min_symbol_occurrences = (int)rename_thresholds.Get("min_symbol_occurrences", 100);
        }
    }

    // Check if any symbol appears frequently enough to warrant renaming with strategy threshold
    if (telemetry.Is<ValueMap>()) {
        ValueMap tel = telemetry;  // Copy to non-const to access Get method
        if ((int)tel.Get("max_symbol_frequency", 0) > min_symbol_occurrences) { // Heuristic threshold based on strategy
            ValueMap m2;
            m2.Set("package", package);
            m2.Set("symbol", (String)tel.Get("hot_symbol", String()));
            suggestion.params = m2;
            return suggestion;
        }
    }

    return Suggestion(); // Return empty suggestion if no hotspot found
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRenameHotspot(const String& package,
                                                                const Value& telemetry,
                                                                CoreIde& ide) const {
    // Use the strategic version by default
    return SuggestRenameHotspotStrategic(package, telemetry, ide);
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestGraphSimplificationStrategic(const String& package,
                                                                              const Value& graph_stats) const {
    Suggestion suggestion;
    suggestion.action = "run_playbook";
    suggestion.target = "reorganize_includes";
    ValueMap m;
    m.Set("package", package);
    suggestion.params = m;
    suggestion.reason = "Long dependency chains or near-cycles detected.";
    suggestion.benefit_score = 0.0;
    suggestion.cost_score = 0.0;
    suggestion.risk_score = 0.0;
    suggestion.confidence_score = 0.0;

    // Get threshold from active strategy or use default
    double cycles_weight = 1.5; // Default cycles weight
    if (active && const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).GetCount() > 0) {
        cycles_weight = (double)const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).Get("cycles", 1.5);
    }

    // Check for graph complexity issues with strategy influence
    if (graph_stats.Is<ValueMap>()) {
        ValueMap stats = graph_stats;  // Copy to non-const to access Get method
        int max_dependency_chain = (int)stats.Get("max_dependency_chain", 0);
        int near_cycles = (int)stats.Get("near_cycles", 0);

        // Adjust thresholds based on strategy weights
        int chain_threshold = (int)(5.0 / cycles_weight); // Higher weight means lower threshold (more sensitive)
        if (chain_threshold < 2) chain_threshold = 2; // Minimum threshold

        if (max_dependency_chain > chain_threshold ||
            (int)(near_cycles * cycles_weight) > 0) { // Weight affects how many cycles we tolerate
            return suggestion;
        }
    }

    return Suggestion(); // Return empty suggestion if no graph issues found
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestGraphSimplification(const String& package,
                                                                      const Value& graph_stats) const {
    // Use the strategic version by default
    return SuggestGraphSimplificationStrategic(package, graph_stats);
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRunOptimizationLoopStrategic(const String& package,
                                                                              const Value& pkg_stats) const {
    Suggestion suggestion;
    suggestion.action = "optimize_package";
    ValueMap m;
    m.Set("name", package);
    m.Set("max_iterations", 5);
    suggestion.params = m;
    suggestion.reason = "High complexity score & long dependency chains.";
    suggestion.benefit_score = 0.0;
    suggestion.cost_score = 0.0;
    suggestion.risk_score = 0.0;
    suggestion.confidence_score = 0.0;

    // Get threshold from active strategy or use default
    double min_risk_score = 0.5; // Default threshold
    if (active) {
        ValueMap thresholds = active->thresholds;
        ValueMap optimize_thresholds = thresholds.Get("optimize_package", ValueMap());
        if (!optimize_thresholds.IsEmpty()) {
            min_risk_score = (double)optimize_thresholds.Get("min_risk_score", 0.5);
        }
    }

    // Check if complexity exceeds thresholds with strategy influence
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats;  // Copy to non-const to access Get method
        double complexity = (double)stats.Get("complexity_score", 0.0);
        double avg_complexity = (double)stats.Get("avg_file_complexity", 0.0);

        if (complexity > min_risk_score || avg_complexity > (min_risk_score * 0.85)) { // Heuristic thresholds based on strategy
            return suggestion;
        }
    }

    return Suggestion(); // Return empty suggestion if complexity is acceptable
}

CoreSupervisor::Suggestion CoreSupervisor::SuggestRunOptimizationLoop(const String& package,
                                                                      const Value& pkg_stats) const {
    // Use the strategic version by default
    return SuggestRunOptimizationLoopStrategic(package, pkg_stats);
}

double CoreSupervisor::ComputeRiskScoreStrategic(const Value& pkg_stats,
                                                 const Value& graph_stats,
                                                 const Value& file_complexity) const {
    double risk = 0.0;

    // Get active strategy or use fallback values
    double include_density_weight = 1.0;
    double complexity_weight = 1.0;
    double dependency_depth_weight = 1.0;
    double edit_volatility_weight = 0.5;

    if (active && const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).GetCount() > 0) {
        include_density_weight = (double)const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).Get("include_density", 1.0);
        complexity_weight = (double)const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).Get("complexity", 1.0);
        dependency_depth_weight = (double)const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).Get("dependency_depth", 1.0);
        edit_volatility_weight = (double)const_cast<ValueMap&>(const_cast<StrategyProfile*>(active)->weights).Get("edit_volatility", 0.5);
    }

    // Calculate risk based on various factors using strategy weights
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats; // Copy to non-const to access Get method
        double avg_complexity = (double)stats.Get("avg_file_complexity", 0.0);
        double include_density = (double)stats.Get("include_density", 0.0);
        int total_bytes = (int)stats.Get("total_bytes", 0);

        // Normalize by typical values and apply strategy weights
        risk += (avg_complexity / 100.0) * complexity_weight * 0.25; // 25% base weight
        risk += (include_density / 0.5) * include_density_weight * 0.25;  // 25% base weight, assuming 50% is high
        risk += (min(double(total_bytes) / 100000.0, 1.0)) * edit_volatility_weight * 0.25; // 25% weight, assuming 100KB is large
    }

    if (graph_stats.Is<ValueMap>()) {
        ValueMap g_stats = graph_stats; // Copy to non-const to access Get method
        int dependency_depth = (int)g_stats.Get("dependency_depth", 0);

        // Normalize and add to risk with strategy weight
        risk += (min(double(dependency_depth) / 10.0, 1.0)) * dependency_depth_weight * 0.25; // 25% weight
    }

    // Cap the risk score between 0 and 1
    return min(risk, 1.0);
}

double CoreSupervisor::ComputeRiskScore(const Value& pkg_stats,
                                        const Value& graph_stats,
                                        const Value& file_complexity) const {
    // Use the strategic version by default
    return ComputeRiskScoreStrategic(pkg_stats, graph_stats, file_complexity);
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

        // Generate suggestions based on strategic heuristics
        Suggestion include_cleanup = SuggestIncludeCleanupStrategic(package, pkg_stats);
        if (!include_cleanup.action.IsEmpty()) {
            plan.steps.Add(include_cleanup);
        }

        Suggestion rename_hotspot = SuggestRenameHotspotStrategic(package, telemetry, ide);
        if (!rename_hotspot.action.IsEmpty()) {
            plan.steps.Add(rename_hotspot);
        }

        Suggestion graph_simplification = SuggestGraphSimplificationStrategic(package, graph_stats);
        if (!graph_simplification.action.IsEmpty()) {
            plan.steps.Add(graph_simplification);
        }

        Suggestion optimization_loop = SuggestRunOptimizationLoopStrategic(package, pkg_stats);
        if (!optimization_loop.action.IsEmpty()) {
            plan.steps.Add(optimization_loop);
        }

        // Calculate risk score using strategic weights and create summary
        double risk_score = ComputeRiskScoreStrategic(pkg_stats, graph_stats, Value());
        plan.summary = Format("Package '%s' shows structural issues: Risk score %.2f", package, risk_score);

        // Add strategy information to the plan
        if (active) {
            ValueMap strategy_map;
            strategy_map.Set("name", active->name);
            strategy_map.Set("description", active->description);
            strategy_map.Set("weights", active->weights);
            plan.strategy_info = strategy_map;
        } else {
            // Fallback strategy info when no strategy is active
            ValueMap fallback_strategy;
            fallback_strategy.Set("name", "default_fallback");
            fallback_strategy.Set("description", "Built-in fallback strategy with default weights");
            ValueMap default_weights;
            default_weights.Set("include_density", 1.0);
            default_weights.Set("complexity", 1.0);
            default_weights.Set("dependency_depth", 1.0);
            default_weights.Set("cycles", 1.5);
            default_weights.Set("edit_volatility", 0.5);
            fallback_strategy.Set("weights", default_weights);
            plan.strategy_info = fallback_strategy;
        }

    } catch (const Exc& e) {
        error = e;
    } catch (...) {
        error = "Unknown error occurred generating optimization plan";
    }

    return plan;
}

Vector<CoreSupervisor::Suggestion> CoreSupervisor::ComputeParetoFront(const Vector<Suggestion>& all) const {
    Vector<Suggestion> front;

    // For each suggestion, check if it's dominated by any other suggestion
    for (int i = 0; i < all.GetCount(); i++) {
        bool is_dominated = false;

        // Check against all other suggestions
        for (int j = 0; j < all.GetCount(); j++) {
            if (i == j) continue;  // Don't compare with itself

            // Check if suggestion j dominates suggestion i
            // A dominates B if:
            // - A is >= B in all *beneficial* metrics (benefit_score, confidence_score)
            // - A is <= B in all *costly* metrics (cost_score, risk_score)
            // - And A is strictly better in at least one metric
            bool dominates = true;
            bool strictly_better = false;

            // Check beneficial metrics (higher is better)
            if (all[j].benefit_score < all[i].benefit_score) {
                dominates = false;
            } else if (all[j].benefit_score > all[i].benefit_score) {
                strictly_better = true;
            }

            if (all[j].confidence_score < all[i].confidence_score) {
                dominates = false;
            } else if (all[j].confidence_score > all[i].confidence_score) {
                strictly_better = true;
            }

            // Check costly metrics (lower is better)
            if (all[j].cost_score > all[i].cost_score) {
                dominates = false;
            } else if (all[j].cost_score < all[i].cost_score) {
                strictly_better = true;
            }

            if (all[j].risk_score > all[i].risk_score) {
                dominates = false;
            } else if (all[j].risk_score < all[i].risk_score) {
                strictly_better = true;
            }

            // If j dominates i (better or equal in all metrics, and strictly better in at least one)
            if (dominates && strictly_better) {
                is_dominated = true;
                break;
            }
        }

        // If the suggestion is not dominated by any other, add to the front
        if (!is_dominated) {
            front.Add(all[i]);
        }
    }

    return front;
}

void CoreSupervisor::ComputeSuggestionMetrics(Suggestion& suggestion,
                                             const String& package,
                                             CoreIde& ide,
                                             const Value& pkg_stats,
                                             const Value& telemetry,
                                             const Value& graph_stats) const {
    // First, ensure semantic analysis has been performed and get semantic data
    String error1;
    Vector<CoreSemantic::Entity> semantic_entities;
    Vector<CoreSemantic::Cluster> semantic_clusters;

    // Try to get semantic data from the ide
    if (const_cast<CoreIde&>(ide).AnalyzeSemantics(error1)) {
        const CoreSemantic& semantic = const_cast<CoreIde&>(ide).GetSemanticAnalyzer();
        semantic_entities = semantic.GetEntities();
        semantic_clusters = semantic.GetClusters();
    }

    // Get strategy-specific objective weights or use defaults if no strategy is active
    double benefit_weight = 1.0;
    double cost_weight = 0.7;
    double risk_weight = 1.2;
    double confidence_weight = 1.0;

    if (active && const_cast<ValueMap&>(active->objective_weights).GetCount() > 0) {
        benefit_weight = (double)const_cast<ValueMap&>(active->objective_weights).Get("benefit", 1.0);
        cost_weight = (double)const_cast<ValueMap&>(active->objective_weights).Get("cost", 0.7);
        risk_weight = (double)const_cast<ValueMap&>(active->objective_weights).Get("risk", 1.2);
        confidence_weight = (double)const_cast<ValueMap&>(active->objective_weights).Get("confidence", 1.0);
    }

    // Benefit score: based on potential improvements
    double benefit_score = 0.0;

    // Calculate benefit based on the specific action
    if (suggestion.target == "cleanup_includes_and_rebuild") {
        // Benefit from reducing include complexity
        if (pkg_stats.Is<ValueMap>()) {
            ValueMap stats = pkg_stats;
            int unused_includes = (int)stats.Get("unused_includes", 0);
            double include_density = (double)stats.Get("include_density", 0.0);

            // Higher benefit for more unused includes and higher density
            benefit_score = min(1.0, (double)unused_includes * 0.05 + include_density * 0.3);
        }
    } else if (suggestion.target == "rename_symbol_safe") {
        // Benefit from consistent naming
        if (telemetry.Is<ValueMap>()) {
            ValueMap tel = telemetry;
            int max_symbol_frequency = (int)tel.Get("max_symbol_frequency", 0);
            benefit_score = min(1.0, (double)max_symbol_frequency / 500.0); // Normalize
        }
    } else if (suggestion.target == "reorganize_includes" ||
               suggestion.target == "simplify_dependency_chains") {
        // Benefit from graph simplification - now also consider semantic cluster coupling
        if (graph_stats.Is<ValueMap>()) {
            ValueMap g_stats = graph_stats;
            int max_dependency_chain = (int)g_stats.Get("max_dependency_chain", 0);
            int near_cycles = (int)g_stats.Get("near_cycles", 0);

            // Higher benefit for problematic graph structures
            benefit_score = min(1.0, (double)max_dependency_chain * 0.02 + (double)near_cycles * 0.1);
        }

        // Additionally, check semantic clusters for coupling reduction opportunities
        double coupling_reduction_potential = 0.0;
        for (const auto& cluster : semantic_clusters) {
            if (cluster.metrics.GetCount() > 0) {
                // Check coupling index - how much this cluster connects to other clusters
                double coupling_index = (double)cluster.metrics.Get("coupling_index", 0.0);
                if (coupling_index > 0.5) { // Heuristic: if coupling index is high, there's potential for improvement
                    coupling_reduction_potential += coupling_index * 0.1;
                }
            }
        }
        benefit_score += min(1.0, coupling_reduction_potential);
    } else if (suggestion.target == "optimize_package") {
        // Benefit from complexity improvement
        if (pkg_stats.Is<ValueMap>()) {
            ValueMap stats = pkg_stats;
            double complexity_score = (double)stats.Get("complexity_score", 0.0);
            double avg_complexity = (double)stats.Get("avg_file_complexity", 0.0);

            benefit_score = min(1.0, complexity_score * 0.5 + avg_complexity * 0.3);
        }
    } else {
        // Default benefit for other actions
        benefit_score = 0.5;
    }

    // Apply benefit weight
    benefit_score *= benefit_weight;

    // Cost score: estimated resources needed
    double cost_score = 0.0;

    // Estimate cost based on the action and package size
    if (pkg_stats.Is<ValueMap>()) {
        ValueMap stats = pkg_stats;
        int total_files = (int)stats.Get("files", 0);
        int total_lines = (int)stats.Get("total_lines", 0);

        // Higher cost for larger packages
        if (suggestion.target == "rename_symbol_safe") {
            // Rename operations touch many files
            cost_score = min(1.0, (double)total_files * 0.01 + (double)total_lines / 10000.0);
        } else if (suggestion.target == "reorganize_includes" ||
                   suggestion.target == "simplify_dependency_chains") {
            // Dependency changes may affect many files
            cost_score = min(1.0, (double)total_files * 0.005 + (double)total_lines / 15000.0);
        } else if (suggestion.target == "cleanup_includes_and_rebuild") {
            // Cleanup operations typically lower cost
            cost_score = min(1.0, (double)total_files * 0.002 + (double)total_lines / 20000.0);
        } else {
            cost_score = min(1.0, (double)total_files * 0.003 + (double)total_lines / 15000.0);
        }
    }

    // Adjust cost based on semantic information
    // Large clusters may be more expensive to modify due to high coupling
    double semantic_cost_modifier = 1.0;
    for (const auto& cluster : semantic_clusters) {
        if (cluster.metrics.GetCount() > 0) {
            int cluster_size = (int)const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("size", 0);
            double coupling_index = (double)const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("coupling_index", 0.0);

            // Large and highly coupled clusters are more expensive to modify
            if (cluster_size > 10 && coupling_index > 0.7) {
                semantic_cost_modifier *= 1.3; // Increase cost by 30%
            }
        }
    }
    cost_score *= semantic_cost_modifier;

    // Apply cost weight
    cost_score *= cost_weight;

    // Risk score: probability of breaking things (reusing existing risk computation)
    double risk_score = ComputeRiskScoreStrategic(pkg_stats, graph_stats, Value());

    // Adjust risk based on semantic information
    // Modify risk based on cluster metrics
    for (const auto& cluster : semantic_clusters) {
        if (cluster.metrics.GetCount() > 0) {
            int cluster_size = (int)const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("size", 0);
            double avg_complexity = (double)const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("avg_complexity", 0.0);

            // Large clusters with high complexity are riskier to modify
            if (cluster_size > 20) {
                risk_score *= 1.2; // Increase risk by 20%
            }
            if (avg_complexity > 10.0) {
                risk_score *= 1.1; // Increase risk by 10%
            }
        }
    }

    // Apply risk weight
    risk_score *= risk_weight;

    // Confidence score: based on signal clarity
    double confidence_score = 0.5; // Base confidence

    if (telemetry.Is<ValueMap>()) {
        ValueMap tel = telemetry;
        int telemetry_signals = 0;

        // Count valid telemetry signals
        String hot_symbol = tel.Get("hot_symbol", String());
        if (!hot_symbol.IsEmpty()) telemetry_signals++;
        if ((int)tel.Get("max_symbol_frequency", 0) > 0) telemetry_signals++;
        if ((int)tel.Get("edit_volatility_score", 0) > 0) telemetry_signals++;

        // Normalize confidence based on signals
        confidence_score = min(1.0, 0.3 + (double)telemetry_signals * 0.2);
    } else {
        // Lower confidence if no telemetry data
        confidence_score = 0.3;
    }

    // Boost confidence if semantic information is available and consistent
    if (semantic_clusters.GetCount() > 0) {
        confidence_score *= 1.1; // Boost by 10% when semantic analysis data is available
    }

    // Apply confidence weight
    confidence_score *= confidence_weight;

    // Update the suggestion with computed scores
    suggestion.benefit_score = min(1.0, benefit_score);
    suggestion.cost_score = min(1.0, cost_score);
    suggestion.risk_score = min(1.0, risk_score);
    suggestion.confidence_score = min(1.0, confidence_score);

    // Populate metrics map with arbitrary computed metrics
    ValueMap metrics;
    metrics.Set("impact", suggestion.benefit_score / max(suggestion.cost_score, 0.01));
    metrics.Set("surface_area", (int)pkg_stats["files"]);
    metrics.Set("graph_delta", 0); // Placeholder - would indicate graph structure change

    // NEW: Enhance metrics with semantic inference data
    String error;
    if (const_cast<CoreIde&>(ide).AnalyzeSemantics(error)) {
        // Check if this suggestion affects entities in multiple subsystems (increases cost/risk)
        const auto& subsystems = const_cast<CoreIde&>(ide).GetSemanticAnalyzer().GetSubsystems();
        int subsystems_affected = 0;
        String target_package = (String)suggestion.params["package"]; // Assuming package is in params

        // Identify which subsystems are affected by this suggestion
        for (const auto& subsystem : subsystems) {
            bool affects_this_subsystem = false;

            // Check if any entity in this subsystem is related to the suggestion target
            for (const auto& entity_name : subsystem.entities) {
                // For now, we'll check if the target package contains the entity name
                // In a more sophisticated implementation, this would check actual relationships
                if (target_package.Find(entity_name) >= 0) {
                    affects_this_subsystem = true;
                    break;
                }
            }

            if (affects_this_subsystem) {
                subsystems_affected++;
            }
        }

        metrics.Set("subsystems_affected", subsystems_affected);

        // Adjust scores based on subsystem cohesion and coupling
        if (subsystems_affected > 1) {
            // Touching multiple subsystems increases risk
            suggestion.risk_score = min(1.0, suggestion.risk_score * 1.5);
            // And potentially increases cost
            suggestion.cost_score = min(1.0, suggestion.cost_score * 1.2);
        }

        // Check if the target entity has a fragile role (like parser or controller)
        const auto& entities = const_cast<CoreIde&>(ide).GetSemanticAnalyzer().GetEntities();
        // Check if there's a specific target entity in the suggestion params
        String target_entity_param = (String)suggestion.params["symbol"]; // Looking for symbol parameter
        if (target_entity_param.IsEmpty()) {
            target_entity_param = (String)suggestion.target; // Or target itself might be an entity name
        }

        for (const auto& entity : entities) {
            if (entity.name == target_entity_param) {
                String role = entity.role;
                if (role == "parser" || role == "controller" || role == "core_component") {
                    // Operations on fragile roles increase risk
                    suggestion.risk_score = min(1.0, suggestion.risk_score * 1.3);
                }

                // Increase benefit score if the suggestion improves high-level layers
                String layer = entity.layer_dependency;
                if (layer == "base" || layer == "core") {
                    // Improvements to foundational layers have higher benefit
                    suggestion.benefit_score = min(1.0, suggestion.benefit_score * 1.1);
                }

                break;
            }
        }

        // Increase benefit score if the suggestion helps reduce coupling between subsystems
        // This is heuristic-based - in real implementation, we'd check if the action
        // specifically simplifies architectural layering or reduces cross-subsystem dependencies
        if (suggestion.target == "reorganize_includes" ||
            suggestion.target == "simplify_dependency_chains" ||
            suggestion.target == "resolve_cycles") {
            // These actions potentially improve subsystem structure
            suggestion.benefit_score = min(1.0, suggestion.benefit_score * 1.1);
        }
    }

    suggestion.metrics = metrics;
}

// Helper method to populate semantic snapshot for the plan
void CoreSupervisor::PopulateSemanticSnapshot(const CoreIde& ide, Plan& plan) const {
    String error1;
    if (const_cast<CoreIde&>(ide).AnalyzeSemantics(error1)) {
        const CoreSemantic& semantic = const_cast<CoreIde&>(ide).GetSemanticAnalyzer();
        const auto& clusters = semantic.GetClusters();
        const auto& entities = semantic.GetEntities();
        const auto& subsystems = semantic.GetSubsystems(); // NEW: Include subsystems

        // Populate snapshot with key semantic metrics
        plan.semantic_snapshot.Set("cluster_count", clusters.GetCount());
        plan.semantic_snapshot.Set("entity_count", entities.GetCount());
        plan.semantic_snapshot.Set("subsystem_count", subsystems.GetCount()); // NEW: Add subsystem count

        // NEW: Calculate subsystem metrics
        if (subsystems.GetCount() > 0) {
            // Find largest subsystem size
            int largest_subsystem_size = 0;
            double avg_subsystem_cohesion = 0.0;
            double avg_subsystem_coupling = 0.0;

            for (const auto& subsystem : subsystems) {
                int size = const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("size", 0);
                if (size > largest_subsystem_size) {
                    largest_subsystem_size = size;
                }

                double cohesion = const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("cohesion_score", 0.0);
                avg_subsystem_cohesion += cohesion;

                double coupling = const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("coupling_score", 0.0);
                avg_subsystem_coupling += coupling;
            }

            avg_subsystem_cohesion /= subsystems.GetCount();
            avg_subsystem_coupling /= subsystems.GetCount();

            plan.semantic_snapshot.Set("largest_subsystem_size", largest_subsystem_size);
            plan.semantic_snapshot.Set("avg_subsystem_cohesion", avg_subsystem_cohesion);
            plan.semantic_snapshot.Set("avg_subsystem_coupling", avg_subsystem_coupling);
        }

        // Find largest cluster size
        int largest_cluster_size = 0;
        for (const auto& cluster : clusters) {
            int size = const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("size", 0);
            if (size > largest_cluster_size) {
                largest_cluster_size = size;
            }
        }
        plan.semantic_snapshot.Set("largest_cluster_size", largest_cluster_size);

        // Calculate centrality scores for top entities
        ValueArray top_entities;
        for (const auto& entity : entities) {
            ValueMap entity_info;
            entity_info.Set("name", entity.name);
            entity_info.Set("fanout", const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fanout", 0));
            entity_info.Set("fannin", const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fannin", 0));

            // NEW: Add inferred relation information to entities
            entity_info.Set("layer_dependency", entity.layer_dependency);
            entity_info.Set("role", entity.role);

            // Calculate centrality as a combination of fanout and fannin
            int fanout = const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fanout", 0);
            int fannin = const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fannin", 0);
            double centrality = sqrt((double)fanout * fannin);
            entity_info.Set("centrality", centrality);

            top_entities.Add(entity_info);
        }

        // Sort entities by centrality and take top 10
        Sort(top_entities, [](const Value& a, const Value& b) {
            return (double)a["centrality"] > (double)b["centrality"];
        });

        if (top_entities.GetCount() > 10) {
            top_entities.SetCount(10);
        }
        plan.semantic_snapshot.Set("top_entities_by_centrality", top_entities);

        // NEW: Include role distribution across the codebase
        ValueMap role_distribution;
        for (const auto& entity : entities) {
            String role = entity.role;
            if (role.IsEmpty()) role = "utility"; // Default role
            int count = role_distribution.Get(role, 0);
            role_distribution.Set(role, count + 1);
        }
        plan.semantic_snapshot.Set("role_distribution", role_distribution);

        // NEW: Include layer distribution across the codebase
        ValueMap layer_distribution;
        for (const auto& entity : entities) {
            String layer = entity.layer_dependency;
            if (!layer.IsEmpty()) {
                int count = layer_distribution.Get(layer, 0);
                layer_distribution.Set(layer, count + 1);
            }
        }
        plan.semantic_snapshot.Set("layer_distribution", layer_distribution);

        // NEW: Identify potential "god components" based on roles and complexity
        ValueArray god_components;
        for (const auto& entity : entities) {
            String role = entity.role;
            int fannin = const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fannin", 0);
            int fanout = const_cast<ValueMap&>(const_cast<CoreSemantic::Entity&>(entity)).attributes.Get("fanout", 0);

            // If role is "god_component" or has too many connections
            if (role == "god_component" || (fannin > 10 && fanout > 10)) {
                ValueMap entity_info;
                entity_info.Set("name", entity.name);
                entity_info.Set("role", entity.role);
                entity_info.Set("fannin", fannin);
                entity_info.Set("fanout", fanout);
                entity_info.Set("layer_dependency", entity.layer_dependency);
                god_components.Add(entity_info);
            }
        }
        plan.semantic_snapshot.Set("potential_god_components", god_components);

        // Detect "god classes" or clusters with extremely high metrics
        ValueArray god_clusters;
        for (const auto& cluster : clusters) {
            int size = const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("size", 0);
            double avg_complexity = const_cast<ValueMap&>(const_cast<CoreSemantic::Cluster&>(cluster)).metrics.Get("avg_complexity", 0.0);

            // Heuristic: clusters that are large and have high average complexity
            if (size > 15 && avg_complexity > 8.0) {
                ValueMap cluster_info;
                cluster_info.Set("name", cluster.name);
                cluster_info.Set("size", size);
                cluster_info.Set("avg_complexity", avg_complexity);
                god_clusters.Add(cluster_info);
            }
        }
        plan.semantic_snapshot.Set("potential_god_clusters", god_clusters);

        // NEW: Add subsystems to the semantic snapshot
        ValueArray subsystems_array;
        for (const auto& subsystem : subsystems) {
            ValueMap subsystem_info;
            subsystem_info.Set("name", subsystem.name);
            subsystem_info.Set("size", const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("size", 0));
            subsystem_info.Set("cohesion_score", const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("cohesion_score", 0.0));
            subsystem_info.Set("coupling_score", const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("coupling_score", 0.0));
            subsystem_info.Set("complexity_sum", const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("complexity_sum", 0));

            // Include role distribution within this subsystem
            Value role_dist = const_cast<ValueMap&>(const_cast<CoreSemantic::Subsystem&>(subsystem)).metrics.Get("role_distribution", ValueMap());
            subsystem_info.Set("role_distribution", role_dist);

            subsystems_array.Add(subsystem_info);
        }
        plan.semantic_snapshot.Set("subsystems", subsystems_array);
    }
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

        // Generate suggestions based on strategic heuristics
        Suggestion include_cleanup = SuggestIncludeCleanupStrategic(package, pkg_stats);
        if (!include_cleanup.action.IsEmpty()) {
            ComputeSuggestionMetrics(include_cleanup, package, ide, pkg_stats, telemetry, graph_stats);
            plan.steps.Add(include_cleanup);
        }

        Suggestion rename_hotspot = SuggestRenameHotspotStrategic(package, telemetry, ide);
        if (!rename_hotspot.action.IsEmpty()) {
            ComputeSuggestionMetrics(rename_hotspot, package, ide, pkg_stats, telemetry, graph_stats);
            plan.steps.Add(rename_hotspot);
        }

        Suggestion graph_simplification = SuggestGraphSimplificationStrategic(package, graph_stats);
        if (!graph_simplification.action.IsEmpty()) {
            ComputeSuggestionMetrics(graph_simplification, package, ide, pkg_stats, telemetry, graph_stats);
            plan.steps.Add(graph_simplification);
        }

        Suggestion optimization_loop = SuggestRunOptimizationLoopStrategic(package, pkg_stats);
        if (!optimization_loop.action.IsEmpty()) {
            ComputeSuggestionMetrics(optimization_loop, package, ide, pkg_stats, telemetry, graph_stats);
            plan.steps.Add(optimization_loop);
        }

        // Calculate risk score using strategic weights and create summary
        double risk_score = ComputeRiskScoreStrategic(pkg_stats, graph_stats, Value());
        plan.summary = Format("Package '%s' shows structural issues: Risk score %.2f", package, risk_score);

        // Add strategy information to the plan
        if (active) {
            ValueMap strategy_map;
            strategy_map.Set("name", active->name);
            strategy_map.Set("description", active->description);
            strategy_map.Set("weights", active->weights);
            plan.strategy_info = strategy_map;
        } else {
            // Fallback strategy info when no strategy is active
            ValueMap fallback_strategy;
            fallback_strategy.Set("name", "default_fallback");
            fallback_strategy.Set("description", "Built-in fallback strategy with default weights");
            ValueMap default_weights;
            default_weights.Set("include_density", 1.0);
            default_weights.Set("complexity", 1.0);
            default_weights.Set("dependency_depth", 1.0);
            default_weights.Set("cycles", 1.5);
            default_weights.Set("edit_volatility", 0.5);
            fallback_strategy.Set("weights", default_weights);
            plan.strategy_info = fallback_strategy;
        }

        // Populate semantic snapshot with semantic analysis data
        PopulateSemanticSnapshot(ide, plan);

    } catch (const Exc& e) {
        error = e;
    } catch (...) {
        error = "Unknown error occurred generating optimization plan";
    }

    return plan;
}

CoreSupervisor::Plan CoreSupervisor::GenerateWorkspacePlan(CoreIde& ide, String& error) {
    Plan plan;

    try {
        // Get overall workspace statistics
        Value workspace_stats = ide.GetWorkspaceStats(error);
        if (!error.IsEmpty()) {
            return plan;
        }

        Value graph_stats = ide.GetGraphStats(error);
        if (!error.IsEmpty()) {
            return plan;
        }

        // For now, we'll add workspace-level suggestions based on overall statistics
        // This is a simplified implementation that could be expanded with more complex logic

        // Check for global workspace issues
        if (graph_stats.Is<ValueMap>()) {
            ValueMap g_stats = graph_stats;
            int dependency_depth = (int)g_stats.Get("max_chain", 0);
            bool has_cycles = (bool)g_stats.Get("cycles", false);
            int edge_count = (int)g_stats.Get("edges", 0);

            // Add suggestion for dependency graph issues
            if (has_cycles) {
                Suggestion suggestion;
                suggestion.action = "run_playbook";
                suggestion.target = "resolve_cycles";
                ValueMap params;
                params.Set("type", "workspace");
                params.Set("issue", "dependency_cycles");
                suggestion.params = params;
                suggestion.reason = "Dependency cycles detected in workspace graph.";
                suggestion.benefit_score = 0.0;
                suggestion.cost_score = 0.0;
                suggestion.risk_score = 0.0;
                suggestion.confidence_score = 0.0;
                // Since this is a workspace-level suggestion, we pass empty values for package-specific data
                ComputeSuggestionMetrics(suggestion, "workspace", ide, workspace_stats, Value(), graph_stats);
                plan.steps.Add(suggestion);
            } else if (dependency_depth > 10) { // Heuristic threshold
                Suggestion suggestion;
                suggestion.action = "run_playbook";
                suggestion.target = "simplify_dependency_chains";
                ValueMap params;
                params.Set("type", "workspace");
                params.Set("max_depth", dependency_depth);
                suggestion.params = params;
                suggestion.reason = "Long dependency chains detected in workspace graph.";
                suggestion.benefit_score = 0.0;
                suggestion.cost_score = 0.0;
                suggestion.risk_score = 0.0;
                suggestion.confidence_score = 0.0;
                // Since this is a workspace-level suggestion, we pass empty values for package-specific data
                ComputeSuggestionMetrics(suggestion, "workspace", ide, workspace_stats, Value(), graph_stats);
                plan.steps.Add(suggestion);
            }
        }

        if (workspace_stats.Is<ValueMap>()) {
            ValueMap ws_stats = workspace_stats;
            int packages_count = (int)ws_stats.Get("packages", 0);
            int total_files = (int)ws_stats.Get("files", 0);
            double avg_size = (double)ws_stats.Get("average_size", 0.0);

            // Add suggestions based on workspace metrics if they exceed thresholds
            if (total_files > 10000) { // Heuristic threshold for large workspaces
                Suggestion suggestion;
                suggestion.action = "run_playbook";
                suggestion.target = "workspace_optimization";
                ValueMap params;
                params.Set("type", "workspace");
                params.Set("file_count", total_files);
                suggestion.params = params;
                suggestion.reason = "Large number of files detected in workspace.";
                suggestion.benefit_score = 0.0;
                suggestion.cost_score = 0.0;
                suggestion.risk_score = 0.0;
                suggestion.confidence_score = 0.0;
                // Since this is a workspace-level suggestion, we pass empty values for package-specific data
                ComputeSuggestionMetrics(suggestion, "workspace", ide, workspace_stats, Value(), graph_stats);
                plan.steps.Add(suggestion);
            }
        }

        // Calculate overall risk score for the workspace
        double risk_score = ComputeRiskScoreStrategic(workspace_stats, graph_stats, Value());

        plan.summary = Format("Workspace shows overall issues: %d packages, %d files, risk score %.2f",
                              (int)workspace_stats["packages"], (int)workspace_stats["files"], risk_score);

        // Add strategy information to the plan
        if (active) {
            ValueMap strategy_map;
            strategy_map.Set("name", active->name);
            strategy_map.Set("description", active->description);
            strategy_map.Set("weights", active->weights);
            plan.strategy_info = strategy_map;
        } else {
            // Fallback strategy info when no strategy is active
            ValueMap fallback_strategy;
            fallback_strategy.Set("name", "default_fallback");
            fallback_strategy.Set("description", "Built-in fallback strategy with default weights");
            ValueMap default_weights;
            default_weights.Set("include_density", 1.0);
            default_weights.Set("complexity", 1.0);
            default_weights.Set("dependency_depth", 1.0);
            default_weights.Set("cycles", 1.5);
            default_weights.Set("edit_volatility", 0.5);
            fallback_strategy.Set("weights", default_weights);
            plan.strategy_info = fallback_strategy;
        }

        // Populate semantic snapshot with semantic analysis data
        PopulateSemanticSnapshot(ide, plan);

    } catch (const Exc& e) {
        error = e;
    } catch (...) {
        error = "Unknown error occurred generating workspace plan";
    }

    return plan;
}