#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		state event.register
		
		loop event:
			center.customer
			sdl.event.pipe
			state.event.pipe:
				target = event.register
		
		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			sdl.video.pipe

*/

NAMESPACE_UPP

void Run04gSdlEventStandalone(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04gSdlEventStandalone: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/04g_sdl_event_standalone.eon"));
		break;
	default:
		throw Exc(Format("Run04gSdlEventStandalone: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
