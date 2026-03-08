#include "Maestro.h"

namespace Upp {

void TrackCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI track [-h] {list,add,remove}\n";
}

void TrackCommand::Execute(const Vector<String>& args) {
	Cout() << "Track command not yet fully implemented in C++ but is on the roadmap.\n";
}

}