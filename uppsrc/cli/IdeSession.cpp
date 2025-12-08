#include "IdeSession.h"
#include <clicore/CoreIde.h>
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

    // Methods to retrieve console and error output
    virtual bool GetConsoleOutput(String& output, String& error) override;
    virtual bool GetErrorsOutput(String& output, String& error) override;

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

One<IdeSession> CreateIdeSession() {
    return new IdeSessionImpl();
}

}