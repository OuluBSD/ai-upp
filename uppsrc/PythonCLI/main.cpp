#include "PythonCLI.h"

using namespace Upp;

CONSOLE_APP_MAIN

{

	PythonCLI cli;

	SetExitCode(cli.Run());

}
