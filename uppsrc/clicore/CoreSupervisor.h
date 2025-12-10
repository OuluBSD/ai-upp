#ifndef _clicore_CoreSupervisor_h_
#define _clicore_CoreSupervisor_h_

#include <Core/Core.h>
#include "StrategyProfile.h"
#include "CoreSemantic.h"
#include "ProjectMemory.h"  // Include for ProjectMemory
#include "GlobalKnowledge.h"  // Include for GlobalKnowledge
#include "LifecycleModel.h"   // Include for LifecyclePhase and LifecycleModel
#include "TemporalSeasonality.h"  // Include for StabilityWindow, SeasonalityPattern, ReleaseCadence
#include "TemporalForecast.h"     // Include for RiskProfile

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
    double PredictValue(const Suggestion& s, const ProjectMemory& mem, const LifecyclePhase& current_phase) const;
    double PredictValue(const Suggestion& s,
                       const ProjectMemory& mem,
                       const DriftMetrics& drift,
                       double stability_index) const;

    // Temporal Strategy Engine v1 - Overload of PredictValue that takes temporal seasonality into account
    double PredictValue(const Suggestion& s,
                       const ProjectMemory& mem,
                       const Vector<StabilityWindow>& stability_windows,
                       const Vector<SeasonalityPattern>& seasonality_patterns,
                       const ReleaseCadence& cadence) const;

    // Meta-Supervisor extension for CWI v1
    void UpdateMetaWeights(const GlobalKnowledge& gk);


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

    // Lifecycle Supervisor v1 - Weights for different lifecycle phases
    struct LifecycleWeights {
        double legacy_risk_multiplier = 1.5;
        double mature_conservatism_bias = 0.3;
        double growth_refactor_bonus = 0.4;
        double decline_alertness_factor = 0.6;
    };
    LifecycleWeights lw;

    // Lifecycle Supervisor v2 - Weights for phase drift aware decisions
    struct DriftWeights {
        double high_drift_conservatism;
        double low_stability_risk_amplifier;
        double stable_refactor_release;
    };
    DriftWeights dw;

    // Temporal Strategy Engine v1 - Weights for temporal reasoning
    struct TemporalWeights {
        double avoid_crunch_multiplier;
        double prefer_stability_bonus;
        double release_cycle_alignment;
    };
    TemporalWeights temporal;

    // Temporal Strategy Engine v2 - Weights for risk-aware decisions
    struct RiskWeights {
        double avoid_high_risk = 0.8;
        double reward_low_volatility = 0.3;
        double shock_sensitivity = 0.5;
    };
    RiskWeights risk_weights;

    // Temporal Strategy Engine v2 - Overload of PredictValue that takes risk profile into account
    double PredictValue(const Suggestion& s,
                       const ProjectMemory& mem,
                       const RiskProfile& risk_profile) const;

    // Method to update drift-aware weights
    void UpdateDriftWeights(const DriftMetrics& drift,
                            double stability_index);
};

#endif