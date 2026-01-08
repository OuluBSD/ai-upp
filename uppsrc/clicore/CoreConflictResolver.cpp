#include "CoreConflictResolver.h"

CoreConflictResolver::CoreConflictResolver() {
    // Constructor implementation
}

Vector<ConflictDetail> CoreConflictResolver::DetectConflicts(const Vector<AgentPlan>& plans) {
    Vector<ConflictDetail> conflicts;
    
    // Check for conflicts between all pairs of plans
    for(int i = 0; i < plans.GetCount(); i++) {
        for(int j = i + 1; j < plans.GetCount(); j++) {
            // Check for edit overlaps
            ConflictDetail edit_conflict = DetectEditOverlap(plans[i], plans[j]);
            if (!edit_conflict.type.IsEmpty()) {
                conflicts.Add(edit_conflict);
            }
            
            // Check for semantic disagreements
            ConflictDetail semantic_conflict = DetectSemanticDisagreement(plans[i], plans[j]);
            if (!semantic_conflict.type.IsEmpty()) {
                conflicts.Add(semantic_conflict);
            }
            
            // Check for refactor collisions
            ConflictDetail refactor_conflict = DetectRefactorCollisions(plans[i], plans[j]);
            if (!refactor_conflict.type.IsEmpty()) {
                conflicts.Add(refactor_conflict);
            }
        }
    }
    
    return conflicts;
}

Vector<TradeOff> CoreConflictResolver::EvaluateTradeOffs(const Vector<ConflictDetail>& conflicts,
                                                         const Vector<AgentPlan>& plans,
                                                         const Vector<AgentProfile>& profiles) {
    Vector<TradeOff> tradeoffs;
    
    for (const ConflictDetail& conflict : conflicts) {
        // Determine which agents are involved in the conflict
        Vector<String> agent_names;
        for (const auto& agent : conflict.agents) {
            agent_names.Add(agent.key);
        }
        
        // Find the agent plans involved
        AgentPlan plan_a, plan_b;
        AgentProfile profile_a, profile_b;
        bool found_a = false, found_b = false;
        
        for (const AgentPlan& plan : plans) {
            if (plan.agent_name == agent_names[0]) {
                plan_a = plan;
                found_a = true;
            }
            if (agent_names.GetCount() > 1 && plan.agent_name == agent_names[1]) {
                plan_b = plan;
                found_b = true;
            }
        }
        
        for (const AgentProfile& profile : profiles) {
            if (profile.name == agent_names[0]) {
                profile_a = profile;
                found_a = true;
            }
            if (agent_names.GetCount() > 1 && profile.name == agent_names[1]) {
                profile_b = profile;
                found_b = true;
            }
        }
        
        // Evaluate trade-offs based on conflict type
        if (conflict.type == "edit_overlap") {
            TradeOff risk_tradeoff = ChooseLowerRisk(conflict, plan_a, plan_b);
            if (!risk_tradeoff.id.IsEmpty()) {
                tradeoffs.Add(risk_tradeoff);
            }
            
            TradeOff benefit_tradeoff = PreferHigherBenefit(conflict, plan_a, plan_b);
            if (!benefit_tradeoff.id.IsEmpty()) {
                tradeoffs.Add(benefit_tradeoff);
            }
        }
        
        // If there are agent profiles, evaluate priority-based trade-offs
        if (found_a && found_b) {
            TradeOff priority_tradeoff = HonorPriority(conflict, profile_a, profile_b);
            if (!priority_tradeoff.id.IsEmpty()) {
                tradeoffs.Add(priority_tradeoff);
            }
        }
    }
    
    return tradeoffs;
}

