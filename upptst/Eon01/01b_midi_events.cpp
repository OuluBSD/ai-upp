#include "Eon01.h"

/*
machine midi.app:

	chain program:
		loop event:
			center.customer
			midi.file.reader.pipe:
				filepath = "midi/saturday_show.mid"
				close_machine = true
			midi.null.sink:
				verbose = true

*/

NAMESPACE_UPP

void Run01bMidiEvents(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run01bMidiEvents: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/01b_midi_events.eon"));
		break;
	default:
		throw Exc(Format("Run01bMidiEvents: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
