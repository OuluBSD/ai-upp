#ifndef _clicore_LifecycleModel_h_
#define _clicore_LifecycleModel_h_

#include <Core/Core.h>

NAMESPACE_UPP

// Forward declaration to avoid circular dependency
class CoreTelemetry;

// Structures for lifecycle phase detection
struct LifecyclePhase : Moveable<LifecyclePhase> {
    String name;
    String description;
    double stability;      // 0-1, how stable this phase is
    double volatility;     // 0-1, how volatile/chaotic
    double refactor_bias;  // -1 to 1, -1 = avoid refactoring, 1 = encourage refactoring
    double entropy;        // 0-1, measure of disorder in the codebase
};

struct PhaseSample : Moveable<PhaseSample> {
    String phase_name;
    double entropy;
    Time timestamp;
    ValueMap metrics;      // Additional metrics recorded at this time
};

struct DriftMetrics : Moveable<DriftMetrics> {
    int transitions;           // Number of phase transitions in history
    int back_and_forth;        // Number of transitions between same phases
    double avg_phase_duration; // Average duration of phases
};

struct ArchitectureDiagnostic : Moveable<ArchitectureDiagnostic> {
    double complexity_index;     // 0-1, normalized complexity score
    double coupling_score;       // 0-1, normalized coupling score
    double structural_entropy;   // 0-1, entropy of package structure
};

class LifecycleModel {
public:
    LifecycleModel();

    // Detect the current lifecycle phase based on various metrics
    LifecyclePhase DetectPhase(const struct TemporalDynamics::Trend& trend,
                               const ArchitectureDiagnostic& diag,
                               double semantic_entropy) const;

    // Record a phase sample in the history
    void RecordPhase(const LifecyclePhase& phase);

    // Get the history of recorded phases
    Vector<PhaseSample> GetHistory() const;

    // Get all known phases (for introspection)
    Vector<LifecyclePhase> GetKnownPhases() const;

    // Compute drift metrics from history
    DriftMetrics ComputeDrift() const;

    // Compute stability index based on drift and trend
    double ComputeStabilityIndex(const DriftMetrics& drift,
                                 const struct TemporalDynamics::Trend& trend) const;

private:
    Vector<PhaseSample> history;
    String storage_path;  // Path to save/load history

public:
    void SetStoragePath(const String& path) { storage_path = path; }
    void Save(const String& path) const;
    void Load(const String& path);
};

END_UPP_NAMESPACE

#endif