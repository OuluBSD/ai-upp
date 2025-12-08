#include "clicore.h"

using namespace Upp;

CoreIde::CoreIde() {
    // Initialize internal state
}

CoreIde::~CoreIde() {
    // Clean up resources
}

// File operations
bool CoreIde::OpenFile(const String& path, String& error) {
    String content;
    return fileOps.OpenFile(path, content, error);
}

bool CoreIde::SaveFile(const String& path, String& error) {
    // For save, we need to get current content somehow
    // In a real implementation, we would save the current in-memory content of the file
    // For now, if the file exists, we'll save it with the same content to just validate we can write
    String content;
    if (FileExists(path)) {
        content = LoadFile(path);  // Load current content if file exists
        if (content.IsEmpty() && !GetFileTime(path)) {
            error = "File access error: " + path;
            return false;
        }
    } else {
        // If file doesn't exist, we'll save it with empty content (create new file)
        content = "";
    }

    return fileOps.SaveFile(path, content, error);
}

// Workspace management
bool CoreIde::SetWorkspaceRoot(const String& root, String& error) {
    if (!workspace.SetWorkspaceRoot(root, error)) {
        return false;
    }

    workspace_root = root;
    return true;
}

const String& CoreIde::GetWorkspaceRoot() const {
    return workspace_root;
}

// Project/package operations
bool CoreIde::SetMainPackage(const String& package, String& error) {
    // First ensure workspace root is set
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use SetWorkspaceRoot first.";
        return false;
    }

    // Use the workspace helper to set the main package
    return workspace.SetMainPackage(package, error);
}

bool CoreIde::GetMainPackage(CoreWorkspace::Package& out, String& error) const {
    return workspace.GetMainPackage(out, error);
}

// Build operations
bool CoreIde::BuildProject(const String& project, const String& config, String& log, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    // If no project specified, use main package
    if (project.IsEmpty() && !workspace.GetMainPackage().IsEmpty()) {
        CoreWorkspace::Package main_pkg;
        if (!workspace.GetMainPackage(main_pkg, error)) {
            return false;
        }
        return build.BuildProject(main_pkg, config, log, error);
    } else if (!project.IsEmpty()) {
        // If project is specified, load it and build
        CoreWorkspace::Package pkg;
        if (!workspace.LoadPackage(project, error)) {
            return false;
        }

        // Try to get the loaded package
        const CoreWorkspace::Package* loaded_pkg = workspace.GetPackage(project);
        if (!loaded_pkg) {
            error = "Could not load package: " + project;
            return false;
        }

        return build.BuildProject(*loaded_pkg, config, log, error);
    } else {
        error = "No project specified and no main package set.";
        return false;
    }
}

bool CoreIde::CleanProject(const String& project, String& log, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    // If no project specified, use main package
    if (project.IsEmpty() && !workspace.GetMainPackage().IsEmpty()) {
        CoreWorkspace::Package main_pkg;
        if (!workspace.GetMainPackage(main_pkg, error)) {
            return false;
        }
        return build.CleanProject(main_pkg, log, error);
    } else if (!project.IsEmpty()) {
        // If project is specified, load it and clean
        CoreWorkspace::Package pkg;
        if (!workspace.LoadPackage(project, error)) {
            return false;
        }

        // Try to get the loaded package
        const CoreWorkspace::Package* loaded_pkg = workspace.GetPackage(project);
        if (!loaded_pkg) {
            error = "Could not load package: " + project;
            return false;
        }

        return build.CleanProject(*loaded_pkg, log, error);
    } else {
        error = "No project specified and no main package set.";
        return false;
    }
}

// Navigation
bool CoreIde::GotoLine(const String& file, int line, String& error) {
    // TODO: Implement line navigation logic
    // For now, just verify the file exists and the line number is valid
    if (!FileExists(file)) {
        error = "File does not exist: " + file;
        return false;
    }

    if (line <= 0) {
        error = "Invalid line number: " + AsString(line);
        return false;
    }

    // In the future, this would navigate to the specific line in an internal editor
    return true;
}

// Search / code search
bool CoreIde::FindInFiles(const String& pattern, const String& directory, bool replace, String& result, String& error) {
    // If no directory is specified, use workspace root or main package directory
    String search_dir = directory;
    if (search_dir.IsEmpty()) {
        if (!workspace.GetMainPackage().IsEmpty()) {
            CoreWorkspace::Package main_pkg;
            if (!workspace.GetMainPackage(main_pkg, error)) {
                // If we can't get the main package, fall back to workspace root
                search_dir = workspace.GetWorkspaceRoot();
            } else {
                search_dir = main_pkg.path;
            }
        } else if (!workspace.GetWorkspaceRoot().IsEmpty()) {
            search_dir = workspace.GetWorkspaceRoot();
        } else {
            error = "No search directory specified and workspace context not established.";
            return false;
        }
    }

    // Delegate to the search helper
    Vector<String> results;
    bool success = search.FindInFiles(pattern, search_dir, replace, "", results, error);

    // Combine results into a single string
    for (const String& res : results) {
        result += res + "\n";
    }

    return success;
}

bool CoreIde::SearchCode(const String& query, String& result, String& error) {
    // First verify workspace context
    if (workspace_root.IsEmpty()) {
        error = "Workspace root not set. Use --workspace-root PATH.";
        return false;
    }

    if (workspace.GetMainPackage().IsEmpty()) {
        error = "No main package set. Use SetMainPackage first.";
        return false;
    }

    // Get the main package and search in its files
    CoreWorkspace::Package main_pkg;
    if (!workspace.GetMainPackage(main_pkg, error)) {
        return false;
    }

    // Use the workspace-aware search method
    Vector<String> results;
    bool success = search.SearchCodeInPackage(query, main_pkg, results, error);

    // Combine results into a single string
    for (const String& res : results) {
        result += res + "\n";
    }

    return success;
}

// Output
bool CoreIde::GetConsoleOutput(String& out, String& error) {
    // Delegate to the console helper
    out = console.GetConsoleOutput();
    return true; // This operation should always succeed
}

bool CoreIde::GetErrorsOutput(String& out, String& error) {
    // Delegate to the console helper
    out = console.GetErrorsOutput();
    return true; // This operation should always succeed
}