NegotiatedResult CoreConflictResolver::Negotiate(const Vector<AgentPlan>& plans,
                                                 const Vector<AgentProfile>& profiles) {
    NegotiatedResult result;

    // Detect conflicts
    Vector<ConflictDetail> conflicts = DetectConflicts(plans);

    // Evaluate trade-offs
    Vector<TradeOff> tradeoffs = EvaluateTradeOffs(conflicts, plans, profiles);

    // Create a map of agent names to their profiles for quick lookup
    ValueMap profile_map;
    for (const AgentProfile& profile : profiles) {
        ValueMap profile_data;
        profile_data.Set("name", profile.name);
        profile_data.Set("preferences", profile.preferences);

        ValueArray goals_array;
        for(const Goal& goal : profile.goals) {
            ValueMap goal_map;
            goal_map.Set("id", goal.id);
            goal_map.Set("description", goal.description);
            goal_map.Set("weights", goal.weights);
            goal_map.Set("priority", goal.priority);
            goals_array.Add(goal_map);
        }
        profile_data.Set("goals", goals_array);
        profile_map.Set(profile.name, profile_data);
    }

    // Create a list of all actions with their conflict status
    Vector<ValueMap> all_actions;
    for (const AgentPlan& plan : plans) {
        if (IsValueMap(plan.proposal)) {
            const ValueMap& proposal_map = ValueTo<ValueMap>(plan.proposal);
            if (proposal_map.Find("actions") >= 0) {
                ValueArray actions = proposal_map.Get("actions");
                for (const Value& action : actions) {
                    if (IsValueMap(action)) {
                        ValueMap action_with_agent = ValueTo<ValueMap>(action);
                        action_with_agent.Set("agent_name", plan.agent_name);
                        action_with_agent.Set("original_action", action);
                        all_actions.Add(action_with_agent);
                    }
                }
            }
        }
    }

    // Process conflicts and make decisions about which actions to keep
    Vector<int> actions_to_discard;  // indices of actions to discard

    for (const ConflictDetail& conflict : conflicts) {
        // Find actions involved in this conflict
        Vector<int> conflicting_action_indices;
        for (int i = 0; i < all_actions.GetCount(); i++) {
            const ValueMap& action = all_actions[i];
            String action_agent_name = action.Get("agent_name", String(""));

            // Check if this action belongs to an agent involved in the conflict
            bool is_conflicting_agent = false;
            for (const auto& agent : conflict.agents) {
                if (agent.key == action_agent_name) {
                    is_conflicting_agent = true;
                    break;
                }
            }

            if (is_conflicting_agent) {
                // For edit overlaps, check if the file matches
                if (conflict.type == "edit_overlap" && action.Get("target", String("")) == conflict.file) {
                    conflicting_action_indices.Add(i);
                }
                // For other conflict types, mark all actions from conflicting agents
                else if (conflict.type != "edit_overlap") {
                    conflicting_action_indices.Add(i);
                }
            }
        }

        // Now decide which actions to keep and which to discard
        if (conflicting_action_indices.GetCount() > 1) {
            // Select the "best" action based on trade-offs
            int best_action_index = conflicting_action_indices[0];  // Default to first

            // Find the action from the agent with the highest priority or lowest risk
            for (int idx : conflicting_action_indices) {
                String current_agent = all_actions[idx].Get("agent_name", String(""));

                // Check if this agent has a higher priority than the current best
                bool is_better_choice = false;

                // Get the current best action's agent for comparison
                String best_agent = all_actions[best_action_index].Get("agent_name", String(""));

                // Check agent priorities
                if (profile_map.Find(best_agent) >= 0 && profile_map.Find(current_agent) >= 0) {
                    ValueMap best_profile = profile_map.Get(best_agent);
                    ValueMap current_profile = profile_map.Get(current_agent);

                    ValueArray best_goals = best_profile.Get("goals");
                    ValueArray current_goals = current_profile.Get("goals");

                    // Calculate average priorities
                    double best_priority_sum = 0.0, current_priority_sum = 0.0;
                    for (const Value& goal : best_goals) {
                        if (IsValueMap(goal)) {
                            best_priority_sum += ValueTo<ValueMap>(goal).Get("priority", 0.0);
                        }
                    }
                    for (const Value& goal : current_goals) {
                        if (IsValueMap(goal)) {
                            current_priority_sum += ValueTo<ValueMap>(goal).Get("priority", 0.0);
                        }
                    }

                    double avg_best_priority = best_goals.GetCount() > 0 ? best_priority_sum / best_goals.GetCount() : 0.0;
                    double avg_current_priority = current_goals.GetCount() > 0 ? current_priority_sum / current_goals.GetCount() : 0.0;

                    // Prefer the agent with higher priority
                    if (avg_current_priority > avg_best_priority) {
                        is_better_choice = true;
                    }
                    // If priorities are equal, prefer based on other factors like risk
                    else if (Abs(avg_current_priority - avg_best_priority) < 0.001) {
                        // Check for risk-related metadata in the action or agent preferences
                        ValueMap agent_prefs = current_profile.Get("preferences");
                        double current_risk_tolerance = agent_prefs.Get("risk_tolerance", 0.5);

                        agent_prefs = best_profile.Get("preferences");
                        double best_risk_tolerance = agent_prefs.Get("risk_tolerance", 0.5);

                        // If current agent has lower risk tolerance, it might be more conservative and better
                        if (current_risk_tolerance < best_risk_tolerance) {
                            is_better_choice = true;
                        }
                    }
                }

                if (is_better_choice) {
                    best_action_index = idx;
                }
            }

            // Mark all other conflicting actions for discard except the best one
            for (int idx : conflicting_action_indices) {
                if (idx != best_action_index) {
                    // Check if this index is already in the discard list to avoid duplicates
                    bool already_discarded = false;
                    for (int discarded_idx : actions_to_discard) {
                        if (discarded_idx == idx) {
                            already_discarded = true;
                            break;
                        }
                    }
                    if (!already_discarded) {
                        actions_to_discard.Add(idx);

                        // Add a tradeoff record for the discarded action
                        TradeOff discarded_tradeoff;
                        discarded_tradeoff.id = "discard_action_" + all_actions[idx].Get("agent_name", String(""));
                        discarded_tradeoff.description = "Action discarded due to conflict resolution";
                        discarded_tradeoff.score = 0.0;

                        ValueMap rationale;
                        rationale.Set("reason", "Action discarded due to conflict with higher priority action");
                        rationale.Set("discarded_agent", all_actions[idx].Get("agent_name", String("")));
                        rationale.Set("kept_agent", all_actions[best_action_index].Get("agent_name", String("")));
                        discarded_tradeoff.rationale = rationale;

                        result.tradeoffs.Add(discarded_tradeoff);
                    }
                }
            }
        }
    }

    // Add actions to final or discarded based on conflict resolution
    for (int i = 0; i < all_actions.GetCount(); i++) {
        bool is_discarded = false;
        for (int discarded_idx : actions_to_discard) {
            if (i == discarded_idx) {
                is_discarded = true;
                break;
            }
        }

        if (is_discarded) {
            result.discarded_actions.Add(all_actions[i].Get("original_action"));
        } else {
            result.final_actions.Add(all_actions[i].Get("original_action"));
        }
    }

    // Add the evaluated tradeoffs to the result
    for (const TradeOff& tradeoff : tradeoffs) {
        result.tradeoffs.Add(tradeoff);
    }

    return result;
}

