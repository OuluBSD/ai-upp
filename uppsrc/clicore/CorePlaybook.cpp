#include "clicore.h"
#include "CorePlaybook.h"

#include <Core/JSON.h>

namespace Upp {

CorePlaybook::CorePlaybook() {
}

bool CorePlaybook::Load(const String& path, String& error) {
    playbooks.Clear();
    
    try {
        String json = LoadFile(path);
        if (json.IsEmpty()) {
            error = "Playbooks file not found or empty: " + path;
            return false;
        }
        
        Json js = ParseJSON(json);
        if (!js.IsArray()) {
            error = "Playbooks file must contain an array of playbooks";
            return false;
        }
        
        for(int i = 0; i < js.GetCount(); i++) {
            const Json& item = js[i];
            if (!item.IsObject()) continue;
            
            Playbook pb;
            pb.id = item("id");
            pb.description = item("description");
            pb.safety_level = item("safety_level", 0.5);
            
            // Parse constraints
            const Json& constraints = item("constraints", Json::Object());
            if (constraints.IsObject()) {
                for(int j = 0; j < constraints.GetCount(); j++) {
                    pb.constraints.GetAdd(constraints.GetKey(j)) = constraints[j];
                }
            }
            
            // Parse steps
            const Json& steps = item("steps", Json::Array());
            if (steps.IsArray()) {
                for(int j = 0; j < steps.GetCount(); j++) {
                    const Json& step = steps[j];
                    if (!step.IsObject()) continue;
                    
                    PlaybookStep pbStep;
                    pbStep.id = step("id");
                    pbStep.action = step("action");
                    
                    // Parse parameters
                    const Json& params = step("params", Json::Object());
                    if (params.IsObject()) {
                        for(int k = 0; k < params.GetCount(); k++) {
                            pbStep.params.GetAdd(params.GetKey(k)) = params[k];
                        }
                    }
                    
                    pb.steps.Add(pbStep);
                }
            }
            
            playbooks.Add(pb);
        }
        
        return true;
    }
    catch (const std::exception& e) {
        error = String(e.what());
        return false;
    }
}

Vector<Playbook> CorePlaybook::GetAll() const {
    return playbooks;
}

const Playbook* CorePlaybook::Find(const String& id) const {
    for(int i = 0; i < playbooks.GetCount(); i++) {
        if (playbooks[i].id == id)
            return &playbooks[i];
    }
    return nullptr;
}

Value CorePlaybook::Run(const Playbook& pb, CoreIde& ide, String& error) const {
    Value shared_state;
    Value result = ValueMap();

    try {
        ValueArray steps_result;

        for(int i = 0; i < pb.steps.GetCount(); i++) {
            const PlaybookStep& step = pb.steps[i];

            // Check if this step violates any safety constraints
            if (step.action == "apply_scenario" || step.action == "apply_if_safe") {
                // Check if apply is allowed by playbook constraints
                if (!pb.constraints.Get("allow_apply", true)) {
                    ValueMap blocked;
                    blocked.GetAdd("status") = "blocked_by_constraint";
                    blocked.GetAdd("reason") = "apply operations not allowed by playbook constraints";

                    ValueMap step_info;
                    step_info.GetAdd("id") = step.id;
                    step_info.GetAdd("action") = step.action;
                    step_info.GetAdd("result") = blocked;
                    steps_result.Add(step_info);

                    continue; // Skip this step but continue with the playbook
                }

                // Check max risk constraint
                double max_risk = pb.constraints.Get("max_risk", 1.0);
                Value proposal = shared_state("proposal", Value());
                if (!proposal.IsVoid()) {
                    double risk_score = proposal("risk_score", 1.0);
                    if (risk_score > max_risk) {
                        ValueMap blocked;
                        blocked.GetAdd("status") = "blocked_by_constraint";
                        blocked.GetAdd("reason") = "risk exceeds max_risk constraint";
                        blocked.GetAdd("risk_score") = risk_score;
                        blocked.GetAdd("max_risk") = max_risk;

                        ValueMap step_info;
                        step_info.GetAdd("id") = step.id;
                        step_info.GetAdd("action") = step.action;
                        step_info.GetAdd("result") = blocked;
                        steps_result.Add(step_info);

                        continue; // Skip this step but continue with the playbook
                    }
                }
            }

            Value step_result = RunStep(pb, step, ide, shared_state, error);
            if (!error.IsEmpty()) {
                result("error", error);
                result("completed_steps", i);
                result("status", "failed");
                return result;
            }

            ValueMap step_info;
            step_info.GetAdd("id") = step.id;
            step_info.GetAdd("action") = step.action;
            step_info.GetAdd("result") = step_result;
            steps_result.Add(step_info);
        }

        result("status", "success");
        result("steps") = steps_result;
        result("playbook_id") = pb.id;
        result("safety_level") = pb.safety_level;

        return result;
    }
    catch (const std::exception& e) {
        error = String(e.what());
        result("error", error);
        result("status", "failed");
        return result;
    }
}

Value CorePlaybook::RunStep(const Playbook& pb,
                           const PlaybookStep& step,
                           CoreIde& ide,
                           Value& shared_state,
                           String& error) const {
    try {
        if (step.action == "scan_workspace") {
            // Get workspace stats and store in shared state
            String ws_error;
            Value ws_result = ide.GetWorkspaceStats(ws_error);
            if (!ws_error.IsEmpty()) {
                error = "Scan workspace failed: " + ws_error;
                return Value();
            }
            
            shared_state("workspace_stats") = ws_result;
            return ws_result;
        }
        else if (step.action == "generate_proposal") {
            String package = step.params.Get("package", "auto_main");
            
            // If package is "auto_main", get it from workspace
            if (package == "auto_main") {
                String ws_error;
                Value ws_stats = ide.GetWorkspaceStats(ws_error);
                if (!ws_error.IsEmpty()) {
                    error = "Could not get workspace stats for auto_main: " + ws_error;
                    return Value();
                }
                package = ws_stats("main_package", "unknown");
            }
            
            String prop_error;
            Value prop_result = ide.GetOptimizationProposal(
                package,
                step.params.Get("max_actions", 10),
                step.params.Get("with_futures", true),
                prop_error
            );
            
            if (!prop_error.IsEmpty()) {
                error = "Generate proposal failed: " + prop_error;
                return Value();
            }
            
            shared_state("proposal") = prop_result;
            return prop_result;
        }
        else if (step.action == "simulate") {
            // Placeholder for simulation step
            Value sim_result = ValueMap().GetAdd("simulation", "placeholder");
            shared_state("simulation") = sim_result;
            return sim_result;
        }
        else if (step.action == "resolve_conflicts") {
            // Placeholder for conflict resolution step
            String resolve_error;
            Value resolve_result = ide.ResolveConflicts(resolve_error);
            if (!resolve_error.IsEmpty()) {
                error = "Conflict resolution failed: " + resolve_error;
                return Value();
            }
            
            shared_state("resolved") = resolve_result;
            return resolve_result;
        }
        else if (step.action == "apply_if_safe") {
            // Check safety constraints before applying
            double max_risk = pb.constraints.Get("max_risk", 1.0);
            int max_actions = pb.constraints.Get("max_actions", 100);  // Default to 100 if not specified
            bool allow_apply = pb.constraints.Get("allow_apply", true);
            bool require_positive_benefit = step.params.Get("require_positive_benefit", false);

            // Check if apply is allowed at all
            if (!allow_apply) {
                ValueMap blocked;
                blocked.GetAdd("status") = "apply_not_allowed_by_constraints";
                return blocked;
            }

            // Get proposal from shared state
            Value proposal = shared_state("proposal", Value());
            if (proposal.IsVoid()) {
                error = "No proposal available for apply_if_safe";
                return Value();
            }

            // Check max actions constraint
            Value actions = proposal("actions", Value());
            if (actions.IsArray()) {
                int action_count = actions.GetCount();
                if (action_count > max_actions) {
                    ValueMap blocked;
                    blocked.GetAdd("status") = "exceeds_max_actions";
                    blocked.GetAdd("action_count") = action_count;
                    blocked.GetAdd("max_actions") = max_actions;
                    return blocked;
                }
            }

            // Check safety constraints
            double risk_score = proposal("risk_score", 1.0);
            double benefit_score = proposal("benefit_score", 0.0);

            if (risk_score > max_risk) {
                ValueMap blocked;
                blocked.GetAdd("status") = "blocked_by_risk";
                blocked.GetAdd("risk_score") = risk_score;
                blocked.GetAdd("max_risk") = max_risk;
                return blocked;
            }

            if (require_positive_benefit && benefit_score <= 0) {
                ValueMap blocked;
                blocked.GetAdd("status") = "blocked_by_negative_benefit";
                blocked.GetAdd("benefit_score") = benefit_score;
                return blocked;
            }

            // Actually apply the changes
            String apply_error;
            Value apply_result = ide.ApplyScenario(proposal, apply_error);
            if (!apply_error.IsEmpty()) {
                error = "Apply scenario failed: " + apply_error;
                return Value();
            }

            shared_state("applied") = apply_result;
            return apply_result;
        }
        else if (step.action == "record_evolution_summary") {
            String ev_error;
            Value ev_result = ide.GetEvolutionSummary(ev_error);
            if (!ev_error.IsEmpty()) {
                error = "Record evolution summary failed: " + ev_error;
                return Value();
            }
            
            shared_state("evolution_summary") = ev_result;
            return ev_result;
        }
        else if (step.action == "apply_scenario") {
            // Check if apply is allowed by constraints
            if (!pb.constraints.Get("allow_apply", true)) {
                ValueMap blocked;
                blocked.GetAdd("status") = "apply_not_allowed_by_constraints";
                return blocked;
            }

            // Check max risk constraint
            double max_risk = pb.constraints.Get("max_risk", 1.0);
            Value scenario = step.params.Get("scenario", Value());
            if (scenario.IsVoid()) {
                scenario = shared_state("scenario", Value());
            }

            if (scenario.IsVoid()) {
                error = "No scenario available for apply_scenario";
                return Value();
            }

            // Check max actions constraint
            int max_actions = pb.constraints.Get("max_actions", 100);
            Value actions = scenario("actions", Value());
            if (actions.IsArray()) {
                int action_count = actions.GetCount();
                if (action_count > max_actions) {
                    ValueMap blocked;
                    blocked.GetAdd("status") = "exceeds_max_actions";
                    blocked.GetAdd("action_count") = action_count;
                    blocked.GetAdd("max_actions") = max_actions;
                    return blocked;
                }
            }

            // Check risk score if available
            double risk_score = scenario("risk_score", 1.0);
            if (risk_score > max_risk) {
                ValueMap blocked;
                blocked.GetAdd("status") = "blocked_by_risk";
                blocked.GetAdd("risk_score") = risk_score;
                blocked.GetAdd("max_risk") = max_risk;
                return blocked;
            }

            String apply_error;
            Value apply_result = ide.ApplyScenario(scenario, apply_error);
            if (!apply_error.IsEmpty()) {
                error = "Apply scenario failed: " + apply_error;
                return Value();
            }

            shared_state("applied") = apply_result;
            return apply_result;
        }
        else {
            error = "Unknown action: " + step.action;
            return Value();
        }
    }
    catch (const std::exception& e) {
        error = String(e.what());
        return Value();
    }
}

}