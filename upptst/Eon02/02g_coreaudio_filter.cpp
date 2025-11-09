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
			softinstru.pipe[loop == "event"]:
				filepath = "TimGM6mb.sf2"
				verbose = false
			corefx.pipe:
				filter = "echo"
			center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02gCoreaudioFilter(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02gCoreaudioFilter: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02g_coreaudio_filter.eon"));
		break;
	default:
		throw Exc(Format("Run02gCoreaudioFilter: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
