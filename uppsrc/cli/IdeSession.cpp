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

    // Semantic Analysis v1
    virtual Value GetSemanticEntities(String& error) override;
    virtual Value GetSemanticClusters(String& error) override;
    virtual Value SearchSemanticEntities(const String& pattern, String& error) override;

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

// Semantic Analysis v1 implementations
Value IdeSessionImpl::GetSemanticEntities(String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic entities";
        return Value();
    }

    // Get the entities from the semantic analyzer
    const auto& entities = core_ide.semantic.GetEntities();

    // Convert entities to ValueArray
    ValueArray result;
    for (const auto& entity : entities) {
        ValueMap entity_map;
        entity_map.Set("name", entity.name);
        entity_map.Set("kind", entity.kind);
        entity_map.Set("file", entity.file);
        entity_map.Set("line", entity.line);

        // Convert relations to ValueMap
        ValueMap relations_map;
        for (const auto& key : entity.relations.GetKeys()) {
            relations_map.Set(key, entity.relations.Get(key));
        }
        entity_map.Set("relations", relations_map);

        // Convert attributes to ValueMap
        ValueMap attributes_map;
        for (const auto& key : entity.attributes.GetKeys()) {
            attributes_map.Set(key, entity.attributes.Get(key));
        }
        entity_map.Set("attributes", attributes_map);

        result.Add(entity_map);
    }

    return result;
}

Value IdeSessionImpl::GetSemanticClusters(String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic clusters";
        return Value();
    }

    // Get the clusters from the semantic analyzer
    const auto& clusters = core_ide.semantic.GetClusters();

    // Convert clusters to ValueArray
    ValueArray result;
    for (const auto& cluster : clusters) {
        ValueMap cluster_map;
        cluster_map.Set("name", cluster.name);

        // Convert entities to ValueArray
        ValueArray entities_array;
        for (const auto& entity : cluster.entities) {
            entities_array.Add(entity);
        }
        cluster_map.Set("entities", entities_array);

        // Convert metrics to ValueMap
        ValueMap metrics_map;
        for (const auto& key : cluster.metrics.GetKeys()) {
            metrics_map.Set(key, cluster.metrics.Get(key));
        }
        cluster_map.Set("metrics", metrics_map);

        result.Add(cluster_map);
    }

    return result;
}

Value IdeSessionImpl::SearchSemanticEntities(const String& pattern, String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic entities";
        return Value();
    }

    // Find entities matching the pattern
    auto entities = core_ide.semantic.FindEntitiesByName(pattern);

    // Convert entities to ValueArray
    ValueArray result;
    for (const auto& entity : entities) {
        ValueMap entity_map;
        entity_map.Set("name", entity.name);
        entity_map.Set("kind", entity.kind);
        entity_map.Set("file", entity.file);
        entity_map.Set("line", entity.line);

        // Convert relations to ValueMap
        ValueMap relations_map;
        for (const auto& key : entity.relations.GetKeys()) {
            relations_map.Set(key, entity.relations.Get(key));
        }
        entity_map.Set("relations", relations_map);

        // Convert attributes to ValueMap
        ValueMap attributes_map;
        for (const auto& key : entity.attributes.GetKeys()) {
            attributes_map.Set(key, entity.attributes.Get(key));
        }
        entity_map.Set("attributes", attributes_map);

        result.Add(entity_map);
    }

    return result;
}

// Semantic Analysis v2 - NEW: Inference layer implementations
Value IdeSessionImpl::GetSemanticSubsystems(String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic subsystems";
        return Value();
    }

    // Get the subsystems from the semantic analyzer
    const auto& subsystems = core_ide.semantic.GetSubsystems();

    // Convert subsystems to ValueArray
    ValueArray result;
    for (const auto& subsystem : subsystems) {
        ValueMap subsystem_map;
        subsystem_map.Set("name", subsystem.name);

        // Convert entities to ValueArray
        ValueArray entities_array;
        for (const auto& entity : subsystem.entities) {
            entities_array.Add(entity);
        }
        subsystem_map.Set("entities", entities_array);

        // Convert metrics to ValueMap
        ValueMap metrics_map;
        for (const auto& key : subsystem.metrics.GetKeys()) {
            metrics_map.Set(key, subsystem.metrics.Get(key));
        }
        subsystem_map.Set("metrics", metrics_map);

        result.Add(subsystem_map);
    }

    return result;
}

Value IdeSessionImpl::GetSemanticEntity(const String& name, String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic entity";
        return Value();
    }

    // Find the specific entity
    auto entities = core_ide.semantic.FindEntitiesByName(name);
    if (entities.GetCount() == 0) {
        error = "Entity not found: " + name;
        return Value();
    }

    // Use the first matching entity
    const auto& entity = entities[0];

    // Convert entity to ValueMap
    ValueMap entity_map;
    entity_map.Set("name", entity.name);
    entity_map.Set("kind", entity.kind);
    entity_map.Set("file", entity.file);
    entity_map.Set("line", entity.line);

    // Convert relations to ValueMap
    ValueMap relations_map;
    for (const auto& key : entity.relations.GetKeys()) {
        relations_map.Set(key, entity.relations.Get(key));
    }
    entity_map.Set("relations", relations_map);

    // Convert attributes to ValueMap
    ValueMap attributes_map;
    for (const auto& key : entity.attributes.GetKeys()) {
        attributes_map.Set(key, entity.attributes.Get(key));
    }
    entity_map.Set("attributes", attributes_map);

    // Add the new inferred relation fields in the relations map
    // Convert co_occurs_with to ValueArray
    ValueArray co_occurs_with_array;
    for (const auto& co_entity : entity.co_occurs_with) {
        co_occurs_with_array.Add(co_entity);
    }
    relations_map.Set("co_occurs_with", co_occurs_with_array);

    // Convert conceptually_related to ValueArray
    ValueArray conceptually_related_array;
    for (const auto& related_entity : entity.conceptually_related) {
        conceptually_related_array.Add(related_entity);
    }
    relations_map.Set("conceptually_related", conceptually_related_array);

    // Add layer_dependency to ValueMap
    entity_map.Set("layer_dependency", entity.layer_dependency);

    // Add role to ValueMap
    entity_map.Set("role", entity.role);

    // Update the relations map in the entity map
    entity_map.Set("relations", relations_map);

    return entity_map;
}

Value IdeSessionImpl::GetSemanticRoles(String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic roles";
        return Value();
    }

    // Create a mapping of entity names to their roles
    ValueMap roles_map;

    const auto& entities = core_ide.semantic.GetEntities();
    for (const auto& entity : entities) {
        roles_map.Set(entity.name, entity.role);
    }

    return roles_map;
}

Value IdeSessionImpl::GetSemanticLayers(String& error) {
    // First ensure semantic analysis has been run
    if (!core_ide.AnalyzeSemantics(error)) {
        if (error.IsEmpty()) error = "Failed to analyze workspace for semantic layers";
        return Value();
    }

    // Create a mapping of entity names to their layer dependencies
    ValueMap layers_map;

    const auto& entities = core_ide.semantic.GetEntities();
    for (const auto& entity : entities) {
        layers_map.Set(entity.name, entity.layer_dependency);
    }

    return layers_map;
}

One<IdeSession> CreateIdeSession() {
    return new IdeSessionImpl();
}

}