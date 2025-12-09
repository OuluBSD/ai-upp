#include "clicore.h"
#include "CoreProposal.h"
#include "CoreIde.h"
#include <Core/Core.h>
#include <SHA1.h>

using namespace Upp;

CoreProposal::CoreProposal() {
}

CoreProposal::Proposal CoreProposal::BuildProposal(CoreIde& ide,
                                                    const String& package,
                                                    int max_actions,
                                                    String& error) {
    Proposal proposal;
    proposal.package = package;
    
    // Generate supervisor plan
    CoreSupervisor::Plan sup_plan = ide.GetSupervisor().GenerateOptimizationPlan(package, ide, error);
    if (!error.IsEmpty()) {
        return proposal; // Return empty proposal on error
    }
    
    // Build scenario plan from supervisor plan
    CoreScenario::ScenarioPlan scenario_plan = ide.GetSupervisor().BuildScenario(package, max_actions, ide, error);
    if (!error.IsEmpty()) {
        return proposal; // Return empty proposal on error
    }
    
    // Simulate scenario WITHOUT applying changes
    CoreScenario::ScenarioResult simulation_result = ide.GetScenario().Simulate(scenario_plan, ide, error);
    if (!error.IsEmpty()) {
        return proposal; // Return empty proposal on error
    }
    
    // Fill in proposal fields
    proposal.supervisor_plan = AsValueMap(sup_plan);
    proposal.scenario_plan = AsValueMap(scenario_plan);
    proposal.simulation_before = AsValue(simulation_result.before);
    proposal.simulation_after = AsValue(simulation_result.after);
    proposal.deltas = AsValue(simulation_result.deltas);
    proposal.patch = String(); // Empty in v1 since no changes are applied
    proposal.file_changes = AsValueMap(simulation_result.file_changes);
    
    // For semantic and architecture snapshots, we'll extract from simulation
    proposal.semantic_snapshot = simulation_result.before.semantic.IsMap() ? 
                                 AsValueMap(simulation_result.before.semantic) : 
                                 ValueMap();
    proposal.architecture_snapshot = simulation_result.before.architecture.IsMap() ? 
                                     AsValueMap(simulation_result.before.architecture) : 
                                     ValueMap();
    
    // Calculate risk, confidence, and benefit scores based on supervisor metrics
    // For now, use the score from the simulation result as a simple aggregate
    proposal.risk_score = simulation_result.before.score; // Placeholder
    proposal.confidence_score = simulation_result.after.score; // Placeholder
    proposal.benefit_score = simulation_result.after.score - simulation_result.before.score; // Difference as benefit
    
    // Generate deterministic ID (using SHA1 of serialized scenario plan)
    String serialized_plan = AsString(AsValue(scenario_plan));
    proposal.id = SHA1String(serialized_plan);
    
    // Add metadata with timestamp
    proposal.metadata.Add("timestamp", GetSysTime().ToString());
    proposal.metadata.Add("strategy", ide.GetActiveStrategy() ? 
                         ide.GetActiveStrategy()->name : "default");
    
    return proposal;
}

Value CoreProposal::ToValue(const Proposal& p) const {
    ValueMap vm;
    vm.Add("id", p.id);
    vm.Add("package", p.package);
    vm.Add("supervisor_plan", p.supervisor_plan);
    vm.Add("scenario_plan", p.scenario_plan);
    vm.Add("simulation_before", p.simulation_before);
    vm.Add("simulation_after", p.simulation_after);
    vm.Add("deltas", p.deltas);
    vm.Add("patch", p.patch);
    vm.Add("file_changes", p.file_changes);
    vm.Add("semantic_snapshot", p.semantic_snapshot);
    vm.Add("architecture_snapshot", p.architecture_snapshot);
    vm.Add("risk_score", p.risk_score);
    vm.Add("confidence_score", p.confidence_score);
    vm.Add("benefit_score", p.benefit_score);
    vm.Add("metadata", p.metadata);
    
    return vm;
}

// Helper function to convert CoreSupervisor::Plan to ValueMap
static ValueMap AsValueMap(const CoreSupervisor::Plan& plan) {
    ValueMap vm;
    vm.Add("summary", plan.summary);
    vm.Add("strategy_info", plan.strategy_info);
    vm.Add("semantic_snapshot", plan.semantic_snapshot);
    
    ValueArray steps;
    for (const auto& step : plan.steps) {
        ValueMap stepMap;
        stepMap.Add("action", step.action);
        stepMap.Add("target", step.target);
        stepMap.Add("params", step.params);
        stepMap.Add("reason", step.reason);
        stepMap.Add("benefit_score", step.benefit_score);
        stepMap.Add("cost_score", step.cost_score);
        stepMap.Add("risk_score", step.risk_score);
        stepMap.Add("confidence_score", step.confidence_score);
        stepMap.Add("metrics", step.metrics);
        steps.Add(stepMap);
    }
    vm.Add("steps", steps);
    
    return vm;
}

// Helper function to convert CoreScenario::ScenarioPlan to ValueMap
static ValueMap AsValueMap(const CoreScenario::ScenarioPlan& plan) {
    ValueMap vm;
    vm.Add("name", plan.name);
    vm.Add("metadata", plan.metadata);
    
    ValueArray actions;
    for (const auto& action : plan.actions) {
        ValueMap actionMap;
        actionMap.Add("type", action.type);
        actionMap.Add("target", action.target);
        actionMap.Add("params", action.params);
        actions.Add(actionMap);
    }
    vm.Add("actions", actions);
    
    return vm;
}

// Helper function to convert ScenarioSnapshot to Value
static Value AsValue(const CoreScenario::ScenarioSnapshot& snapshot) {
    ValueMap vm;
    vm.Add("telemetry", snapshot.telemetry);
    vm.Add("semantic", snapshot.semantic);
    vm.Add("architecture", snapshot.architecture);
    vm.Add("behavior", snapshot.behavior);
    vm.Add("score", snapshot.score);
    return vm;
}