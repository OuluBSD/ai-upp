#ifndef CMD_COMMANDEXECUTOR_H
#define CMD_COMMANDEXECUTOR_H

#include "cli.h"
#include "IdeSession.h"

namespace Upp {

class CommandExecutor {
public:
    CommandExecutor(const CommandRegistry& registry);

    InvocationResult Invoke(const String& name,
                           const VectorMap<String, String>& args);

private:
    // Handler functions for integrated commands (using IdeSession)
    InvocationResult HandleOpenFile(const VectorMap<String, String>& args);
    InvocationResult HandleSaveFile(const VectorMap<String, String>& args);
    InvocationResult HandleSetMainPackage(const VectorMap<String, String>& args);
    InvocationResult HandleBuildProject(const VectorMap<String, String>& args);
    InvocationResult HandleCleanProject(const VectorMap<String, String>& args);
    InvocationResult HandleGotoLine(const VectorMap<String, String>& args);

    // Handler functions for stub commands (not yet integrated)
    InvocationResult HandleFindInFiles(const VectorMap<String, String>& args);
    InvocationResult HandleSearchCode(const VectorMap<String, String>& args);
    InvocationResult HandleShowConsole(const VectorMap<String, String>& args);
    InvocationResult HandleShowErrors(const VectorMap<String, String>& args);

    const CommandRegistry& registry;
    Ptr<IdeSession> session;
};

}

#endif