#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		
		loop audio:
			center.customer
			center.audio.src.dbg_generator
			sdl.audio

*/

NAMESPACE_UPP

void Run04aSdlAudio(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04aSdlAudio: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("04a_sdl_audio.eon"));
		break;
	default:
		throw Exc(Format("Run04aSdlAudio: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
