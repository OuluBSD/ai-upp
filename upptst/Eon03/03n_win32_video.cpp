#include "Eon03.h"

/*
machine win.app:
	
	driver context:
		win.context
	
	chain program:
		
		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			win.video.pipe

*/

NAMESPACE_UPP

void Run03nWin32Video(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03nWin32Video: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03n_win32_video.eon"));
		break;
	default:
		throw Exc(Format("Run03nWin32Video: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
