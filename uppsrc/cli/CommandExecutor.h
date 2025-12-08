#ifndef CMD_COMMANDEXECUTOR_H
#define CMD_COMMANDEXECUTOR_H

#include <Core/Core.h>
#include "Command.h"
#include "CommandRegistry.h"
#include "IdeSession.h"

namespace Upp {

class CommandExecutor {
public:
    CommandExecutor(const CommandRegistry& registry, One<IdeSession> session);

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

    // Handler functions for editor commands (using IdeSession)
    InvocationResult HandleInsertText(const VectorMap<String, String>& args);
    InvocationResult HandleEraseRange(const VectorMap<String, String>& args);
    InvocationResult HandleReplaceAll(const VectorMap<String, String>& args);
    InvocationResult HandleUndo(const VectorMap<String, String>& args);
    InvocationResult HandleRedo(const VectorMap<String, String>& args);

    // Handler functions for stub commands (not yet integrated)
    InvocationResult HandleFindInFiles(const VectorMap<String, String>& args);
    InvocationResult HandleSearchCode(const VectorMap<String, String>& args);
    InvocationResult HandleShowConsole(const VectorMap<String, String>& args);
    InvocationResult HandleShowErrors(const VectorMap<String, String>& args);

    // Handler functions for symbol analysis commands
    InvocationResult HandleFindDefinition(const VectorMap<String, String>& args);
    InvocationResult HandleFindUsages(const VectorMap<String, String>& args);

    // Handler functions for graph analysis commands
    InvocationResult HandleGetBuildOrder(const VectorMap<String, String>& args);
    InvocationResult HandleDetectCycles(const VectorMap<String, String>& args);
    InvocationResult HandleAffectedPackages(const VectorMap<String, String>& args);

    const CommandRegistry& registry;
    One<IdeSession> session;
};

}

#endif