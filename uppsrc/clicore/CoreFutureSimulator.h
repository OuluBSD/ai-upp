#ifndef _clicore_CoreFutureSimulator_h_
#define _clicore_CoreFutureSimulator_h_

#include "clicore.h"

NAMESPACE_UPP

class CoreFutureSimulator {
public:
    struct FutureBranch : Moveable<FutureBranch> {
        String id;                     // e.g. "branch_1", "branch_low_risk", ...
        ValueMap starting_point;       // snapshot: telemetry + architecture + semantic summary
        ValueArray actions;            // scenario actions included in this branch
        ValueMap projected_metrics;    // predicted complexity, coupling, cycles, risk, benefit, etc.
        ValueMap terminal_state;       // predicted "end" state snapshot
        double score = 0;              // overall fitness score for this branch
    };

    struct OutcomeHorizon : Moveable<OutcomeHorizon> {
        ValueArray branches;           // list of FutureBranch (full details)
        ValueMap best_branch;          // minimal summary of top-scoring branch
        ValueMap stats;                // horizon-level stats (min/max/avg risk, benefit, etc.)
    };

public:
    CoreFutureSimulator();

    OutcomeHorizon Explore(const CoreScenario::ScenarioPlan& base_plan,
                           const Vector<CoreScenario::ScenarioAction>& negotiated_actions,
                           const LifecyclePhase& phase,
                           const TemporalDynamics::Trend& trend,
                           const TemporalSeasonality::StabilityWindow& window);

private:
    FutureBranch BuildBranch(const String& id,
                             const CoreScenario::ScenarioPlan& base_plan,
                             const Vector<CoreScenario::ScenarioAction>& actions,
                             const LifecyclePhase& phase,
                             const TemporalDynamics::Trend& trend,
                             const TemporalSeasonality::StabilityWindow& window);

    ValueMap PredictMetrics(const CoreScenario::ScenarioPlan& base_plan,
                            const Vector<CoreScenario::ScenarioAction>& actions,
                            const LifecyclePhase& phase,
                            const TemporalDynamics::Trend& trend);

    double ScoreBranch(const ValueMap& metrics,
                       const LifecyclePhase& phase,
                       const TemporalDynamics::Trend& trend,
                       const TemporalSeasonality::StabilityWindow& window);
};

END_UPP_NAMESPACE

#endif