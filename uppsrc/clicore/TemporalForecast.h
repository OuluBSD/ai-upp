#ifndef _clicore_TemporalForecast_h_
#define _clicore_TemporalForecast_h_

#include "Core.h"
#include "LifecycleModel.h"
#include "TemporalSeasonality.h"

struct ForecastPoint : Moveable<ForecastPoint> {
    int t;                     // future timestep
    String predicted_phase;
    double predicted_entropy;  // heuristic extrapolation
    double confidence;
};

struct ShockScenario : Moveable<ShockScenario> {
    String type;               // "developer_churn", "api_break", "mass_refactor", etc.
    double severity;           // 0–1
    double probability;        // 0–1
};

struct RiskProfile : Moveable<RiskProfile> {
    double long_term_risk;     // 0–1
    double volatility_risk;
    double schedule_risk;
    double architectural_risk;
    Vector<ShockScenario> possible_shocks;
};

class TemporalForecast {
public:
    Vector<ForecastPoint> ForecastLifecycle(
        const Vector<PhaseSample>& history,
        int horizon
    ) const;

    RiskProfile ComputeRiskProfile(
        const Vector<PhaseSample>& history,
        const ReleaseCadence& cadence
    ) const;

    ShockScenario SimulateShock(
        const Vector<PhaseSample>& history,
        const String& type
    ) const;
};

#endif