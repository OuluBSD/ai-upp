#ifndef _clicore_CoreSupervisor_h_
#define _clicore_CoreSupervisor_h_

#include <Core/Core.h>
#include "StrategyProfile.h"
#include "CoreSemantic.h"
#include "ProjectMemory.h"  // Include for ProjectMemory
#include "GlobalKnowledge.h"  // Include for GlobalKnowledge

class CoreIde;
class CoreScenario;  // Forward declaration to avoid circular dependency

class CoreSupervisor : Moveable<CoreSupervisor> {
public:
    struct Suggestion : Moveable<Suggestion> {
        String action;             // e.g., "run_playbook"
        String target;             // e.g., "cleanup_includes_and_rebuild"
        Value params;              // param map
        String reason;             // explanation (human-readable)

        // Multi-objective fields added in v3
        double benefit_score;      // improvement expected
        double cost_score;         // estimated implementation cost
        double risk_score;         // risk of breakage or refactor impact
        double confidence_score;   // heuristic confidence level
        ValueMap metrics;          // arbitrary computed metrics (extensible)
    };

    struct Plan : Moveable<Plan> {
        Vector<Suggestion> steps;
        String summary;
        Value strategy_info;  // Contains name, description, weights of the strategy used
        ValueMap semantic_snapshot;  // Semantic information for multi-objective scoring
    };

    // Helper method to compute multi-objective metrics for a suggestion
    void ComputeSuggestionMetrics(Suggestion& suggestion,
                                 const String& package,
                                 CoreIde& ide,
                                 const Value& pkg_stats,
                                 const Value& telemetry,
                                 const Value& graph_stats) const;

    // Method to compute Pareto front of suggestions
    Vector<Suggestion> ComputeParetoFront(const Vector<Suggestion>& all) const;

    // Helper method to populate semantic snapshot for the plan
    void PopulateSemanticSnapshot(const CoreIde& ide, Plan& plan) const;

    CoreSupervisor();

    void SetStrategyRegistry(const StrategyRegistry* reg);
    bool SetActiveStrategy(const String& name, String& error);
    const StrategyProfile* GetActiveStrategy() const;

    // Top-level entries: generate improvement plan for a package or workspace
    Plan GenerateOptimizationPlan(const String& package,
                                  CoreIde& ide,
                                  String& error);

    Plan GenerateWorkspacePlan(CoreIde& ide, String& error);

    // NEW: Build a scenario plan from suggestions
    Value BuildScenario(const String& package,
                       int max_actions,
                       CoreIde& ide,
                       String& error);

    // Adaptive Priority Engine (APE) v4 methods
    void UpdateAdaptiveWeights(const ProjectMemory& mem);
    double PredictValue(const Suggestion& s, const ProjectMemory& mem) const;

    // Meta-Supervisor extension for CWI v1
    void UpdateMetaWeights(const GlobalKnowledge& gk);

    // Needed for supervisor_predict command
    AdaptiveWeights GetAdaptiveWeightsForTesting() const { return adaptive; }

private:
    Suggestion SuggestIncludeCleanup(const String& package,
                                     const Value& pkg_stats) const;

    Suggestion SuggestRenameHotspot(const String& package,
                                    const Value& telemetry,
                                    CoreIde& ide) const;

    Suggestion SuggestGraphSimplification(const String& package,
                                          const Value& graph_stats) const;

    Suggestion SuggestRunOptimizationLoop(const String& package,
                                          const Value& pkg_stats) const;

    double ComputeRiskScore(const Value& pkg_stats,
                            const Value& graph_stats,
                            const Value& file_complexity) const;

    // New methods for strategy-aware heuristics
    Suggestion SuggestIncludeCleanupStrategic(const String& package,
                                              const Value& pkg_stats) const;

    Suggestion SuggestRenameHotspotStrategic(const String& package,
                                             const Value& telemetry,
                                             CoreIde& ide) const;

    Suggestion SuggestGraphSimplificationStrategic(const String& package,
                                                   const Value& graph_stats) const;

    Suggestion SuggestRunOptimizationLoopStrategic(const String& package,
                                                   const Value& pkg_stats) const;

    double ComputeRiskScoreStrategic(const Value& pkg_stats,
                                     const Value& graph_stats,
                                     const Value& file_complexity) const;

    // Members for strategy management
    const StrategyRegistry* registry = nullptr;
    const StrategyProfile* active = nullptr;

    // Adaptive Priority Engine (APE) v4 members
    bool ape_enabled = true;

    struct AdaptiveWeights {
        double benefit_multiplier = 1.0;
        double risk_penalty = 1.0;
        double confidence_boost = 1.0;
        double novelty_bias = 0.1;
    };
    AdaptiveWeights adaptive;

    // Meta-Supervisor extension for CWI v1
    struct MetaWeights {
        double pattern_success_bias = 0.0;
        double refactor_success_bias = 0.0;
        double topology_risk_adjustment = 0.0;
    };
    MetaWeights meta;
};

#endif