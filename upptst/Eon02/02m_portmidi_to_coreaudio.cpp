#include "Eon02.h"

/*
machine midi.app:
	chain program:
		loop event:
			center.customer
			midi.src.side.portmidi[][loop == "input"]
		loop input:
			center.customer
			coresynth.pipe[loop == "event"]:
				instrument = "rhodey"
				verbose = false
			center.audio.sink.hw:
				realtime = true

*/

NAMESPACE_UPP

void Run02mPortmidiToCoreaudio(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02mPortmidiToCoreaudio: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("02m_portmidi_to_coreaudio.eon"));
		break;
	default:
		throw Exc(Format("Run02mPortmidiToCoreaudio: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
