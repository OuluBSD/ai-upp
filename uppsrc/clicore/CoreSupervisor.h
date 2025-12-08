#ifndef _clicore_CoreSupervisor_h_
#define _clicore_CoreSupervisor_h_

#include <Core/Core.h>
#include <CtrlCore/CtrlCore.h>

class CoreIde;

class CoreSupervisor : Moveable<CoreSupervisor> {
public:
    struct Suggestion : Moveable<Suggestion> {
        String action;   // e.g., "run_playbook"
        String target;   // e.g., "cleanup_includes_and_rebuild"
        Value params;    // param map
        String reason;   // explanation (human-readable)
    };

    struct Plan : Moveable<Plan> {
        Vector<Suggestion> steps;
        String summary;
    };

    CoreSupervisor();

    // Top-level entry: generate improvement plan for a package
    Plan GenerateOptimizationPlan(const String& package,
                                  CoreIde& ide,
                                  String& error);

private:
    Suggestion SuggestIncludeCleanup(const String& package,
                                     const Value& pkg_stats);

    Suggestion SuggestRenameHotspot(const String& package,
                                    const Value& telemetry,
                                    CoreIde& ide);

    Suggestion SuggestGraphSimplification(const String& package,
                                          const Value& graph_stats);

    Suggestion SuggestRunOptimizationLoop(const String& package,
                                          const Value& pkg_stats);

    double ComputeRiskScore(const Value& pkg_stats,
                            const Value& graph_stats,
                            const Value& file_complexity);
};

#endif