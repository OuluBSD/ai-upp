#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		
		loop video:
			center.customer
			center.video.src.dbg_generator:
				mode = "noise"
			sdl.video.pipe
		
		loop audio:
			center.customer
			center.audio.src.dbg_generator:
				waveform = "noise"
			sdl.audio

*/

NAMESPACE_UPP

void Run04bSdlVideo(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04bSdlVideo: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/04b_sdl_video.eon"));
		break;
	default:
		throw Exc(Format("Run04bSdlVideo: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
