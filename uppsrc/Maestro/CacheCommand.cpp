#include "Maestro.h"

namespace Upp {

void CacheCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI cache [-h] {stats,show,prune} ...\n"
	       << "positional arguments:\n"
	       << "    stats             Show cache statistics\n"
	       << "    show              Show cache entry details\n"
	       << "    prune             Prune old cache entries\n";
}

void CacheCommand::Execute(const Vector<String>& args) {
	Cout() << "Cache management logic is not yet fully ported to C++.\n";
}

void TrackCacheCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI track-cache [-h] {invalidate} ...\n"
	       << "positional arguments:\n"
	       << "    invalidate  Invalidate the cached track data for the current repo.\n";
}

void TrackCacheCommand::Execute(const Vector<String>& args) {
	Cout() << "Track cache logic is not yet fully ported to C++.\n";
}

}