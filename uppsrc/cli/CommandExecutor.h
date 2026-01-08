#ifndef CMD_COMMANDEXECUTOR_H
#define CMD_COMMANDEXECUTOR_H

#include <Core/Core.h>
#include "Command.h"
#include "CommandRegistry.h"
#include "IdeSession.h"

namespace Upp {

class CommandExecutor {
public:
    CommandExecutor(const CommandRegistry& registry, One<IdeSession> session);

    InvocationResult Invoke(const String& name,
                           const VectorMap<String, String>& args);

private:
    // Handler functions for integrated commands (using IdeSession)
    InvocationResult HandleOpenFile(const VectorMap<String, String>& args);
    InvocationResult HandleSaveFile(const VectorMap<String, String>& args);
    InvocationResult HandleSetMainPackage(const VectorMap<String, String>& args);
    InvocationResult HandleBuildProject(const VectorMap<String, String>& args);
    InvocationResult HandleCleanProject(const VectorMap<String, String>& args);
    InvocationResult HandleGotoLine(const VectorMap<String, String>& args);

    // Handler functions for editor commands (using IdeSession)
    InvocationResult HandleInsertText(const VectorMap<String, String>& args);
    InvocationResult HandleEraseRange(const VectorMap<String, String>& args);
    InvocationResult HandleReplaceAll(const VectorMap<String, String>& args);
    InvocationResult HandleUndo(const VectorMap<String, String>& args);
    InvocationResult HandleRedo(const VectorMap<String, String>& args);

    // Handler functions for stub commands (not yet integrated)
    InvocationResult HandleFindInFiles(const VectorMap<String, String>& args);
    InvocationResult HandleSearchCode(const VectorMap<String, String>& args);
    InvocationResult HandleShowConsole(const VectorMap<String, String>& args);
    InvocationResult HandleShowErrors(const VectorMap<String, String>& args);

    // Handler functions for symbol analysis commands
    InvocationResult HandleFindDefinition(const VectorMap<String, String>& args);
    InvocationResult HandleFindUsages(const VectorMap<String, String>& args);

    // Handler functions for graph analysis commands
    InvocationResult HandleGetBuildOrder(const VectorMap<String, String>& args);
    InvocationResult HandleDetectCycles(const VectorMap<String, String>& args);
    InvocationResult HandleAffectedPackages(const VectorMap<String, String>& args);

    // Handler functions for refactoring commands
    InvocationResult HandleRenameSymbol(const VectorMap<String, String>& args);
    InvocationResult HandleRemoveDeadIncludes(const VectorMap<String, String>& args);
    InvocationResult HandleCanonicalizeIncludes(const VectorMap<String, String>& args);

    // Handler function for command introspection
    InvocationResult HandleDescribeCommand(const VectorMap<String, String>& args);

    // Handler functions for telemetry commands
    InvocationResult HandleWorkspaceStats(const VectorMap<String, String>& args);
    InvocationResult HandlePackageStats(const VectorMap<String, String>& args);
    InvocationResult HandleFileComplexity(const VectorMap<String, String>& args);
    InvocationResult HandleGraphStats(const VectorMap<String, String>& args);
    InvocationResult HandleEditHistory(const VectorMap<String, String>& args);

    // Optimization Loop v1 handler
    InvocationResult HandleOptimizePackage(const VectorMap<String, String>& args);

    // AI Supervisor Layer v1 handler
    InvocationResult HandleGetOptimizationPlan(const VectorMap<String, String>& args);

    // AI Supervisor Layer v2 handler
    InvocationResult HandleGetWorkspacePlan(const VectorMap<String, String>& args);

    // Dynamic Strategy Engine handlers
    InvocationResult HandleListStrategies(const VectorMap<String, String>& args);
    InvocationResult HandleGetStrategy(const VectorMap<String, String>& args);
    InvocationResult HandleSetStrategy(const VectorMap<String, String>& args);
    InvocationResult HandleSupervisorFront(const VectorMap<String, String>& args);
    InvocationResult HandleSupervisorRank(const VectorMap<String, String>& args);

