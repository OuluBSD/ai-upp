#include <Core/Core.h>
#include <Vfs/ProgDB/ProgDB.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.IsEmpty() || args[0] == "help" || args[0] == "--help") {
		Cout() << "ade: agentic development environment\n";
		Cout() << "commands: help, status\n";
		SetExitCode(0);
		return;
	}
	if(args[0] == "status") {
		Cout() << "status: scaffold\n";
		SetExitCode(0);
		return;
	}
	Cerr() << "ade: unknown command: " << args[0] << "\n";
	SetExitCode(1);
}
