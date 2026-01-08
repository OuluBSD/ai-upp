#include "StrategicNavigator.h"

StrategicNavigator::StrategicNavigator() {
    // Constructor implementation
}

void StrategicNavigator::RegisterAgent(const AgentProfile& profile) {
    // Add the profile to the agents vector
    agents.Add(profile);
}

Vector<AgentProfile> StrategicNavigator::GetAgents() const {
    return agents;
}

AgentPlan StrategicNavigator::BuildAgentPlan(CoreIde& ide,
                                             const AgentProfile& profile,
                                             String& error) {
    AgentPlan plan;
    plan.agent_name = profile.name;

    // Initialize metadata
    plan.metadata = ValueMap();
    plan.metadata.Set("timestamp", Time::Now());
    plan.metadata.Set("strategy", profile.preferences.Get("strategy", "default"));
    plan.metadata.Set("risk_tolerance", profile.preferences.Get("risk_tolerance", 0.5));

    // Initialize goal IDs for metadata
    ValueArray goal_ids;
    for(const Goal& goal : profile.goals) {
        goal_ids.Add(goal.id);
    }
    plan.metadata.Set("goal_ids", goal_ids);

    // Get the main package for the proposal
    CoreWorkspace::Package main_package;
    if (!ide.GetMainPackage(main_package, error)) {
        return plan; // Return plan with error
    }

    // Build the proposal using CoreIde's BuildProposal method
    // For now, we'll use a fixed max_actions - in a real implementation,
    // this could be configured via agent preferences
    int max_actions = profile.preferences.Get("max_actions", 10);
    Value proposal = ide.BuildProposal(main_package.name, max_actions, error);

    if (!error.IsEmpty()) {
        return plan; // Return plan with error
    }

    plan.proposal = proposal;

    // Optionally filter or re-weight suggestions based on Goal.weights & priority
    // This would involve post-processing the proposal based on agent goals
    // For now, we store the weights snapshot in metadata
    ValueArray goals_info;
    for(const Goal& goal : profile.goals) {
        ValueMap goal_info;
        goal_info.Set("id", goal.id);
        goal_info.Set("weights", goal.weights);
        goal_info.Set("priority", goal.priority);
        goals_info.Add(goal_info);
    }
    plan.metadata.Set("goals_snapshot", goals_info);

    return plan;
}

GlobalPlan StrategicNavigator::BuildGlobalPlan(CoreIde& ide,
                                               const Vector<AgentProfile>& input_agents,
                                               String& error) {
    GlobalPlan global_plan;
    
    // Build individual agent plans
    for(int i = 0; i < input_agents.GetCount(); i++) {
        AgentPlan agent_plan = BuildAgentPlan(ide, input_agents[i], error);
        if (!error.IsEmpty()) {
            return GlobalPlan(); // Return empty plan on error
        }
        global_plan.agent_plans.Add(agent_plan);
    }
    
    // Detect conflicts between plans
    Vector<AgentPlan> agent_plans;
    for(int i = 0; i < global_plan.agent_plans.GetCount(); i++) {
        agent_plans.Add(global_plan.agent_plans[i]);
    }
    global_plan.conflicts = DetectConflicts(agent_plans);
    
    // Merge plans considering conflicts
    global_plan.merged = MergePlans(agent_plans, global_plan.conflicts);
    
    return global_plan;
}

