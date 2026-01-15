#include "PythonCLI.h"

using namespace Upp;

CONSOLE_APP_MAIN
{
	const auto& cmds = CommandLine();

	if(cmds.GetCount() > 0) {
		// If command line arguments are provided, treat the first as a script file
		String filename = cmds[0];
		Cout() << "Loading script file: " << filename << "\n";
		PythonCLI cli;
		if (!cli.RunScript(filename))
			SetExitCode(1);
	} else {
		// Otherwise, run in interactive mode
		Cout() << "No arguments, running in interactive mode\n";
		PythonCLI cli;
		SetExitCode(cli.Run());
	}
}