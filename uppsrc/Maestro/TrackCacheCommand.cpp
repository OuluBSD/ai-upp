#include "Maestro.h"

namespace Upp {

void TrackCacheCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI track-cache [-h] {invalidate} ...\n"
	       << "positional arguments:\n"
	       << "  {invalidate}\n"
	       << "    invalidate  Invalidate the cached track data for the current repo.\n"
	       << "options:\n"
	       << "  -h, --help    show this help message and exit\n";
}

void TrackCacheCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'track-cache' is not yet fully implemented in C++.\n";
}

}