ConflictDetail CoreConflictResolver::DetectEditOverlap(const AgentPlan& a, const AgentPlan& b) {
    ConflictDetail conflict;

    // Extract actions from the proposals
    if (IsValueMap(a.proposal) && IsValueMap(b.proposal)) {
        const ValueMap& proposal_a = ValueTo<ValueMap>(a.proposal);
        const ValueMap& proposal_b = ValueTo<ValueMap>(b.proposal);

        if (proposal_a.Find("actions") >= 0 && proposal_b.Find("actions") >= 0) {
            ValueArray actions_a = proposal_a.Get("actions");
            ValueArray actions_b = proposal_b.Get("actions");

            for (const Value& action_a : actions_a) {
                if (IsValueMap(action_a)) {
                    const ValueMap& action_a_map = ValueTo<ValueMap>(action_a);
                    String file_a = action_a_map.Get("target", String(""));
                    Value params_a = action_a_map.Get("params", ValueMap());

                    for (const Value& action_b : actions_b) {
                        if (IsValueMap(action_b)) {
                            const ValueMap& action_b_map = ValueTo<ValueMap>(action_b);
                            String file_b = action_b_map.Get("target", String(""));
                            Value params_b = action_b_map.Get("params", ValueMap());

                            // If same file, check for overlapping edits
                            if (file_a == file_b && !file_a.IsEmpty()) {
                                // Extract position and count information if available
                                bool has_position_overlap = false;

                                if (IsValueMap(params_a) && IsValueMap(params_b)) {
                                    const ValueMap& params_a_map = ValueTo<ValueMap>(params_a);
                                    const ValueMap& params_b_map = ValueTo<ValueMap>(params_b);

                                    // Check for position-based overlaps (for insert/replace operations)
                                    int pos_a = params_a_map.Get("pos", -1);
                                    int count_a = params_a_map.Get("count", -1);  // length of text being replaced/removed
                                    int pos_b = params_b_map.Get("pos", -1);
                                    int count_b = params_b_map.Get("count", -1);

                                    // For insert operations, we might have length of inserted text
                                    int insert_len_a = params_a_map.Get("insert_len", -1);
                                    int insert_len_b = params_b_map.Get("insert_len", -1);

                                    if (pos_a >= 0 && pos_b >= 0) {
                                        // Calculate ranges [pos, pos + count] for overlap detection
                                        int end_a = pos_a + (count_a > 0 ? count_a : 0);
                                        int end_b = pos_b + (count_b > 0 ? count_b : 0);

                                        // Check for range overlap
                                        if ((pos_a <= end_b && end_a >= pos_b) ||
                                            (pos_b <= end_a && end_b >= pos_a)) {
                                            has_position_overlap = true;
                                        } else if (insert_len_a > 0 && insert_len_b > 0) {
                                            // If both are insert operations, check if they're inserting at the same position
                                            if (pos_a == pos_b) {
                                                has_position_overlap = true;
                                            }
                                        }
                                    }
                                }

                                if (has_position_overlap) {
                                    conflict.file = file_a;
                                    conflict.type = "edit_overlap";

                                    ValueMap agent_a_details, agent_b_details;
                                    agent_a_details.Set("action", action_a);
                                    agent_b_details.Set("action", action_b);

                                    conflict.agents.Set(a.agent_name, agent_a_details);
                                    conflict.agents.Set(b.agent_name, agent_b_details);

                                    // Add metadata about the conflict
                                    conflict.metadata.Set("severity", "high");
                                    conflict.metadata.Set("rationale",
                                        "Agents attempting to modify overlapping ranges in the same file");

                                    return conflict;  // Return the first conflict found
                                } else {
                                    // Even if positions don't overlap, same file modifications can still be conflicting
                                    conflict.file = file_a;
                                    conflict.type = "edit_overlap";

                                    ValueMap agent_a_details, agent_b_details;
                                    agent_a_details.Set("action", action_a);
                                    agent_b_details.Set("action", action_b);

                                    conflict.agents.Set(a.agent_name, agent_a_details);
                                    conflict.agents.Set(b.agent_name, agent_b_details);

                                    // Add metadata about the conflict
                                    conflict.metadata.Set("severity", "medium");
                                    conflict.metadata.Set("rationale", "Both agents attempting to modify the same file");

                                    return conflict;  // Return the first conflict found
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return conflict;  // Empty conflict if none found
}

ConflictDetail CoreConflictResolver::DetectSemanticDisagreement(const AgentPlan& a, const AgentPlan& b) {
    ConflictDetail conflict;
    
    // This is a simplified implementation - in reality, this would analyze
    // semantic snapshots in the proposals to detect incompatible changes
    if (IsValueMap(a.proposal) && IsValueMap(b.proposal)) {
        const ValueMap& proposal_a = ValueTo<ValueMap>(a.proposal);
        const ValueMap& proposal_b = ValueTo<ValueMap>(b.proposal);
        
        // Check if both proposals have semantic information and if they contain
        // conflicting semantic changes
        if (proposal_a.Find("semantic_snapshot") >= 0 && proposal_b.Find("semantic_snapshot") >= 0) {
            // For now, we'll just mark any scenario with semantic snapshots as potentially
            // having a disagreement - a real implementation would analyze for actual conflicts
            conflict.type = "semantic_disagreement";
            conflict.metadata.Set("rationale", "Both agents propose semantic changes that need review");
            
            ValueMap agent_a_details, agent_b_details;
            agent_a_details.Set("proposal", a.proposal);
            agent_b_details.Set("proposal", b.proposal);
            
            conflict.agents.Set(a.agent_name, agent_a_details);
            conflict.agents.Set(b.agent_name, agent_b_details);
        }
    }
    
    return conflict;
}

ConflictDetail CoreConflictResolver::DetectRefactorCollisions(const AgentPlan& a, const AgentPlan& b) {
    ConflictDetail conflict;
    
    // Check for refactor collisions like rename vs delete, pipeline reorder vs flatten, etc.
    if (IsValueMap(a.proposal) && IsValueMap(b.proposal)) {
        const ValueMap& proposal_a = ValueTo<ValueMap>(a.proposal);
        const ValueMap& proposal_b = ValueTo<ValueMap>(b.proposal);
        
        if (proposal_a.Find("actions") >= 0 && proposal_b.Find("actions") >= 0) {
            ValueArray actions_a = proposal_a.Get("actions");
            ValueArray actions_b = proposal_b.Get("actions");
            
            for (const Value& action_a : actions_a) {
                if (IsValueMap(action_a)) {
                    const ValueMap& action_a_map = ValueTo<ValueMap>(action_a);
                    String type_a = action_a_map.Get("type", String(""));
                    
                    for (const Value& action_b : actions_b) {
                        if (IsValueMap(action_b)) {
                            const ValueMap& action_b_map = ValueTo<ValueMap>(action_b);
                            String type_b = action_b_map.Get("type", String(""));
                            
                            // Check for incompatible refactor types
                            if ((type_a == "rename" && type_b == "delete") ||
                                (type_a == "delete" && type_b == "rename") ||
                                (type_a == "reorder_pipeline" && type_b == "flatten_pipeline") ||
                                (type_a == "flatten_pipeline" && type_b == "reorder_pipeline")) {
                                
                                conflict.type = "refactor_collision";
                                conflict.metadata.Set("rationale", 
                                    "Incompatible refactor operations: " + type_a + " vs " + type_b);
                                
                                ValueMap agent_a_details, agent_b_details;
                                agent_a_details.Set("action", action_a);
                                agent_b_details.Set("action", action_b);
                                
                                conflict.agents.Set(a.agent_name, agent_a_details);
                                conflict.agents.Set(b.agent_name, agent_b_details);
                                
                                return conflict;  // Return the first conflict found
                            }
                        }
                    }
                }
            }
        }
    }
    
    return conflict;
}

TradeOff CoreConflictResolver::ChooseLowerRisk(const ConflictDetail& conflict,
                                               const AgentPlan& plan_a,
                                               const AgentPlan& plan_b) {
    TradeOff tradeoff;
    
    // Determine risk levels of each plan (simplified approach)
    // In reality, this would extract risk metrics from the proposals
    double risk_a = 0.0, risk_b = 0.0;
    
    if (IsValueMap(plan_a.proposal)) {
        const ValueMap& proposal_a = ValueTo<ValueMap>(plan_a.proposal);
        if (proposal_a.Find("risk_score") >= 0) {
            risk_a = proposal_a.Get("risk_score", 0.0);
        } else if (proposal_a.Find("strategy") >= 0) {
            // Infer risk from strategy name
            String strategy = proposal_a.Get("strategy", String(""));
            if (strategy.Find("aggressive") >= 0) risk_a = 0.8;
            else if (strategy.Find("conservative") >= 0) risk_a = 0.2;
            else risk_a = 0.5;  // Default
        }
    }
    
    if (IsValueMap(plan_b.proposal)) {
        const ValueMap& proposal_b = ValueTo<ValueMap>(plan_b.proposal);
        if (proposal_b.Find("risk_score") >= 0) {
            risk_b = proposal_b.Get("risk_score", 0.0);
        } else if (proposal_b.Find("strategy") >= 0) {
            // Infer risk from strategy name
            String strategy = proposal_b.Get("strategy", String(""));
            if (strategy.Find("aggressive") >= 0) risk_b = 0.8;
            else if (strategy.Find("conservative") >= 0) risk_b = 0.2;
            else risk_b = 0.5;  // Default
        }
    }
    
    if (risk_a < risk_b) {
        tradeoff.id = "choose_agent_" + plan_a.agent_name;
        tradeoff.description = "Prefer lower risk action: " + plan_a.agent_name + " vs " + plan_b.agent_name;
        tradeoff.score = 1.0 - risk_a;  // Higher score for lower risk
        
        ValueMap rationale;
        rationale.Set("reason", "Lower risk preference");
        rationale.Set("agent_a_risk", risk_a);
        rationale.Set("agent_b_risk", risk_b);
        tradeoff.rationale = rationale;
    } else if (risk_b < risk_a) {
        tradeoff.id = "choose_agent_" + plan_b.agent_name;
        tradeoff.description = "Prefer lower risk action: " + plan_b.agent_name + " vs " + plan_a.agent_name;
        tradeoff.score = 1.0 - risk_b;  // Higher score for lower risk
        
        ValueMap rationale;
        rationale.Set("reason", "Lower risk preference");
        rationale.Set("agent_a_risk", risk_a);
        rationale.Set("agent_b_risk", risk_b);
        tradeoff.rationale = rationale;
    }
    
    return tradeoff;
}

TradeOff CoreConflictResolver::PreferHigherBenefit(const ConflictDetail& conflict,
                                                    const AgentPlan& plan_a,
                                                    const AgentPlan& plan_b) {
    TradeOff tradeoff;
    
    // Determine benefit levels of each plan (simplified approach)
    double benefit_a = 0.0, benefit_b = 0.0;
    
    if (IsValueMap(plan_a.proposal)) {
        const ValueMap& proposal_a = ValueTo<ValueMap>(plan_a.proposal);
        if (proposal_a.Find("benefit_score") >= 0) {
            benefit_a = proposal_a.Get("benefit_score", 0.0);
        } else if (proposal_a.Find("confidence_score") >= 0) {
            benefit_a = proposal_a.Get("confidence_score", 0.0);
        }
    }
    
    if (IsValueMap(plan_b.proposal)) {
        const ValueMap& proposal_b = ValueTo<ValueMap>(plan_b.proposal);
        if (proposal_b.Find("benefit_score") >= 0) {
            benefit_b = proposal_b.Get("benefit_score", 0.0);
        } else if (proposal_b.Find("confidence_score") >= 0) {
            benefit_b = proposal_b.Get("confidence_score", 0.0);
        }
    }
    
    if (benefit_a > benefit_b) {
        tradeoff.id = "choose_agent_" + plan_a.agent_name;
        tradeoff.description = "Prefer higher benefit action: " + plan_a.agent_name + " vs " + plan_b.agent_name;
        tradeoff.score = benefit_a;
        
        ValueMap rationale;
        rationale.Set("reason", "Higher benefit preference");
        rationale.Set("agent_a_benefit", benefit_a);
        rationale.Set("agent_b_benefit", benefit_b);
        tradeoff.rationale = rationale;
    } else if (benefit_b > benefit_a) {
        tradeoff.id = "choose_agent_" + plan_b.agent_name;
        tradeoff.description = "Prefer higher benefit action: " + plan_b.agent_name + " vs " + plan_a.agent_name;
        tradeoff.score = benefit_b;
        
        ValueMap rationale;
        rationale.Set("reason", "Higher benefit preference");
        rationale.Set("agent_a_benefit", benefit_a);
        rationale.Set("agent_b_benefit", benefit_b);
        tradeoff.rationale = rationale;
    }
    
    return tradeoff;
}

TradeOff CoreConflictResolver::HonorPriority(const ConflictDetail& conflict,
                                             const AgentProfile& profile_a,
                                             const AgentProfile& profile_b) {
    TradeOff tradeoff;
    
    // Calculate overall priority for each agent based on their goals
    double priority_a = 0.0, priority_b = 0.0;
    int goal_count_a = profile_a.goals.GetCount();
    int goal_count_b = profile_b.goals.GetCount();
    
    for (const Goal& goal : profile_a.goals) {
        priority_a += goal.priority;
    }
    if (goal_count_a > 0) {
        priority_a /= goal_count_a;
    }
    
    for (const Goal& goal : profile_b.goals) {
        priority_b += goal.priority;
    }
    if (goal_count_b > 0) {
        priority_b /= goal_count_b;
    }
    
    if (priority_a > priority_b) {
        tradeoff.id = "honor_priority_" + profile_a.name;
        tradeoff.description = "Honor higher priority agent: " + profile_a.name + " vs " + profile_b.name;
        tradeoff.score = priority_a;
        
        ValueMap rationale;
        rationale.Set("reason", "Higher priority agent preference");
        rationale.Set("agent_a_priority", priority_a);
        rationale.Set("agent_b_priority", priority_b);
        tradeoff.rationale = rationale;
    } else if (priority_b > priority_a) {
        tradeoff.id = "honor_priority_" + profile_b.name;
        tradeoff.description = "Honor higher priority agent: " + profile_b.name + " vs " + profile_a.name;
        tradeoff.score = priority_b;
        
        ValueMap rationale;
        rationale.Set("reason", "Higher priority agent preference");
        rationale.Set("agent_a_priority", priority_a);
        rationale.Set("agent_b_priority", priority_b);
        tradeoff.rationale = rationale;
    }
    
    return tradeoff;
}