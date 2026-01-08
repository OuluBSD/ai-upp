#include "CoreFutureSimulator.h"
#include "CoreScenario.h"
#include "CoreTelemetry.h"
#include "CoreGraph.h"
#include "CoreSemantic.h"

NAMESPACE_UPP

namespace TemporalDynamics {
    struct Trend {
        double slope = 0.0;
        double volatility = 0.0;
        double direction = 0.0;
    };
}

CoreFutureSimulator::CoreFutureSimulator() {
}

CoreFutureSimulator::OutcomeHorizon CoreFutureSimulator::Explore(
    const CoreScenario::ScenarioPlan& base_plan,
    const Vector<CoreScenario::ScenarioAction>& negotiated_actions,
    const LifecyclePhase& phase,
    const TemporalDynamics::Trend& trend,
    const TemporalSeasonality::StabilityWindow& window) {
    
    OutcomeHorizon horizon;
    
    // Generate different branches based on the base plan and negotiated actions
    Vector<String> branch_types = {
        "all_actions",
        "low_risk", 
        "high_benefit",
        "stability_aligned",
        "minimal_change"
    };
    
    for (const String& branch_type : branch_types) {
        FutureBranch branch = BuildBranch(branch_type, base_plan, negotiated_actions, 
                                          phase, trend, window);
        horizon.branches.Add(AsValue(branch));
    }
    
    // Find the best branch (highest score)
    double best_score = -1.0;
    int best_idx = -1;
    for (int i = 0; i < horizon.branches.GetCount(); i++) {
        const FutureBranch& branch = static_cast<const FutureBranch&>(horizon.branches[i]);
        if (branch.score > best_score) {
            best_score = branch.score;
            best_idx = i;
        }
    }
    
    if (best_idx >= 0) {
        const FutureBranch& best_branch = static_cast<const FutureBranch&>(horizon.branches[best_idx]);
        ValueMap best_summary;
        best_summary.Set("id", best_branch.id);
        best_summary.Set("score", best_branch.score);
        best_summary.Set("projected_metrics", best_branch.projected_metrics);
        horizon.best_branch = best_summary;
    }
    
    // Compute overall statistics for the horizon
    ValueMap stats;
    double total_score = 0, min_score = 10.0, max_score = -10.0;
    double total_risk = 0, min_risk = 10.0, max_risk = -10.0;
    double total_benefit = 0, min_benefit = 10.0, max_benefit = -10.0;
    int risk_count = 0, benefit_count = 0;
    
    for (int i = 0; i < horizon.branches.GetCount(); i++) {
        const FutureBranch& branch = static_cast<const FutureBranch&>(horizon.branches[i]);
        
        total_score += branch.score;
        if (branch.score < min_score) min_score = branch.score;
        if (branch.score > max_score) max_score = branch.score;
        
        if (branch.projected_metrics.Find("risk") >= 0) {
            double risk = (double)branch.projected_metrics.Get("risk", 0.0);
            total_risk += risk;
            risk_count++;
            if (risk < min_risk) min_risk = risk;
            if (risk > max_risk) max_risk = risk;
        }
        
        if (branch.projected_metrics.Find("benefit") >= 0) {
            double benefit = (double)branch.projected_metrics.Get("benefit", 0.0);
            total_benefit += benefit;
            benefit_count++;
            if (benefit < min_benefit) min_benefit = benefit;
            if (benefit > max_benefit) max_benefit = benefit;
        }
    }
    
    stats.Set("branch_count", horizon.branches.GetCount());
    stats.Set("avg_score", horizon.branches.GetCount() > 0 ? total_score / horizon.branches.GetCount() : 0.0);
    stats.Set("min_score", min_score <= 10.0 ? min_score : 0.0);
    stats.Set("max_score", max_score >= -10.0 ? max_score : 0.0);
    stats.Set("avg_risk", risk_count > 0 ? total_risk / risk_count : 0.5);
    stats.Set("min_risk", min_risk <= 10.0 ? min_risk : 0.0);
    stats.Set("max_risk", max_risk >= -10.0 ? max_risk : 1.0);
    stats.Set("avg_benefit", benefit_count > 0 ? total_benefit / benefit_count : 0.5);
    stats.Set("min_benefit", min_benefit <= 10.0 ? min_benefit : 0.0);
    stats.Set("max_benefit", max_benefit >= -10.0 ? max_benefit : 1.0);
    
    horizon.stats = stats;
    
    return horizon;
}

