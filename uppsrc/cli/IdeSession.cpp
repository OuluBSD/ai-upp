#include "IdeSession.h"
#include "IdeHost.h"
#include <Core/Core.h>

namespace Upp {

class IdeSessionImpl : public IdeSession {
public:
    IdeSessionImpl();
    virtual ~IdeSessionImpl();

    // File operations
    virtual bool OpenFile(const String& path, String& error) override;
    virtual bool SaveFile(const String& path, String& error) override;

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

private:
    bool initialized;
    String last_console_output;
    String last_errors;
};

IdeSessionImpl::IdeSessionImpl() : initialized(false) {
    // Initialize the session - try to initialize the IdeHost
    String error;
    if (GetGlobalIdeHost().Init(error)) {
        initialized = true;
    }
    // Note: We continue even if initialization fails, as the caller can check error states
}

IdeSessionImpl::~IdeSessionImpl() {
    // Cleanup session resources
}

bool IdeSessionImpl::OpenFile(const String& path, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (path.IsEmpty()) {
        error = "Path is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();
        ide.EditFile(path);
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while opening file: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while opening file";
        return false;
    }
}

bool IdeSessionImpl::SaveFile(const String& path, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (path.IsEmpty()) {
        error = "Path is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();
        // First, make the file the current file if it's not already
        if (path != ide.editfile) {
            ide.EditFile(path);
        }
        // Then save it
        ide.SaveFile(true); // always save
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while saving file: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while saving file";
        return false;
    }
}

bool IdeSessionImpl::SetMainPackage(const String& package, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (package.IsEmpty()) {
        error = "Package name is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();
        ide.SetMain(package);
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while setting main package: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while setting main package";
        return false;
    }
}

bool IdeSessionImpl::BuildProject(const String& project, const String& config, String& log, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (project.IsEmpty()) {
        error = "Project name is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();

        // Set the main package to build
        String old_main = ide.GetMain();
        ide.SetMain(project);

        // Try to set the configuration if possible
        // For now, we'll just use the config parameter as a hint
        // In a real implementation, we'd need to set the build mode

        // Perform the build
        ide.BeginBuilding(true); // clear console and begin building

        const Workspace& wspc = ide.IdeWorkspace();
        int pi = ide.GetPackageIndex();
        if (pi >= 0 && pi < wspc.GetCount()) {
            Vector<String> linkfile, immfile;
            String linkopt;
            bool ok = BuildPackage(wspc, pi, 0, 1, ide.pocfg, Null, linkfile, immfile, linkopt);
            ide.EndBuilding(ok);

            // Capture console output and errors
            Vector<String> errors = ide.PickErrors();
            for(const String& e : errors) {
                log += e + "\n";
            }

            if(ok) {
                log += "Build completed successfully for project '" + project + "'";
            } else {
                log += "Build failed for project '" + project + "'";
            }
        } else {
            log = "Project not found: " + project;
        }

        // Restore the original main package if needed
        if (old_main != project) {
            ide.SetMain(old_main);
        }

        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while building project: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while building project";
        return false;
    }
}

bool IdeSessionImpl::CleanProject(const String& project, String& log, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (project.IsEmpty()) {
        error = "Project name is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();

        // Set the main package to clean
        String old_main = ide.GetMain();
        ide.SetMain(project);

        // Perform the clean
        const Workspace& wspc = ide.IdeWorkspace();
        int pi = ide.GetPackageIndex();

        if (pi >= 0 && pi < wspc.GetCount()) {
            ide.console.Clear(); // Clear console before cleaning
            CleanPackage(wspc, pi);
            log = "Clean completed for project '" + project + "'";
        } else {
            log = "Project not found: " + project;
        }

        // Restore the original main package if needed
        if (old_main != project) {
            ide.SetMain(old_main);
        }

        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while cleaning project: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while cleaning project";
        return false;
    }
}

bool IdeSessionImpl::GotoLine(const String& file, int line, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (line <= 0) {
        error = "Line number must be positive, got: " + IntStr(line);
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();

        if (!file.IsEmpty()) {
            // If file is specified, go to that file and line
            ide.GotoPos(file, line - 1); // Convert to 0-based line index
        } else {
            // If no file specified, go to line in the current file
            ide.GotoPos(String(), line - 1);
        }
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while going to line: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while going to line";
        return false;
    }
}

bool IdeSessionImpl::ShowConsole(String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    try {
        // In headless mode, we can't "show" the console panel,
        // but we can access its content through the IDE instance
        Ide& ide = GetGlobalIdeHost().GetIde();
        // For now, this method just ensures we can access console functionality
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while accessing console: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while accessing console";
        return false;
    }
}

bool IdeSessionImpl::ShowErrors(String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    try {
        // In headless mode, we can't "show" the errors panel,
        // but we can access error data through the IDE instance
        Ide& ide = GetGlobalIdeHost().GetIde();
        // For now, this method just ensures we can access error functionality
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while accessing errors: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while accessing errors";
        return false;
    }
}

bool IdeSessionImpl::FindInFiles(const String& pattern, const String& directory, String& result, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (pattern.IsEmpty()) {
        error = "Search pattern is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();

        // For headless mode, we'll implement a text search in files
        result.Clear();

        // Get the directory to search in
        String searchDir = directory.IsEmpty() ? "." : directory;

        // Create a list of files to search
        Vector<String> files;
        FindFile ff(AppendFileName(searchDir, "*.*"));
        while(ff) {
            if (ff.IsFile() && IsTextFile(ff.GetPath())) {
                files.Add(ff.GetPath());
            }
            ff.Next();
        }

        // Search for the pattern in each file
        for(const String& file : files) {
            // Load the file content and search for the pattern
            String content = LoadFile(file);
            int pos = 0;
            int lineNum = 1;
            const char* start = content;
            const char* ptr = start;

            while ((pos = content.Find(pattern, pos)) >= 0) {
                // Find the line number where the pattern occurs
                lineNum = 1;
                ptr = start;
                while (ptr < start + pos) {
                    if (*ptr == '\n') lineNum++;
                    ptr++;
                }

                result += Format("%s:%d: %s\n", file, lineNum, String(content.Mid(pos).Left(50) + "...").ToStd());
                pos++;  // Move to next position to continue searching
            }
        }

        if (result.IsEmpty()) {
            result = "Pattern '" + pattern + "' not found in directory: " + searchDir;
        }

        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while finding in files: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while finding in files";
        return false;
    }
}

bool IdeSessionImpl::SearchCode(const String& query, String& result, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    if (query.IsEmpty()) {
        error = "Search query is empty";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();

        // Trigger the indexer first to ensure the code database is up to date
        ide.TriggerIndexer0(); // This is the proper method to trigger indexer

        // For headless mode, we'll simulate code search by using the annotation system
        // In a real implementation, we'd integrate with the assist/navigator system
        result = "SearchCode: Found references for '" + query + "' (simulated)";

        // We can't fully implement the code search without UI components
        // But we can at least trigger the proper indexer and potentially
        // access the code index (though it's tied to the UI in current implementation)

        // For now, return a message indicating we acknowledge the operation
        result = "Triggered code search for: " + query + " (indexing and search results would be available in UI mode)";

        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while searching code: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while searching code";
        return false;
    }
}

bool IdeSessionImpl::GetConsoleOutput(String& output, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();
        // Access the console content - we'll try to get its text content
        // Since Console is a CodeEditor, we can access its content

        // This is approximate - in practice, we might need to access the console differently
        // depending on how TheIDE stores console output
        output = "Console output would be retrieved here in a full implementation";

        // In a real implementation, we might have to access the console's internal storage
        // which may not be directly accessible in headless mode
        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while accessing console output: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while accessing console output";
        return false;
    }
}

bool IdeSessionImpl::GetErrorsOutput(String& output, String& error) {
    if (!initialized) {
        error = "IdeSession not properly initialized";
        return false;
    }

    try {
        Ide& ide = GetGlobalIdeHost().GetIde();
        // Get errors from the IDE instance
        Vector<String> errors = ide.PickErrors();

        output.Clear();
        for(const String& e : errors) {
            output += e + "\n";
        }

        if (output.IsEmpty()) {
            output = "No errors found";
        }

        return true;
    }
    catch (const Exc& e) {
        error = String("Exception while accessing errors output: ") + e;
        return false;
    }
    catch (...) {
        error = "Unknown error while accessing errors output";
        return false;
    }
}

Ptr<IdeSession> CreateIdeSession() {
    return new IdeSessionImpl();
}

}