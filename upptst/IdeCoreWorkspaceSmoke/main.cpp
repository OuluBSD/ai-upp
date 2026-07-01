#include "IdeCoreWorkspaceSmoke.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	IdeCoreWorkspace workspace;
	(void)workspace.GetMainPackage();
	(void)workspace.GetNests(false);
}
