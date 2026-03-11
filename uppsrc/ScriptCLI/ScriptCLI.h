#ifndef _ScriptCLI_ScriptCLI_h_
#define _ScriptCLI_ScriptCLI_h_

#include <Core/Core.h>
#include <ScriptCommon/ScriptCommon.h>

NAMESPACE_UPP

#include "CommandRegistry.h"
#include "RunCommand.h"
#include "LintCommand.h"
#include "PluginCommand.h"
#include "McpCommand.h"

int ScriptCliMain(const Vector<String>& args);

END_UPP_NAMESPACE

#endif
