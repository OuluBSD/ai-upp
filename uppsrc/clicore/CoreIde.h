#ifndef _clicore_CoreIde_h_
#define _clicore_CoreIde_h_

#include <Core/Core.h>
#include "CoreWorkspace.h"
#include "CoreBuild.h"
#include "CoreSearch.h"
#include "CoreConsole.h"
#include "CoreFileOps.h"
#include "CoreEditor.h"
#include "CoreAssist.h"
#include "CoreGraph.h"
#include "CoreRefactor.h"
#include "CoreTelemetry.h"
#include "CoreOptimize.h"
#include "CoreSupervisor.h"
#include "StrategyProfile.h"
#include "CoreSemantic.h"
#include "CoreScenario.h"

using namespace Upp;

class CoreIde {
public:
    CoreIde();
    ~CoreIde();

    // File operations
    bool OpenFile(const String& path, String& error);
    bool SaveFile(const String& path, String& error);

    // Editor operations
    CoreEditor* FindEditorForPath(const String& path);
    CoreEditor* GetCurrentEditor();
    CoreEditor* OpenEditor(const String& path, String& error);
    bool CloseFile(const String& path, String& error);

    // Editor-specific operations
    bool EditorInsert(const String& path, int pos, const String& text, String& error);
    bool EditorErase(const String& path, int pos, int count, String& error);
    bool EditorReplace(const String& path, int pos, int count, const String& replacement, String& error);
    bool EditorGotoLine(const String& path, int line, int& out_pos, String& error);
    bool EditorFindFirst(const String& path, const String& pattern, int start_pos,
                         bool case_sensitive, int& out_pos, String& error);
    bool EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                          bool case_sensitive, int& out_count, String& error);
    bool EditorUndo(const String& path, String& error);
    bool EditorRedo(const String& path, String& error);

    // Workspace management
    bool SetWorkspaceRoot(const String& root, String& error);
    const String& GetWorkspaceRoot() const;

    // Project/package operations
    bool SetMainPackage(const String& package, String& error);
    bool GetMainPackage(CoreWorkspace::Package& out, String& error) const;

    // Build operations
    bool BuildProject(const String& project, const String& config, String& log, String& error);
    bool CleanProject(const String& project, String& log, String& error);

    // Navigation
    bool GotoLine(const String& file, int line, String& error);

    // Search / code search
    bool FindInFiles(const String& pattern, const String& directory, bool replace, String& result, String& error);
    bool SearchCode(const String& query, String& result, String& error);

    // Symbol assistance
    bool IndexWorkspace(String& error);
    bool FindSymbolDefinition(const String& symbol, String& file, int& line, String& error);
    bool FindSymbolUsages(const String& symbol, Vector<String>& locs, String& error);

    // Graph operations
    bool RebuildGraph(String& error);
    bool GetBuildOrder(Vector<String>& out_order, String& error);
    bool GetCycles(Vector<Vector<String>>& out_cycles, String& error);
    bool GetAffectedPackages(const String& filepath, Vector<String>& out_packages, String& error);

    // Output
    bool GetConsoleOutput(String& out, String& error);
    bool GetErrorsOutput(String& out, String& error);

    // Refactoring operations
    bool RenameSymbol(const String& old, const String& nw, String& error);
    bool RemoveDeadIncludes(const String& path, String& error, int* out_count = nullptr);
    bool CanonicalizeIncludes(const String& path, String& error, int* out_count = nullptr);

    // Telemetry & Analytics v1
    Value GetWorkspaceStats();
    Value GetPackageStats(const String& pkg);
    Value GetPackageStats(const String& pkg, String& error);
    Value GetTelemetryData(const String& pkg, String& error);
    Value GetGraphStats(const String& pkg, String& error);
    Value GetFileComplexity(const String& path);
    Value GetGraphStats();
    Value GetEditHistory();

    // Optimization Loop v1
    Value RunOptimizationLoop(const String& package,
                              const CoreOptimize::LoopConfig& cfg,
                              String& error);

    // Supervisor v1 - Generate optimization plan for a package
    Value GenerateOptimizationPlan(const String& package, String& error);

    // Supervisor v2 - Generate optimization plan for the entire workspace
    Value GenerateWorkspacePlan(String& error);

    // Strategy registry management for Supervisor v2
    bool InitializeStrategies(const String& strategies_path, String& error);
    bool SetActiveStrategy(const String& name, String& error);
    const StrategyProfile* GetActiveStrategy() const;
    const Vector<StrategyProfile>& GetAllStrategies() const;

    // Semantic analysis v1
    bool AnalyzeSemantics(String& error);
    const CoreSemantic& GetSemanticAnalyzer() const;
    CoreSemantic& GetSemanticAnalyzer();

    // Scenario operations
    Value BuildScenarioFromPlan(const String& package,
                                int max_actions,
                                String& error);
    Value SimulateScenario(const Value& plan_desc,
                           String& error);
    Value ApplyScenario(const Value& plan_desc,
                        String& error);

    // Revert patch functionality
    Value RevertPatch(const String& patch_text, String& error);

    // Proposal generation v1
    Value BuildProposal(const String& package,
                        int max_actions,
                        String& error);

    // Getter methods for access to internal components
    CoreWorkspace& GetWorkspace() { return workspace; }
    const CoreWorkspace& GetWorkspace() const { return workspace; }
    CoreFileOps& GetFileOps() { return fileOps; }
    const CoreFileOps& GetFileOps() const { return fileOps; }

private:
    // Internal state: workspace, packages, logs, etc.
    CoreWorkspace workspace;
    CoreBuild build;
    CoreSearch search;
    CoreConsole console;
    CoreFileOps fileOps;
    CoreAssist assist;  // Added CoreAssist member
    CoreGraph graph;    // Added CoreGraph member
    CoreRefactor refactor;  // Added CoreRefactor member
    CoreTelemetry telemetry;  // Added CoreTelemetry member
    CoreOptimize optimizer;   // Added CoreOptimize member
    CoreSupervisor supervisor; // Added CoreSupervisor member
    CoreSemantic semantic;     // Added CoreSemantic member
    CoreScenario scenario;     // Added CoreScenario member
    CoreProposal proposal;     // Added CoreProposal member
    StrategyRegistry strategy_registry; // Added StrategyRegistry for Supervisor v2
    String workspace_root;

    // Core Editor management
    Array<CoreEditor> editors;  // Use One<> to ensure move-only semantics
    Index<String> editor_paths;  // Map from path to editor index
    int current_editor_index;    // Index of currently active editor
};

#endif