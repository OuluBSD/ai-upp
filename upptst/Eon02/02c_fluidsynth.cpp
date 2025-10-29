#include "Eon02.h"

/*
machine midi.app:

	chain program:

		loop event:
			center.customer
			midi.file.reader[][loop == "input"]:
				filepath = "midi/saturday_show.mid"
				close_machine = true

		loop input:
			center.customer
			fluidsynth.pipe[loop == "event"]:
				verbose = false
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02cFluidsynth(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02cFluidsynth: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("02c_fluidsynth.eon"));
		break;
	default:
		throw Exc(Format("Run02cFluidsynth: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
