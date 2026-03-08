#include "Maestro.h"

namespace Upp {

void TutorialCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI tutorial [-h] [name_or_number] [page]\n"
	       << "Interactive tutorials for Maestro features.\n"
	       << "Available Tutorials:\n"
	       << "  1. intro                    \n"
	       << "  2. resolve-cli              \n"
	       << "  3. resolve-cli-for-daemon   \n"
	       << "  4. resolve-runbooks         \n"
	       << "  5. resolve-runbook-workflows\n"
	       << "  6. work-track               \n"
	       << "  7. write-track              \n"
	       << "Run 'maestro tutorial list' for full descriptions.\n"
	       << "positional arguments:\n"
	       << "  name_or_number  Tutorial name, number, or \"list\"\n"
	       << "  page            Page number (starting from 1)\n"
	       << "options:\n"
	       << "  -h, --help      show this help message and exit\n";
}

void TutorialCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'tutorial' is not yet fully implemented in C++.\n";
}

}