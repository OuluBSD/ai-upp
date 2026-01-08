#ifndef UPP_COREEVOLUTION_H
#define UPP_COREEVOLUTION_H

#include <Core/Core.h>

NAMESPACE_UPP

struct EvolutionEvent : Moveable<EvolutionEvent> {
    Time timestamp;
    String id;                 // unique id for the event
    String package;
    String agent_name;         // which agent / strategy triggered it (if any)
    String scenario_id;        // link to ScenarioPlan / Proposal id
    String lifecycle_phase;    // at time of change
    String strategy;           // active strategy profile name
    ValueMap context;          // e.g. drift, stability, seasonality snapshot

    // Change "genome" – categorical characterization
    Vector<String> change_kinds;   // e.g. ["include_cleanup", "rename", "graph_simplification"]
    ValueMap metrics_before;       // selected metrics snapshot
    ValueMap metrics_after;
    ValueMap deltas;               // normalized deltas (complexity, cycles, coupling, etc.)

    bool succeeded = true;         // true if applied & kept
    bool reverted = false;         // true if later undone
};

struct EvolutionSummary : Moveable<EvolutionSummary> {
    int total_events = 0;
    int successful = 0;
    int reverted_count = 0;

    ValueMap by_change_kind;     // kind -> aggregate stats
    ValueMap by_strategy;        // strategy -> success/failure, avg benefit/risk
    ValueMap by_phase;           // lifecycle phase -> preferred change kinds
};

class CoreEvolution {
public:
    CoreEvolution();

    void Load(const String& path);
    void Save(const String& path) const;

    void RecordEvent(const EvolutionEvent& ev);

    Vector<EvolutionEvent> GetTimeline() const;

    EvolutionSummary Summarize() const;

private:
    Vector<EvolutionEvent> events;
    String storage_path;   // e.g. <workspace-root>/.aiupp/evolution.json
};

END_UPP_NAMESPACE

#endif