ValueMap StrategicNavigator::DetectConflicts(const Vector<AgentPlan>& plans) const {
    ValueMap conflicts;

    // Heuristic conflict detection
    // For example: multiple agents touching same files with incompatible refactor types
    for(int i = 0; i < plans.GetCount(); i++) {
        for(int j = i + 1; j < plans.GetCount(); j++) {
            String conflict_key = Format("%s_vs_%s", plans[i].agent_name, plans[j].agent_name);
            ValueMap conflict_details;

            // Extract proposals for both agents
            Value proposal_i = plans[i].proposal;
            Value proposal_j = plans[j].proposal;

            // Check for conflicts in the proposals (simplified approach)
            // In a real implementation, this would deeply analyze the proposal structure
            if (IsValueMap(proposal_i) && IsValueMap(proposal_j)) {
                const ValueMap& map_i = ValueTo<ValueMap>(proposal_i);
                const ValueMap& map_j = ValueTo<ValueMap>(proposal_j);

                // Check if both proposals have 'actions' that might conflict
                if (map_i.Find("actions") >= 0 && map_j.Find("actions") >= 0) {
                    ValueArray actions_i = map_i.Get("actions");
                    ValueArray actions_j = map_j.Get("actions");

                    // Check for potential file conflicts
                    ValueArray conflicting_files;
                    for (int a = 0; a < actions_i.GetCount(); a++) {
                        if (IsValueMap(actions_i[a])) {
                            const ValueMap& action_i = ValueTo<ValueMap>(actions_i[a]);
                            String target_i = action_i.Get("target", String(""));

                            for (int b = 0; b < actions_j.GetCount(); b++) {
                                if (IsValueMap(actions_j[b])) {
                                    const ValueMap& action_j = ValueTo<ValueMap>(actions_j[b]);
                                    String target_j = action_j.Get("target", String(""));

                                    if (target_i == target_j && !target_i.IsEmpty()) {
                                        // Same target file - potential conflict
                                        conflicting_files.Add(target_i);
                                    }
                                }
                            }
                        }
                    }

                    if (conflicting_files.GetCount() > 0) {
                        conflict_details.Set("type", "file_conflict");
                        conflict_details.Set("conflicting_files", conflicting_files);
                        conflicts.Set(conflict_key, conflict_details);
                    }
                }
            }
        }
    }

    return conflicts;
}

ValueMap StrategicNavigator::MergePlans(const Vector<AgentPlan>& plans,
                                        const ValueMap& conflicts) const {
    ValueMap merged;

    // Create a merged sequence of actions that respects priorities and conflicts
    // This is a more sophisticated approach considering agent priorities and temporal windows

    // First, collect all agent plans with their priorities
    Vector<ValueMap> prioritized_plans;
    for(int i = 0; i < plans.GetCount(); i++) {
        ValueMap plan_data;
        plan_data.Set("agent_name", plans[i].agent_name);
        plan_data.Set("proposal", plans[i].proposal);
        plan_data.Set("metadata", plans[i].metadata);

        // Calculate overall priority based on goals
        double total_priority = 0.0;
        int goal_count = 0;
        if (IsValueMap(plans[i].metadata)) {
            const ValueMap& metadata = ValueTo<ValueMap>(plans[i].metadata);
            if (metadata.Find("goals_snapshot") >= 0) {
                ValueArray goals = metadata.Get("goals_snapshot");
                for (int g = 0; g < goals.GetCount(); g++) {
                    if (IsValueMap(goals[g])) {
                        const ValueMap& goal = ValueTo<ValueMap>(goals[g]);
                        total_priority += goal.Get("priority", 0.0);
                        goal_count++;
                    }
                }
            }
        }
        double priority = (goal_count > 0) ? total_priority / goal_count : 0.0;
        plan_data.Set("priority", priority);

        prioritized_plans.Add(plan_data);
    }

    // Sort plans by priority (descending)
    Sort(prioritized_plans, [](const ValueMap& a, const ValueMap& b) {
        return a.Get("priority", 0.0) > b.Get("priority", 0.0);
    });

    // Store the ordered plans in the merged result
    merged.Set("ordered_plans", prioritized_plans);
    merged.Set("conflicts_resolved", conflicts.GetCount() == 0);  // True if no conflicts detected
    merged.Set("total_agents", plans.GetCount());
    merged.Set("conflict_count", conflicts.GetCount());

    // Add conflict information
    merged.Set("conflicts", conflicts);

    return merged;
}