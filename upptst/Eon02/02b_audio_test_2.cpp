#include "Eon02.h"

/*
chain player.audio.generator:

	loop output:
		center.customer
		center.audio.src.dbg_generator
		center.audio.side.src.center

	loop input:
		center.customer
		center.audio.side.sink.center
		center.audio.sink.hw

*/

NAMESPACE_UPP

void Run02bAudioTest2(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run02bAudioTest2: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/02b_audio_test_2.eon"));
		break;
	default:
		throw Exc(Format("Run02bAudioTest2: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