CoreFutureSimulator::FutureBranch CoreFutureSimulator::BuildBranch(
    const String& id,
    const CoreScenario::ScenarioPlan& base_plan,
    const Vector<CoreScenario::ScenarioAction>& actions,
    const LifecyclePhase& phase,
    const TemporalDynamics::Trend& trend,
    const TemporalSeasonality::StabilityWindow& window) {
    
    FutureBranch branch;
    branch.id = "branch_" + id;
    branch.starting_point = base_plan.metadata; // Use metadata as starting point
    
    if (id == "all_actions") {
        branch.actions = AsValueArray(actions);
    } 
    else if (id == "low_risk") {
        // Select only actions with low risk
        for (const auto& action : actions) {
            // Heuristic: assume risk is in params, if not present default to medium risk
            double risk = (double)action.params.Get("risk", 0.5);
            if (risk < 0.5) {
                branch.actions.Add(AsValue(action));
            }
        }
    }
    else if (id == "high_benefit") {
        // Sort actions by benefit and take top N
        Vector<CoreScenario::ScenarioAction> sorted_actions = actions;
        // Sort in descending order by benefit
        for(int i = 0; i < sorted_actions.GetCount(); i++) {
            for(int j = i + 1; j < sorted_actions.GetCount(); j++) {
                double benefit_i = (double)sorted_actions[i].params.Get("benefit", 0.0);
                double benefit_j = (double)sorted_actions[j].params.Get("benefit", 0.0);
                if(benefit_i < benefit_j) {
                    Swap(sorted_actions[i], sorted_actions[j]);
                }
            }
        }
        // Take top 50% of actions (or minimum of 1)
        int count = std::max(1, (int)(sorted_actions.GetCount() * 0.5));
        for(int i = 0; i < std::min(count, sorted_actions.GetCount()); i++) {
            branch.actions.Add(AsValue(sorted_actions[i]));
        }
    }
    else if (id == "stability_aligned") {
        // Select actions that match current stability window
        for (const auto& action : actions) {
            double stability_req = (double)action.params.Get("stability_requirement", 0.5);
            if (stability_req >= window.lower_bound && stability_req <= window.upper_bound) {
                branch.actions.Add(AsValue(action));
            }
        }
    }
    else if (id == "minimal_change") {
        // Select actions that provide non-trivial improvement with minimal effort
        for (const auto& action : actions) {
            double benefit = (double)action.params.Get("benefit", 0.0);
            double effort = (double)action.params.Get("effort", 1.0);
            // Heuristic: benefit/effort ratio above average
            if (benefit > 0.3 && effort < 0.7) {
                branch.actions.Add(AsValue(action));
            }
        }
    }
    
    // Predict metrics based on selected actions
    branch.projected_metrics = PredictMetrics(base_plan, 
                                             (Vector<CoreScenario::ScenarioAction>)branch.actions,
                                             phase, trend);
    
    branch.score = ScoreBranch(branch.projected_metrics, phase, trend, window);
    
    return branch;
}

