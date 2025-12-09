#include "IdeSession.h"
#include <clicore/CoreIde.h>
#include <clicore/StrategyProfile.h>
#include <Core/Core.h>

namespace Upp {

class IdeSessionImpl : public IdeSession {
public:
    IdeSessionImpl();
    virtual ~IdeSessionImpl();

    // Workspace management
    virtual bool SetWorkspaceRoot(const String& root, String& error) override;

    // File operations
    virtual bool OpenFile(const String& path, String& error) override;
    virtual bool SaveFile(const String& path, String& error) override;

    // Editor operations
    virtual bool EditorInsert(const String& path, int pos, const String& text, String& error) override;
    virtual bool EditorErase(const String& path, int pos, int count, String& error) override;
    virtual bool EditorReplace(const String& path, int pos, int count, const String& replacement, String& error) override;
    virtual bool EditorGotoLine(const String& path, int line, int& out_pos, String& error) override;
    virtual bool EditorFindFirst(const String& path, const String& pattern, int start_pos,
                                 bool case_sensitive, int& out_pos, String& error) override;
    virtual bool EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                                  bool case_sensitive, int& out_count, String& error) override;
    virtual bool EditorUndo(const String& path, String& error) override;
    virtual bool EditorRedo(const String& path, String& error) override;

    // Project/build operations
    virtual bool SetMainPackage(const String& package, String& error) override;
    virtual bool BuildProject(const String& project, const String& config, String& log, String& error) override;
    virtual bool CleanProject(const String& project, String& log, String& error) override;

    // Navigation / misc
    virtual bool GotoLine(const String& file, int line, String& error) override;
    virtual bool ShowConsole(String& error) override;
    virtual bool ShowErrors(String& error) override;

    // Additional methods for other commands
    virtual bool FindInFiles(const String& pattern, const String& directory, String& result, String& error) override;
    virtual bool SearchCode(const String& query, String& result, String& error) override;

    // Symbol assistance methods
    virtual bool FindDefinition(const String& symbol, String& file, int& line, String& error) override;
    virtual bool FindUsages(const String& symbol, Vector<String>& locs, String& error) override;

    // Methods to retrieve console and error output
    virtual bool GetConsoleOutput(String& output, String& error) override;
    virtual bool GetErrorsOutput(String& output, String& error) override;

    // Graph operations
    virtual bool GetBuildOrder(Vector<String>& out_order, String& error) override;
    virtual bool FindCycles(Vector<Vector<String>>& out_cycles, String& error) override;
    virtual bool AffectedPackages(const String& filepath,
                                  Vector<String>& out_packages,
                                  String& error) override;

    // Refactoring operations
    virtual bool RenameSymbol(const String& old_name,
                              const String& new_name,
                              String& error) override;

    virtual bool RemoveDeadIncludes(const String& path,
                                    String& error) override;

    virtual bool CanonicalizeIncludes(const String& path,
                                      String& error) override;

    // Telemetry & Analytics v1 methods (PART C)
    virtual Value GetWorkspaceStats(String& error) override;
    virtual Value GetPackageStats(const String& pkg, String& error) override;
    virtual Value GetFileComplexity(const String& path, String& error) override;
    virtual Value GetGraphStats(String& error) override;
    virtual Value GetEditHistory(String& error) override;

    // Optimization Loop v1 (PART C)
    virtual Value OptimizePackage(
        const String& package,
        int max_iterations,
        double converge_threshold,
        bool stop_on_worse,
        bool stop_on_converge,
        String& error
    ) override;

    // AI Supervisor Layer v1 (PART C)
    virtual Value GetOptimizationPlan(const String& package, String& error) override;

    // AI Supervisor Layer v2 - Workspace planning
    virtual Value GetWorkspacePlan(String& error) override;

    // Dynamic Strategy Engine (PART E)
    virtual bool SetActiveStrategy(const String& name, String& error) override;
    virtual Value GetActiveStrategy(String& error) override;
    virtual Value GetStrategy(const String& name, String& error) override;  // Get specific strategy by name
    virtual Value ListStrategies(String& error) override;

private:
    CoreIde core_ide;
};

IdeSessionImpl::IdeSessionImpl() {
    // Initialize the session with the CoreIde
    // CoreIde constructor does basic initialization
}

