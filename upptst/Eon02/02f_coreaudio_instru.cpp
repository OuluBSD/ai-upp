#include "Eon02.h"

/*
machine midi.app:
	chain program:
		loop event:
			center.customer
			midi.file.reader[][loop == "input"]:
				filepath = "midi/short_piano.mid"
				close_machine = true

		loop input:
			center.customer
			coresynth.pipe[loop == "event"]:
				instrument = "rhodey"
				verbose = false
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02fCoreaudioInstru(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02fCoreaudioInstru: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02f_coreaudio_instru.eon"));
		break;
	default:
		throw Exc(Format("Run02fCoreaudioInstru: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
