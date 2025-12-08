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
};

One<IdeSession> CreateIdeSession();

}

#endif