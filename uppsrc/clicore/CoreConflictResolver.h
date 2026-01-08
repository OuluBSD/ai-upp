#ifndef _clicore_CoreConflictResolver_h_
#define _clicore_CoreConflictResolver_h_

#include <Core/Core.h>
#include <Core/ValueMap.h>
#include <Core/ValueArray.h>

#include "StrategicNavigator.h"  // Needed for AgentPlan and AgentProfile

struct ConflictDetail : Moveable<ConflictDetail> {
    String file;
    int line;
    String type;          // "edit_overlap", "semantic_disagreement", "refactor_collision"
    ValueMap agents;      // agent_name -> details
    ValueMap metadata;    // extra fields: severity, rationale
};

struct TradeOff : Moveable<TradeOff> {
    String id;         // "choose_agent_A", "prefer_lower_risk", etc.
    String description;
    double score;      // higher = better
    ValueMap rationale;
};

struct NegotiatedResult : Moveable<NegotiatedResult> {
    ValueArray final_actions;     // merged scenario actions
    ValueArray discarded_actions; // actions removed due to conflict
    ValueArray tradeoffs;         // computed TradeOff decisions
};

class CoreConflictResolver {
public:
    CoreConflictResolver();

    Vector<ConflictDetail> DetectConflicts(const Vector<AgentPlan>& plans);
    Vector<TradeOff> EvaluateTradeOffs(const Vector<ConflictDetail>& conflicts,
                                       const Vector<AgentPlan>& plans,
                                       const Vector<AgentProfile>& profiles);

    NegotiatedResult Negotiate(const Vector<AgentPlan>& plans,
                               const Vector<AgentProfile>& profiles);

private:
    ConflictDetail DetectEditOverlap(const AgentPlan& a, const AgentPlan& b);
    ConflictDetail DetectSemanticDisagreement(const AgentPlan& a, const AgentPlan& b);
    ConflictDetail DetectRefactorCollisions(const AgentPlan& a, const AgentPlan& b);

    TradeOff ChooseLowerRisk(const ConflictDetail&, const AgentPlan&, const AgentPlan&);
    TradeOff PreferHigherBenefit(const ConflictDetail&, const AgentPlan&, const AgentPlan&);
    TradeOff HonorPriority(const ConflictDetail&, const AgentProfile&, const AgentProfile&);
};

#endif