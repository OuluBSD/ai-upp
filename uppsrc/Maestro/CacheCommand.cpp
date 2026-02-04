#include "Maestro.h"

namespace Upp {

void CacheCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI cache [-h] {stats,show,prune} ...\n"
	       << "positional arguments:\n"
	       << "  {stats,show,prune}  Cache subcommands\n"
	       << "    stats             Show cache statistics\n"
	       << "    show              Show cache entry details\n"
	       << "    prune             Prune old cache entries\n"
	       << "options:\n"
	       << "  -h, --help          show this help message and exit\n";
}

void CacheCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'cache' is not yet fully implemented in C++.\n";
}

}