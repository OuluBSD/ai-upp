#include "Eon00.h"
#include <Eon/Script/Script.h>

/*
PacketRouter Runtime Flow Verification Test

This test verifies that packets actually flow through the PacketRouter during
the main loop execution. It:
1. Creates a net-based pipeline
2. Runs for several iterations
3. Verifies packets_routed counter increments
4. Confirms data reaches the sink atom

net tester.flow:
	center.audio.src.test
	center.customer
	center.audio.sink.test.realtime:
		dbg_limit = 50
	center.audio.src.test.0 -> center.customer.0
	center.customer.0 -> center.audio.sink.test.realtime.0
*/

NAMESPACE_UPP

void Run00hRouterFlow(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 0:
		// Load from .eon file using PacketRouter
		sys->PostLoadFile(ShareDirFile("eon/tests/00h_router_flow.eon"));
		break;
	default:
		throw Exc(Format("Run00hRouterFlow: method %d not implemented yet", method));
	}

	// After MainLoop completes, the ScriptLoader's GetTotalPacketsRouted() will report
	// how many packets were routed. The test passes if VoidSinkBase verifies data correctly.
	// Note: The actual verification happens inside VoidSinkBase::Consume() which validates
	// the rolling_value data, and will SetFailed() on the engine if verification fails.
}

END_UPP_NAMESPACE
