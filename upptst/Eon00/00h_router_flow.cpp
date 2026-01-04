#include "Eon00.h"
#include <Eon/Script/Script.h>

/*
PacketRouter Runtime Flow Verification Test

This test verifies that packets flow through PacketRouter when the net
includes the debug audio/video generators, SDL event/audio bridges, and
PortMidi hardware source so the metadata-driven credit path is exercised.
The net is described by share/eon/tests/00h_router_flow.eon and dispatches
packets through sdl.audio, sdl.video.pipe, sdl.event.pipe, and midi.null.sink.
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
