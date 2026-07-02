#include "dbg.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	SetExitCode(RunDbgCli(CommandLine()));
}
