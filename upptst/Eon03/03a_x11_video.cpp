#include "Eon03.h"

/*
machine x11.app:
	
	driver context:
		x11.context
	
	chain program:
		
		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			x11.video.pipe

*/

NAMESPACE_UPP

void Run03aX11Video(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03aX11Video: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03a_x11_video.eon"));
		break;
	default:
		throw Exc(Format("Run03aX11Video: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
