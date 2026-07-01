#include "IdeCoreWorkspaceSmoke.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	IdeCoreWorkspace workspace;
	IdeCoreConsoleHost host;
	(void)workspace.GetMainPackage();
	(void)workspace.GetNests(false);
	(void)host.IsRunning();
	host.Kill();
}
