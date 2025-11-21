#include "Eon00.h"

/*
PacketRouter DSL Test - Fork topology (fan-out connections)

net tester.fork:
	center.audio.src.test
	center.customer
	center.audio.sink.test.realtime:
		dbg_limit = 100
	center.audio.src.test.0 -> center.customer.0
	center.audio.src.test.0 -> center.audio.sink.test.realtime.0
	center.customer.0 -> center.audio.sink.test.realtime.0

Tests:
- One-to-many connections (src.test -> customer, src.test -> sink)
- Multiple inputs to single sink (customer -> sink, src.test -> sink)
*/

NAMESPACE_UPP

void Run00eForkNet(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		// Load from .eon file using PacketRouter fork topology
		sys->PostLoadFile(ShareDirFile("eon/tests/00e_fork_net.eon"));
		break;
	default:
		throw Exc(Format("Run00eForkNet: method %d not implemented yet", method));
	}
}

END_UPP_NAMESPACE