    // Semantic Analysis v1 handlers
    InvocationResult HandleSemanticEntities(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticClusters(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticFind(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticAnalyze(const VectorMap<String, String>& args);

    // Semantic Analysis v2 - NEW: Inference layer handlers
    InvocationResult HandleSemanticSubsystems(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticEntity(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticRoles(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticLayers(const VectorMap<String, String>& args);

    // Semantic Analysis v3 - NEW: Behavioral analysis handlers
    InvocationResult HandleSemanticBehavior(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticBehaviorEntity(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticBehaviorGraph(const VectorMap<String, String>& args);
    InvocationResult HandleSemanticPipeline(const VectorMap<String, String>& args);

    // Scenario operation handlers (PART D)
    InvocationResult HandleBuildScenario(const VectorMap<String, String>& args);
    InvocationResult HandleSimulateScenario(const VectorMap<String, String>& args);
    InvocationResult HandleApplyScenario(const VectorMap<String, String>& args);

    // Patch and revert handlers (PART E)
    InvocationResult HandleScenarioDiff(const VectorMap<String, String>& args);
    InvocationResult HandleScenarioRevert(const VectorMap<String, String>& args);

    // Proposal export handler (PART F)
    InvocationResult HandleExportProposal(const VectorMap<String, String>& args);

    // Cross-Workspace Intelligence (CWI) v1 handlers
    InvocationResult HandleGlobalStats(const VectorMap<String, String>& args);
    InvocationResult HandleGlobalPredict(const VectorMap<String, String>& args);
    InvocationResult HandleExportGlobalKnowledge(const VectorMap<String, String>& args);
    InvocationResult HandleImportGlobalKnowledge(const VectorMap<String, String>& args);

    // Lifecycle Supervisor v1 handlers
    InvocationResult HandleLifecyclePhase(const VectorMap<String, String>& args);
    InvocationResult HandleLifecyclePhases(const VectorMap<String, String>& args);
    InvocationResult HandleLifecyclePredict(const VectorMap<String, String>& args);

    // Lifecycle Supervisor v2 handlers
    InvocationResult HandleLifecycleDrift(const VectorMap<String, String>& args);
    InvocationResult HandleLifecycleStability(const VectorMap<String, String>& args);
    InvocationResult HandleLifecycleTimeline(const VectorMap<String, String>& args);

    // Orchestrator v1 - Multi-project roadmap handlers
    InvocationResult HandleOrchestratorAddWorkspace(const VectorMap<String, String>& args);
    InvocationResult HandleOrchestratorSummaries(const VectorMap<String, String>& args);
    InvocationResult HandleOrchestratorRoadmap(const VectorMap<String, String>& args);

    // Temporal Strategy Engine v1 handlers
    InvocationResult HandleTemporalSeasonality(const VectorMap<String, String>& args);
    InvocationResult HandleTemporalCadence(const VectorMap<String, String>& args);
    InvocationResult HandleTemporalWindows(const VectorMap<String, String>& args);

    // Temporal Strategy Engine v2 - Forecasting & Shock Modeling command handlers
    InvocationResult HandleTemporalForecast(const VectorMap<String, String>& args);
    InvocationResult HandleTemporalRisk(const VectorMap<String, String>& args);
    InvocationResult HandleTemporalShock(const VectorMap<String, String>& args);

    // Strategic Navigator v1 - Multi-agent planning command handlers
    InvocationResult HandleListAgents(const VectorMap<String, String>& args);
    InvocationResult HandleAgentPlan(const VectorMap<String, String>& args);
    InvocationResult HandleGlobalPlan(const VectorMap<String, String>& args);

    // Conflict Resolver v1 - Patch-level negotiation command handlers
    InvocationResult HandleResolveConflicts(const VectorMap<String, String>& args);

    // Multi-Branch Futures & Outcome Horizon Engine - v2
    InvocationResult HandleExploreFutures(const VectorMap<String, String>& args);

    // Playbook Engine v1 - High-level workflow automation
    InvocationResult HandleListPlaybooks(const VectorMap<String, String>& args);
    InvocationResult HandleRunPlaybook(const VectorMap<String, String>& args);

    // Hybrid Instrument Commands
    InvocationResult HandleInstrumentBuildHybrid(const VectorMap<String, String>& args);
    InvocationResult HandleInstrumentRenderHybrid(const VectorMap<String, String>& args);

    // Regression Lab v1 - Agent-based regression testing
    InvocationResult HandleListRegressionSpecs(const VectorMap<String, String>& args);
    InvocationResult HandleRunRegression(const VectorMap<String, String>& args);
    InvocationResult HandleCompareRegressions(const VectorMap<String, String>& args);

    const CommandRegistry& registry;
    One<IdeSession> session;
};

}

#endif