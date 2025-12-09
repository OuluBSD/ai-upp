#ifndef _clicore_CoreProposal_h_
#define _clicore_CoreProposal_h_

#include <Core/Core.h>
#include "CoreScenario.h"
#include "CoreSupervisor.h"
#include "CoreSemantic.h"

using namespace Upp;

class CoreIde;

class CoreProposal : Moveable<CoreProposal> {
public:
    struct Proposal : Moveable<Proposal> {
        String id;                 // unique, deterministic string (e.g. hash of plan + timestamp)
        String package;            // package the proposal concerns
        ValueMap supervisor_plan;  // raw supervisor plan
        ValueMap scenario_plan;    // raw scenario plan
        Value   simulation_before;
        Value   simulation_after;
        Value   deltas;
        String  patch;             // unified diff from scenario apply
        ValueMap file_changes;
        ValueMap semantic_snapshot;
        ValueMap architecture_snapshot;
        double  risk_score = 0;
        double  confidence_score = 0;
        double  benefit_score = 0;
        ValueMap metadata;         // free-form tags: strategy, playbook, timestamp, etc.
    };

    CoreProposal();

    Proposal BuildProposal(CoreIde& ide,
                           const String& package,
                           int max_actions,
                           String& error);

    Value ToValue(const Proposal& p) const;
};

#endif