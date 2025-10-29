#include "Eon01.h"

/*
machine sdl.app:

	meta params channel.links():
		meta for int i = 0, i < channel.count, i++:
			loop channel.$i

	chain program:
		state event.register

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

void Run01dMetaTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run01dMetaTest: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("01d_meta_test.eon"));
		break;
	default:
		throw Exc(Format("Run01dMetaTest: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
