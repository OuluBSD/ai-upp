#include "Maestro.h"

namespace Upp {

void SolutionsCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI solutions [-h] {add,list,ls,remove,rm,show,edit} ...\n"
	       << "positional arguments:\n"
	       << "  {add,list,ls,remove,rm,show,edit}\n"
	       << "                        Solutions subcommands\n"
	       << "    add                 Add a new solution\n"
	       << "    list (ls)           List solutions\n"
	       << "    remove (rm)         Remove a solution\n"
	       << "    show                Show solution details\n"
	       << "    edit                Edit a solution in $EDITOR\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void SolutionsCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'solutions' is not yet fully implemented in C++.\n";
}

}