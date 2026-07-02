#include "dbg.h"

using namespace Upp;

static void PrintHelp()
{
	Cout() << "Usage: dbg [--help] [--backends]\n"
	       << "\n"
	       << "  --backends  List planned debugger backends.\n"
	       << "  --help      Show this help text.\n"
	       << "\n"
	       << "This is the first headless dbg CLI skeleton.\n";
}

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	bool show_help = args.IsEmpty();
	bool show_backends = false;

	for(const String& a : args) {
		if(a == "--help" || a == "-h")
			show_help = true;
		else if(a == "--backends")
			show_backends = true;
		else
			show_help = true;
	}

	if(show_backends) {
		const VectorMap<String, String> backends = GetPlannedDbgBackends();
		Cout() << "Planned backends: ";
		for(int i = 0; i < backends.GetCount(); i++) {
			if(i)
				Cout() << ", ";
			Cout() << backends.GetKey(i);
		}
		Cout() << "\n";
		Cout().Flush();
	}

	if(show_help) {
		PrintHelp();
		Cout().Flush();
	}

	SetExitCode(0);
}
