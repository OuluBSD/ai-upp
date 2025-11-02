#include "Eon03.h"

/*
loop player.audio.ffmpeg:
	center.customer
	perma.audio.source.decoder:
		filepath = "${FILE}"
		stop_machine = true
	center.audio.sink.hw

*/

NAMESPACE_UPP

void Run03cAudioFile(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03cAudioFile: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03c_audio_file.eon"));
		break;
	default:
		throw Exc(Format("Run03cAudioFile: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
