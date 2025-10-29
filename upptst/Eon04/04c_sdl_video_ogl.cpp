#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		
		loop ogl.fbo:
			ogl.customer
			sdl.fbo.standalone:
				shader.frag.path = "shaders/toys/simple/simple_single/stage0.glsl"
		
		loop audio:
			center.customer
			center.audio.src.dbg_generator:
				waveform = "noise"
			sdl.audio

*/

NAMESPACE_UPP

void Run04cSdlVideoOgl(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04cSdlVideoOgl: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("04c_sdl_video_ogl.eon"));
		break;
	default:
		throw Exc(Format("Run04cSdlVideoOgl: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
