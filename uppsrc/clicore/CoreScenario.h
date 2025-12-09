#ifndef _clicore_CoreScenario_h_
#define _clicore_CoreScenario_h_

#include "clicore.h"

class CoreSupervisor;  // Forward declaration to avoid circular dependency

class CoreScenario : Moveable<CoreScenario> {
public:
    struct ScenarioAction : Moveable<ScenarioAction> {
        String type;      // "playbook", "command", "refactor"
        String target;    // e.g. playbook name, command name, or refactor id
        ValueMap params;  // arguments
    };

    struct ScenarioPlan : Moveable<ScenarioPlan> {
        String name;
        Vector<ScenarioAction> actions;
        ValueMap metadata; // optional, e.g. originating strategy, tags
    };

    struct ScenarioSnapshot : Moveable<ScenarioSnapshot> {
        Value telemetry;
        Value semantic;
        Value architecture;
        Value behavior;
        double score;
    };

    struct ScenarioResult : Moveable<ScenarioResult> {
        ScenarioPlan plan;
        ScenarioSnapshot before;
        ScenarioSnapshot after;
        ValueMap deltas;   // metric deltas

        // New: Patch representation for diff and revert
        String unified_diff;      // textual patch representation
        ValueArray file_changes;  // structured summary per file

        bool applied = false;
    };

    CoreScenario();

    // Build a plan from Supervisor suggestions
    ScenarioPlan BuildPlanFromSupervisor(const Value& sup_plan_value,  // Use generic Value
                                         int max_actions);

    // Simulate applying a scenario (no actual edits)
    ScenarioResult Simulate(const ScenarioPlan& plan,
                            CoreIde& ide,
                            String& error);

    // Execute a scenario (real edits applied via CLI core)
    ScenarioResult Apply(const ScenarioPlan& plan,
                         CoreIde& ide,
                         String& error);

    // Revert changes based on a patch (v1)
    ScenarioResult Revert(const String& patch_text,
                          CoreIde& ide,
                          String& error);

private:
    ScenarioSnapshot Snapshot(CoreIde& ide) const;
    ValueMap ComputeDeltas(const ScenarioSnapshot& before,
                           const ScenarioSnapshot& after) const;
};

#endif