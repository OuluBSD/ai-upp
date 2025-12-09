#ifndef CMD_IDESESSION_H
#define CMD_IDESESSION_H

#include <Core/Core.h>

namespace Upp {

class IdeSession : public Pte<IdeSession> {
public:
    virtual ~IdeSession() {}

    // Workspace management
    virtual bool SetWorkspaceRoot(const String& root, String& error) = 0;

    // File operations
    virtual bool OpenFile(const String& path, String& error) = 0;
    virtual bool SaveFile(const String& path, String& error) = 0;

    // Editor operations
    virtual bool EditorInsert(const String& path, int pos, const String& text, String& error) = 0;
    virtual bool EditorErase(const String& path, int pos, int count, String& error) = 0;
    virtual bool EditorReplace(const String& path, int pos, int count, const String& replacement, String& error) = 0;
    virtual bool EditorGotoLine(const String& path, int line, int& out_pos, String& error) = 0;
    virtual bool EditorFindFirst(const String& path, const String& pattern, int start_pos,
                                 bool case_sensitive, int& out_pos, String& error) = 0;
    virtual bool EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                                  bool case_sensitive, int& out_count, String& error) = 0;
    virtual bool EditorUndo(const String& path, String& error) = 0;
    virtual bool EditorRedo(const String& path, String& error) = 0;

    // Project/build operations
    virtual bool SetMainPackage(const String& package, String& error) = 0;
    virtual bool BuildProject(const String& project, const String& config, String& log, String& error) = 0;
    virtual bool CleanProject(const String& project, String& log, String& error) = 0;

    // Navigation / misc
    virtual bool GotoLine(const String& file, int line, String& error) = 0;
    virtual bool ShowConsole(String& error) = 0;
    virtual bool ShowErrors(String& error) = 0;

    // Additional methods for other commands
    virtual bool FindInFiles(const String& pattern, const String& directory, String& result, String& error) = 0;
    virtual bool SearchCode(const String& query, String& result, String& error) = 0;

    // Symbol assistance methods
    virtual bool FindDefinition(const String& symbol, String& file, int& line, String& error) = 0;
    virtual bool FindUsages(const String& symbol, Vector<String>& locs, String& error) = 0;

    // Methods to retrieve console and error output
    virtual bool GetConsoleOutput(String& output, String& error) = 0;
    virtual bool GetErrorsOutput(String& output, String& error) = 0;

    // Graph operations
    virtual bool GetBuildOrder(Vector<String>& out_order, String& error) = 0;
    virtual bool FindCycles(Vector<Vector<String>>& out_cycles, String& error) = 0;
    virtual bool AffectedPackages(const String& filepath,
                                  Vector<String>& out_packages,
                                  String& error) = 0;

    // Refactoring operations (PART F - Add these virtual methods)
    virtual bool RenameSymbol(const String& old_name,
                              const String& new_name,
                              String& error) = 0;

    virtual bool RemoveDeadIncludes(const String& path,
                                    String& error,
                                    int* out_count = nullptr) = 0;

    virtual bool CanonicalizeIncludes(const String& path,
                                      String& error,
                                      int* out_count = nullptr) = 0;

    // Telemetry & Analytics v1 methods (PART C)
    virtual Value GetWorkspaceStats(String& error) = 0;
    virtual Value GetPackageStats(const String& pkg, String& error) = 0;
    virtual Value GetFileComplexity(const String& path, String& error) = 0;
    virtual Value GetGraphStats(String& error) = 0;
    virtual Value GetEditHistory(String& error) = 0;

    // Optimization Loop v1 (PART C)
    virtual Value OptimizePackage(
        const String& package,
        int max_iterations,
        double converge_threshold,
        bool stop_on_worse,
        bool stop_on_converge,
        String& error
    ) = 0;

    // AI Supervisor Layer v1 (PART C)
    virtual Value GetOptimizationPlan(const String& package, String& error) = 0;

    // AI Supervisor Layer v2 - Workspace planning
    virtual Value GetWorkspacePlan(String& error) = 0;

    // Dynamic Strategy Engine (PART E)
    virtual bool SetActiveStrategy(const String& name, String& error) = 0;
    virtual Value GetActiveStrategy(String& error) = 0;
    virtual Value GetStrategy(const String& name, String& error) = 0;  // Get specific strategy by name
    virtual Value ListStrategies(String& error) = 0;

    // Semantic Analysis v1
    virtual Value GetSemanticEntities(String& error) = 0;
    virtual Value GetSemanticClusters(String& error) = 0;
    virtual Value SearchSemanticEntities(const String& pattern, String& error) = 0;

    // Semantic Analysis v2 - NEW: Inference layer methods
    virtual Value GetSemanticSubsystems(String& error) = 0;
    virtual Value GetSemanticEntity(const String& name, String& error) = 0;
    virtual Value GetSemanticRoles(String& error) = 0;
    virtual Value GetSemanticLayers(String& error) = 0;

    // Semantic Analysis v3 - NEW: Behavioral analysis methods
    virtual Value GetSemanticBehavior(String& error) = 0;
    virtual Value GetSemanticBehaviorEntity(const String& name, String& error) = 0;
    virtual Value GetSemanticBehaviorGraph(String& error) = 0;
    virtual Value GetSemanticPipeline(String& error) = 0;

    // Semantic Analysis v4 - NEW: Architecture diagnostic methods
    virtual Value GetSemanticPatterns(String& error) = 0;
    virtual Value GetSemanticAntiPatterns(String& error) = 0;
    virtual Value GetSemanticArchitecture(String& error) = 0;
    virtual Value GetSemanticOutliers(String& error) = 0;

    // Scenario operations (PART C)
    virtual Value BuildScenario(const String& package,
                                int max_actions,
                                String& error) = 0;

    virtual Value SimulateScenario(const Value& plan_desc,
                                   String& error) = 0;

    virtual Value ApplyScenario(const Value& plan_desc,
                                String& error) = 0;

    // Revert patch functionality (PART D)
    virtual Value RevertPatch(const String& patch_text, String& error) = 0;

    // Proposal generation v1
    virtual Value BuildProposal(const String& package,
                                int max_actions,
                                String& error) = 0;
};

One<IdeSession> CreateIdeSession();

}

#endif