ValueMap CoreFutureSimulator::PredictMetrics(
    const CoreScenario::ScenarioPlan& base_plan,
    const Vector<CoreScenario::ScenarioAction>& actions,
    const LifecyclePhase& phase,
    const TemporalDynamics::Trend& trend) {
    
    ValueMap metrics;
    
    // Initialize heuristics based on the phase and trend
    double base_risk = 0.5;
    double base_benefit = 0.5;
    double base_complexity = 0.3;
    double base_coupling = 0.2;
    double base_cycles = 0.1;
    
    // Adjust metrics based on lifecycle phase characteristics
    if (phase.stability < 0.3) {
        // Unstable phase - higher risk
        base_risk += 0.2;
        base_complexity += 0.1;
    } else if (phase.stability > 0.7) {
        // Stable phase - lower risk
        base_risk -= 0.1;
    }
    
    // Adjust based on trend volatility
    base_risk += trend.volatility * 0.1;
    
    // Calculate aggregate metrics based on actions
    double total_risk = base_risk;
    double total_benefit = base_benefit;
    double total_complexity = base_complexity;
    double total_coupling = base_coupling;
    double total_cycles = base_cycles;
    
    for (const auto& action : actions) {
        // Adjust metrics based on individual action properties
        double action_risk = (double)action.params.Get("risk", 0.5);
        double action_benefit = (double)action.params.Get("benefit", 0.3);
        double action_complexity = (double)action.params.Get("complexity", 0.2);
        double action_coupling = (double)action.params.Get("coupling", 0.1);
        
        total_risk += action_risk * 0.1;  // Weighted addition
        total_benefit += action_benefit * 0.15;
        total_complexity += action_complexity * 0.1;
        total_coupling += action_coupling * 0.1;
        
        // Cycle creation is more likely with complex changes
        total_cycles += action_complexity * action_risk * 0.05;
    }
    
    // Normalize metrics to [0, 1] range
    total_risk = min(1.0, max(0.0, total_risk));
    total_benefit = min(1.0, max(0.0, total_benefit));
    total_complexity = min(1.0, max(0.0, total_complexity));
    total_coupling = min(1.0, max(0.0, total_coupling));
    total_cycles = min(1.0, max(0.0, total_cycles));
    
    metrics.Set("risk", total_risk);
    metrics.Set("benefit", total_benefit);
    metrics.Set("complexity", total_complexity);
    metrics.Set("coupling", total_coupling);
    metrics.Set("cycles", total_cycles);
    metrics.Set("complexity_delta", total_complexity - base_complexity);
    metrics.Set("coupling_delta", total_coupling - base_coupling);
    metrics.Set("entropy_delta", (total_risk + total_complexity) / 2 - phase.entropy);
    
    return metrics;
}

double CoreFutureSimulator::ScoreBranch(
    const ValueMap& metrics,
    const LifecyclePhase& phase,
    const TemporalDynamics::Trend& trend,
    const TemporalSeasonality::StabilityWindow& window) {
    
    // Extract metrics
    double risk = (double)metrics.Get("risk", 0.5);
    double benefit = (double)metrics.Get("benefit", 0.5);
    double complexity = (double)metrics.Get("complexity", 0.3);
    double coupling = (double)metrics.Get("coupling", 0.2);
    
    // Score calculation based on phase requirements
    double score = 0.0;
    
    // Benefit contributes positively
    score += benefit * 0.4;
    
    // Risk contributes negatively, adjusted by phase
    double risk_factor = phase.stability < 0.5 ? 0.3 : 0.2;  // Higher penalty in stable phases
    score -= risk * risk_factor;
    
    // Complexity and coupling penalties
    score -= complexity * 0.15;
    score -= coupling * 0.1;
    
    // Reward for staying within stability window
    if (risk >= window.lower_bound && risk <= window.upper_bound) {
        score += 0.1;  // Positive adjustment for stability alignment
    }
    
    // Adjust for trend direction
    if (trend.direction < 0) {
        // Negative trend - penalize high-risk approaches
        score -= risk * 0.05;
    } else {
        // Positive trend - slightly favor beneficial changes
        score += benefit * 0.05;
    }
    
    // Ensure score is within reasonable bounds
    score = min(1.0, max(-1.0, score));
    
    return score;
}

END_UPP_NAMESPACE