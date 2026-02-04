#include "Maestro.h"

namespace Upp {

void SettingsCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI settings [-h]\n"
	       << "                           {list,ls,l,get,g,set,s,edit,e,reset,r,wizard,w,profile,prof,pr,help,h}\n"
	       << "                           ...\n"
	       << "positional arguments:\n"
	       << "  {list,ls,l,get,g,set,s,edit,e,reset,r,wizard,w,profile,prof,pr,help,h}\n"
	       << "                        Settings subcommands\n"
	       << "    profile (prof, pr)  Manage settings profiles\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void SettingsCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'settings' is not yet fully implemented in C++.\n";
}

}