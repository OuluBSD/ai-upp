#include "ScriptCLI.h"

NAMESPACE_UPP

int ScriptCliMain(const Vector<String>& args)
{
	return HandleScriptCliCommand(args);
}

END_UPP_NAMESPACE

CONSOLE_APP_MAIN
{
	Upp::SetExitCode(Upp::ScriptCliMain(Upp::CommandLine()));
}
