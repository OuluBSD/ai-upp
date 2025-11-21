#include "Eon00.h"

/*
PacketRouter DSL Test - Audio generator using net syntax

net tester.generator:
	center.customer
	center.audio.src.test
	center.audio.sink.test.realtime:
		dbg_limit = 100
	center.customer.0 -> center.audio.src.test.0
	center.audio.src.test.0 -> center.audio.sink.test.realtime.0
*/

NAMESPACE_UPP

void Run00dAudioGenNet(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		// Load from .eon file using PacketRouter net syntax
		sys->PostLoadFile(ShareDirFile("eon/tests/00d_audio_gen_net.eon"));
		break;
	default:
		throw Exc(Format("Run00dAudioGenNet: method %d not implemented yet", method));
	}
}

END_UPP_NAMESPACE
