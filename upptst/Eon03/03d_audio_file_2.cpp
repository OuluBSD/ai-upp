#include "Eon03.h"

/*
chain player.audio.generator:
	
	loop output:
		center.customer
		perma.audio.source.decoder:
			filepath = "${FILE}"
			stop_machine = true
		center.audio.side.src.center
	
	loop input:
		center.customer
		center.audio.side.sink.center
		center.audio.sink.hw

*/

NAMESPACE_UPP

void Run03dAudioFile2(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03dAudioFile2: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03d_audio_file_2.eon"));
		break;
	default:
		throw Exc(Format("Run03dAudioFile2: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
