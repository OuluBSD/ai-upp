#include "clicore.h"

using namespace Upp;

CoreIde::CoreIde() {
    // Initialize internal state
    current_editor_index = -1;
}

CoreIde::~CoreIde() {
    // Clean up resources
}

// File operations
bool CoreIde::OpenFile(const String& path, String& error) {
    return OpenEditor(path, error) != nullptr;
}

bool CoreIde::SaveFile(const String& path, String& error) {
    if (path.IsEmpty()) {
        // Save the current editor if no path is specified
        if (current_editor_index >= 0 && current_editor_index < editors.GetCount()) {
            return editors[current_editor_index].SaveFile(error);
        } else {
            error = "No file path specified and no current editor available";
            return false;
        }
    }

    // Find the editor for the specified path
    int index = editor_paths.Find(path);
    if (index >= 0) {
        // Editor exists, save it
        return editors[index].SaveFile(error);
    } else {
        // Editor doesn't exist, so save the original file directly
        String content;
        if (FileExists(path)) {
            content = LoadFile(path);
        }
        return fileOps.SaveFile(path, content, error);
    }
}

// Editor operations
CoreEditor* CoreIde::FindEditorForPath(const String& path) {
    int index = editor_paths.Find(path);
    if (index >= 0 && index < editors.GetCount()) {
        return &editors[index];
    }
    return nullptr;
}

CoreEditor* CoreIde::GetCurrentEditor() {
    if (current_editor_index >= 0 && current_editor_index < editors.GetCount()) {
        return &editors[current_editor_index];
    }
    return nullptr;
}

CoreEditor* CoreIde::OpenEditor(const String& path, String& error) {
    // Check if editor for this path already exists
    int index = editor_paths.Find(path);
    if (index >= 0) {
        // Editor already exists, set it as current and return it
        current_editor_index = index;
        return &editors[index];
    }

    // Editor doesn't exist, create a new one
    CoreEditor new_editor;
    if (!new_editor.LoadFile(path, error)) {
        return nullptr;  // Error already set by LoadFile
    }

    // Add the new editor
    int new_index = editors.GetCount();
    editors.Add(new_editor);
    editor_paths.Add(path);

    // Set as current editor
    current_editor_index = new_index;

    return &editors[new_index];
}

bool CoreIde::CloseFile(const String& path, String& error) {
    int index = editor_paths.Find(path);
    if (index < 0) {
        error = "File not open: " + path;
        return false;
    }

    // Remove the editor
    editors.Remove(index);
    editor_paths.Remove(index);

    // Update current_editor_index if needed
    if (current_editor_index == index) {
        current_editor_index = -1;  // No current editor
    } else if (current_editor_index > index) {
        current_editor_index--;  // Adjust index after removal
    }

    return true;
}

// Editor-specific operations
bool CoreIde::EditorInsert(const String& path, int pos, const String& text, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Insert(pos, text);
}

bool CoreIde::EditorErase(const String& path, int pos, int count, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Erase(pos, count);
}

bool CoreIde::EditorReplace(const String& path, int pos, int count, const String& replacement, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Replace(pos, count, replacement);
}

bool CoreIde::EditorGotoLine(const String& path, int line, int& out_pos, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->GotoLine(line, out_pos);
}

bool CoreIde::EditorFindFirst(const String& path, const String& pattern, int start_pos,
                              bool case_sensitive, int& out_pos, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->FindFirst(pattern, start_pos, case_sensitive, out_pos);
}

bool CoreIde::EditorReplaceAll(const String& path, const String& pattern, const String& replacement,
                               bool case_sensitive, int& out_count, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->ReplaceAll(pattern, replacement, case_sensitive, out_count);
}

bool CoreIde::EditorUndo(const String& path, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Undo();
}

bool CoreIde::EditorRedo(const String& path, String& error) {
    CoreEditor* editor = FindEditorForPath(path);
    if (!editor) {
        // Try to open the file in editor
        editor = OpenEditor(path, error);
        if (!editor) {
            return false;
        }
    }

    return editor->Redo();
}

// Workspace management
bool CoreIde::SetWorkspaceRoot(const String& root, String& error) {
    if (!workspace.SetWorkspaceRoot(root, error)) {
        return false;
    }

    workspace_root = root;

    // Rebuild graph after workspace root changes
    String graph_error;
    if (!RebuildGraph(graph_error)) {
        // Log the error but don't fail the SetWorkspaceRoot operation
        console.AddToConsole("Warning: Failed to rebuild dependency graph: " + graph_error);
    }

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
    bool result = workspace.SetMainPackage(package, error);

    // Rebuild graph after setting main package
    if (result) {
        String graph_error;
        if (!RebuildGraph(graph_error)) {
            // Log the error but don't fail the SetMainPackage operation
            console.AddToConsole("Warning: Failed to rebuild dependency graph: " + graph_error);
        }
    }

    return result;
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

// Symbol assistance
bool CoreIde::IndexWorkspace(String& error) {
    return assist.IndexWorkspace(workspace, error);
}

bool CoreIde::FindSymbolDefinition(const String& symbol, String& file, int& line, String& error) {
    return assist.FindDefinition(symbol, file, line);
}

bool CoreIde::FindSymbolUsages(const String& symbol, Vector<String>& locs, String& error) {
    return assist.FindUsages(symbol, locs);
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

// Graph operations
bool CoreIde::RebuildGraph(String& error) {
    return graph.BuildPackageGraph(workspace, error);
}

bool CoreIde::GetBuildOrder(Vector<String>& out_order, String& error) {
    return graph.TopologicalSort(out_order, error);
}

bool CoreIde::GetCycles(Vector<Vector<String>>& out_cycles, String& error) {
    return graph.DetectCycles(out_cycles);
}

bool CoreIde::GetAffectedPackages(const String& filepath, Vector<String>& out_packages, String& error) {
    return graph.AffectedPackagesByFile(filepath, workspace, out_packages);
}