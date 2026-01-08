#ifndef _clicore_MetaOrchestrator_h_
#define _clicore_MetaOrchestrator_h_

#include <Core/Core.h>
#include "LifecycleModel.h"

NAMESPACE_UPP

struct ProjectSummary : Moveable<ProjectSummary> {
    String name;
    String path;
    double stability;          // 0-1, stability score for the project
    String lifecycle_phase;    // Current lifecycle phase
    double entropy;            // 0-1, entropy measure
    int size_loc;             // Size in lines of code
    int packages;             // Number of packages in project
};

struct CrossWorkspacePlan : Moveable<CrossWorkspacePlan> {
    String strategy_name;
    ValueArray proposals;      // Array of proposals for different projects
    ValueMap global_metrics;   // Metrics across all projects
};

class MetaOrchestrator {
public:
    MetaOrchestrator();

    // Add a workspace to the orchestrator
    void AddWorkspace(const String& path);

    // Get summaries for all managed workspaces
    Vector<ProjectSummary> Summaries() const;

    // Build a global roadmap across all workspaces
    CrossWorkspacePlan BuildGlobalRoadmap(const String& strategy) const;

    // Get all registered workspace paths
    Vector<String> GetWorkspacePaths() const;

private:
    Vector<String> workspace_paths;
    Vector<ProjectSummary> summaries;
};

END_UPP_NAMESPACE

#endif