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
			fmsynth.pipe[loop == "event"]:
				filepath = "presets/fmsynth/presets/old_school_organ.fmp"
				verbose = false
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02eFmsynth(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02eFmsynth: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02e_fmsynth.eon"));
		break;
	default:
		throw Exc(Format("Run02eFmsynth: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
