#ifndef CMD_IDESESSION_H
#define CMD_IDESESSION_H

#include "cli.h"
#include <Core/Core.h>

namespace Upp {

class IdeSession {
public:
    virtual ~IdeSession() {}

    // File operations
    virtual bool OpenFile(const String& path, String& error) = 0;
    virtual bool SaveFile(const String& path, String& error) = 0;

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

    // Methods to retrieve console and error output
    virtual bool GetConsoleOutput(String& output, String& error) = 0;
    virtual bool GetErrorsOutput(String& output, String& error) = 0;
};

Ptr<IdeSession> CreateIdeSession();

}

#endif