#ifndef _clicore_GlobalKnowledge_GlobalKnowledge_h_
#define _clicore_GlobalKnowledge_GlobalKnowledge_h_

#include "clicore.h"

class GlobalKnowledge : public Moveable<GlobalKnowledge> {
public:
    struct PatternStats : public Moveable<PatternStats> {
        int occurrences = 0;
        int successes = 0;
        int failures = 0;
        double avg_benefit = 0;
        double avg_risk = 0;
        double avg_confidence = 0;
    };

    struct RefactorStats : public Moveable<RefactorStats> {
        int uses = 0;
        int successful = 0;
        int reverted = 0;
        double avg_delta_complexity = 0;
    };

    struct TopologyStats : public Moveable<TopologyStats> {
        double avg_cycles = 0;
        double avg_depth = 0;
        double avg_coupling = 0;
        int count = 0;
    };

    GlobalKnowledge();
    
    void Load(const String& path);
    void Save(const String& path) const;

    void RecordWorkspaceSnapshot(const ValueMap& snapshot);
    void RecordPatternOutcome(const String& pattern, bool success, const ValueMap& deltas);
    void RecordRefactorOutcome(const String& type, bool success, const ValueMap& deltas);

    // Analytics
    PatternStats GetPatternStats(const String& pattern) const;
    RefactorStats GetRefactorStats(const String& refactor) const;
    TopologyStats GetTopologyStats() const;

private:
    ValueMap data;     // root JSON structure
    String  storage_path;
};

#endif