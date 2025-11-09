#include "Eon02.h"

/*
machine midi.app:
	chain program:
		loop event:
			center.customer
			midi.src.side.portmidi[][loop == "input"]
		loop input:
			center.customer
			fluidsynth.pipe[loop == "event"]:
				patch = 1
			center.audio.sink.hw:
				realtime = true

*/

NAMESPACE_UPP

void Run02lPortmidiToFluidsynth(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02lPortmidiToFluidsynth: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02l_portmidi_to_fluidsynth.eon"));
		break;
	default:
		throw Exc(Format("Run02lPortmidiToFluidsynth: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
