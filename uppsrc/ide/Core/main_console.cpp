#include "Core.h"
#include "ConsoleHeadless.h"

namespace Upp {

#ifndef flagGUI

void RunConsoleIde()
{
	if(HandleConsoleIdeArgs(CommandLine()))
		return;
	Cout() << GetConsoleIdeExperimentalNotice() << "\n";
}

CONSOLE_APP_MAIN
{
	RunConsoleIde();
}

#endif

} // namespace Upp
