#include "Eon01.h"

/*
machine sdl.app:

	meta loopstmt pipe.fn():
		event.src.test.pipe
		state.event.pipe:
			target = event.register
			dbg_limit = 100

	chain program:
		state event.register

		loop event1:
			center.customer
			$pipe.fn()

*/

NAMESPACE_UPP

void Run01cMetaTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run01cMetaTest: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("01c_meta_test.eon"));
		break;
	default:
		throw Exc(Format("Run01cMetaTest: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
