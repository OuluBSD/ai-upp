#include "Maestro.h"

namespace Upp {

void UxCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI ux [-h] {eval,postmortem,list,show} ...\n"
	       << "positional arguments:\n"
	       << "  {eval,postmortem,list,show}\n"
	       << "                        UX subcommands\n"
	       << "    eval                Run blindfold UX evaluation for a goal\n"
	       << "    postmortem          Turn UX eval findings into issues and WorkGraph\n"
	       << "    list                List all UX evaluation runs\n"
	       << "    show                Show UX evaluation summary\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void UxCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'ux' is not yet fully implemented in C++.\n";
}

}