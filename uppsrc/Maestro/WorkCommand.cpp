#include "Maestro.h"

namespace Upp {

void WorkCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI work [-h] [--ignore-gates]\n"
	       << "                       {any,track,phase,issue,task,discuss,analyze,fix,subwork,resume}\n"
	       << "                       ...\n"
	       << "positional arguments:\n"
	       << "  {any,track,phase,issue,task,discuss,analyze,fix,subwork,resume}\n"
	       << "                        Work subcommands\n"
	       << "    any                 Let AI select the best work item\n"
	       << "    track               Work on a track\n"
	       << "    phase               Work on a phase\n"
	       << "    issue               Work on an issue\n"
	       << "    task                Work on a task\n"
	       << "    discuss             Start a discussion for a work item\n"
	       << "    analyze             Analyze a target before work\n"
	       << "    fix                 Fix a target or issue\n"
	       << "    subwork             Manage subwork sessions\n"
	       << "    resume              Resume a work session\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n"
	       << "  --ignore-gates        Bypass all work gates\n";
}

void WorkCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'work' is not yet fully implemented in C++.\n";
}

}