IdeSessionImpl::~IdeSessionImpl() {
    // Cleanup session resources
}

bool IdeSessionImpl::SetWorkspaceRoot(const String& root, String& error) {
    return core_ide.SetWorkspaceRoot(root, error);
}

bool IdeSessionImpl::OpenFile(const String& path, String& error) {
    return core_ide.OpenFile(path, error);
}

bool IdeSessionImpl::SaveFile(const String& path, String& error) {
    return core_ide.SaveFile(path, error);
}

bool IdeSessionImpl::SetMainPackage(const String& package, String& error) {
    return core_ide.SetMainPackage(package, error);
}

bool IdeSessionImpl::BuildProject(const String& project, const String& config, String& log, String& error) {
    return core_ide.BuildProject(project, config, log, error);
}

bool IdeSessionImpl::CleanProject(const String& project, String& log, String& error) {
    return core_ide.CleanProject(project, log, error);
}

bool IdeSessionImpl::GotoLine(const String& file, int line, String& error) {
    // For now, delegate to CoreIde or implement basic validation
    // CoreIde doesn't have this functionality yet, so we'll validate basic parameters
    if (!FileExists(file)) {
        error = "File does not exist: " + file;
        return false;
    }

    if (line <= 0) {
        error = "Invalid line number: " + AsString(line);
        return false;
    }

    // In the future, CoreIde will handle this functionality
    error = "TODO: Implement GotoLine in CoreIde";
    return false;
}

bool IdeSessionImpl::ShowConsole(String& error) {
    // For a headless session, "showing" the console just means it's accessible
    // This is a no-op in our implementation
    return true;
}

bool IdeSessionImpl::ShowErrors(String& error) {
    // For a headless session, "showing" errors just means they're accessible
    // This is a no-op in our implementation
    return true;
}

bool IdeSessionImpl::FindInFiles(const String& pattern, const String& directory, String& result, String& error) {
    bool replace = false; // For now, we only support find, not replace
    return core_ide.FindInFiles(pattern, directory, replace, result, error);
}

bool IdeSessionImpl::SearchCode(const String& query, String& result, String& error) {
    return core_ide.SearchCode(query, result, error);
}

bool IdeSessionImpl::GetConsoleOutput(String& output, String& error) {
    return core_ide.GetConsoleOutput(output, error);
}

bool IdeSessionImpl::GetErrorsOutput(String& output, String& error) {
    return core_ide.GetErrorsOutput(output, error);
}

// Editor-specific operations
bool IdeSessionImpl::EditorInsert(const String& path, int pos, const String& text, String& error) {
    return core_ide.EditorInsert(path, pos, text, error);
}

bool IdeSessionImpl::EditorErase(const String& path, int pos, int count, String& error) {
    return core_ide.EditorErase(path, pos, count, error);
}

bool IdeSessionImpl::EditorReplace(const String& path, int pos, int count, const String& replacement, String& error) {
    return core_ide.EditorReplace(path, pos, count, replacement, error);
}

bool IdeSessionImpl::EditorGotoLine(const String& path, int line, int& out_pos, String& error) {
    return core_ide.EditorGotoLine(path, line, out_pos, error);
}

bool IdeSessionImpl::EditorFindFirst(const String& path, const String& pattern, int start_pos,
                                     bool case_sensitive, int& out_pos, String& error) {
    return core_ide.EditorFindFirst(path, pattern, start_pos, case_sensitive, out_pos, error);
}

bool IdeSessionImpl::EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                                      bool case_sensitive, int& out_count, String& error) {
    return core_ide.EditorReplaceAll(path, pattern, replacement, case_sensitive, out_count, error);
}

bool IdeSessionImpl::EditorUndo(const String& path, String& error) {
    return core_ide.EditorUndo(path, error);
}

bool IdeSessionImpl::EditorRedo(const String& path, String& error) {
    return core_ide.EditorRedo(path, error);
}

bool IdeSessionImpl::FindDefinition(const String& symbol, String& file, int& line, String& error) {
    return core_ide.FindSymbolDefinition(symbol, file, line, error);
}

bool IdeSessionImpl::FindUsages(const String& symbol, Vector<String>& locs, String& error) {
    return core_ide.FindSymbolUsages(symbol, locs, error);
}

