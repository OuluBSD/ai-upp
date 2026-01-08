#ifndef _clicore_StrategicNavigator_StrategicNavigator_h_
#define _clicore_StrategicNavigator_StrategicNavigator_h_

#include <Core/Core.h>
#include <Core/ValueMap.h>
#include <Core/ValueArray.h>

#include "CoreIde.h"  // Needed for CoreIde reference

struct Goal : Moveable<Goal> {
    String id;                 // e.g. "reduce_complexity", "flatten_dependencies"
    String description;
    ValueMap weights;          // weight per metric: complexity, coupling, cycles, telemetry, etc.
    double priority;           // 0–1
};

struct AgentProfile : Moveable<AgentProfile> {
    String name;               // e.g. "complexity_agent", "graph_agent"
    ValueMap preferences;      // agent-specific settings (strategy name, risk tolerance, etc.)
    Vector<Goal> goals;
};

struct AgentPlan : Moveable<AgentPlan> {
    String agent_name;
    ValueMap metadata;         // e.g. strategy used, timestamps
    Value   proposal;          // full proposal (reusing CoreProposal format)
};

struct GlobalPlan : Moveable<GlobalPlan> {
    ValueArray agent_plans;    // list of AgentPlan
    ValueMap   conflicts;      // conflicts between agent plans
    ValueMap   merged;         // merged high-level sequence of actions
};

class StrategicNavigator {
public:
    StrategicNavigator();

    void RegisterAgent(const AgentProfile& profile);
    Vector<AgentProfile> GetAgents() const;

    AgentPlan BuildAgentPlan(CoreIde& ide,
                             const AgentProfile& profile,
                             String& error);

    GlobalPlan BuildGlobalPlan(CoreIde& ide,
                               const Vector<AgentProfile>& agents,
                               String& error);

private:
    Vector<AgentProfile> agents;

    ValueMap DetectConflicts(const Vector<AgentPlan>& plans) const;
    ValueMap MergePlans(const Vector<AgentPlan>& plans,
                        const ValueMap& conflicts) const;
};

#endif