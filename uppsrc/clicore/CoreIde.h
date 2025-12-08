#ifndef _clicore_CoreIde_h_
#define _clicore_CoreIde_h_

#include <Core/Core.h>
#include "CoreWorkspace.h"
#include "CoreBuild.h"
#include "CoreSearch.h"
#include "CoreConsole.h"
#include "CoreFileOps.h"

using namespace Upp;

class CoreIde {
public:
    CoreIde();
    ~CoreIde();

    // File operations
    bool OpenFile(const String& path, String& error);
    bool SaveFile(const String& path, String& error);

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

    // Output
    bool GetConsoleOutput(String& out, String& error);
    bool GetErrorsOutput(String& out, String& error);

private:
    // Internal state: workspace, packages, logs, etc.
    CoreWorkspace workspace;
    CoreBuild build;
    CoreSearch search;
    CoreConsole console;
    CoreFileOps fileOps;
    String workspace_root;
};

#endif