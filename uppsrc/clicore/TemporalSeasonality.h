#ifndef _clicore_TemporalSeasonality_h_
#define _clicore_TemporalSeasonality_h_

#include <Core/Core.h>
#include "LifecycleModel.h"

NAMESPACE_UPP

struct SeasonalityPattern : Moveable<SeasonalityPattern> {
    String name;
    double intensity;      // 0-1, how strong this pattern is
    double confidence;     // 0-1, confidence in this pattern
    Vector<int> peaks;     // Time units where this pattern peaks
};

struct ReleaseCadence : Moveable<ReleaseCadence> {
    int average_interval;  // Average interval between releases (in days)
    double confidence;     // 0-1, confidence in this cadence
};

struct StabilityWindow : Moveable<StabilityWindow> {
    int start;            // Start time unit for stability window
    int end;              // End time unit for stability window
    double predicted_safety; // 0-1, predicted safety of changes during this window
};

class TemporalSeasonality {
public:
    TemporalSeasonality();

    // Detect seasonality patterns in lifecycle history
    Vector<SeasonalityPattern> DetectSeasonality(const Vector<PhaseSample>& history) const;

    // Infer release cadence from history
    ReleaseCadence InferReleaseCadence(const Vector<PhaseSample>& history) const;

    // Predict stability windows based on history and release cadence
    Vector<StabilityWindow> PredictStabilityWindows(const Vector<PhaseSample>& history,
                                                    const ReleaseCadence& cadence) const;

private:
    // Internal data for seasonality analysis
};

END_UPP_NAMESPACE

#endif