// Graph operations
bool IdeSessionImpl::GetBuildOrder(Vector<String>& out_order, String& error) {
    return core_ide.GetBuildOrder(out_order, error);
}

bool IdeSessionImpl::FindCycles(Vector<Vector<String>>& out_cycles, String& error) {
    return core_ide.GetCycles(out_cycles, error);
}

bool IdeSessionImpl::AffectedPackages(const String& filepath,
                                      Vector<String>& out_packages,
                                      String& error) {
    return core_ide.GetAffectedPackages(filepath, out_packages, error);
}

// Refactoring operations
bool IdeSessionImpl::RenameSymbol(const String& old_name,
                                  const String& new_name,
                                  String& error) {
    return core_ide.RenameSymbol(old_name, new_name, error);
}

bool IdeSessionImpl::RemoveDeadIncludes(const String& path,
                                        String& error) {
    return core_ide.RemoveDeadIncludes(path, error);
}

bool IdeSessionImpl::CanonicalizeIncludes(const String& path,
                                          String& error) {
    return core_ide.CanonicalizeIncludes(path, error);
}

// Telemetry & Analytics v1 implementations
Value IdeSessionImpl::GetWorkspaceStats(String& error) {
    return core_ide.GetWorkspaceStats();
}

Value IdeSessionImpl::GetPackageStats(const String& pkg, String& error) {
    return core_ide.GetPackageStats(pkg);
}

Value IdeSessionImpl::GetFileComplexity(const String& path, String& error) {
    return core_ide.GetFileComplexity(path);
}

Value IdeSessionImpl::GetGraphStats(String& error) {
    return core_ide.GetGraphStats();
}

Value IdeSessionImpl::GetEditHistory(String& error) {
    return core_ide.GetEditHistory();
}

// Optimization Loop v1 implementations
Value IdeSessionImpl::OptimizePackage(
    const String& package,
    int max_iterations,
    double converge_threshold,
    bool stop_on_worse,
    bool stop_on_converge,
    String& error
) {
    CoreOptimize::LoopConfig config;
    config.max_iterations = max_iterations;
    config.converge_threshold = converge_threshold;
    config.stop_on_worse = stop_on_worse;
    config.stop_on_converge = stop_on_converge;

    // Call the CoreIde's RunOptimizationLoop method
    return core_ide.RunOptimizationLoop(package, config, error);
}

// AI Supervisor Layer v1 implementation
Value IdeSessionImpl::GetOptimizationPlan(const String& package, String& error) {
    return core_ide.GenerateOptimizationPlan(package, error);
}

Value IdeSessionImpl::GetWorkspacePlan(String& error) {
    return core_ide.GenerateWorkspacePlan(error);
}

// Dynamic Strategy Engine implementations
bool IdeSessionImpl::SetActiveStrategy(const String& name, String& error) {
    return core_ide.SetActiveStrategy(name, error);
}

Value IdeSessionImpl::GetActiveStrategy(String& error) {
    const StrategyProfile* profile = core_ide.GetActiveStrategy();
    if (!profile) {
        error = "No active strategy found";
        return Value();
    }

    ValueMap result;
    result.Set("name", profile->name);
    result.Set("description", profile->description);
    result.Set("weights", profile->weights);
    result.Set("thresholds", profile->thresholds);

    return result;
}

Value IdeSessionImpl::GetStrategy(const String& name, String& error) {
    // Find the strategy in the registry by name
    const StrategyProfile* profile = core_ide.strategy_registry.Find(name);
    if (!profile) {
        error = "Strategy not found: " + name;
        return Value();
    }

    ValueMap result;
    result.Set("name", profile->name);
    result.Set("description", profile->description);
    result.Set("weights", profile->weights);
    result.Set("thresholds", profile->thresholds);

    return result;
}

Value IdeSessionImpl::ListStrategies(String& error) {
    ValueArray result;

    // Get all strategies from the registry
    const Vector<StrategyProfile>& profiles = core_ide.GetAllStrategies();

    for (const auto& profile : profiles) {
        ValueMap strategy_info;
        strategy_info.Set("name", profile.name);
        strategy_info.Set("description", profile.description);
        result.Add(strategy_info);
    }

    return result;
}

One<IdeSession> CreateIdeSession() {
    return new IdeSessionImpl();
}

}