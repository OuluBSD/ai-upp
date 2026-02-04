#include "Maestro.h"

namespace Upp {

void LogCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI log [-h] {scan,list,ls,show,sh} ...\n"
	       << "positional arguments:\n"
	       << "  {scan,list,ls,show,sh}\n"
	       << "                        Log subcommands\n"
	       << "    scan                Scan build/run output or log files for errors and warnings\n"
	       << "    list (ls)           List all log scans\n"
	       << "    show (sh)           Show scan details and findings\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void LogCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'log' is not yet fully implemented in C++.\n";
}

}