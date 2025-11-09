#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		state event.register
		
		loop ogl.fbo.buffer:
			id = "buffer"
			ogl.customer
			sdl.ogl.fbo.side [][id == "screen"]:
				shader.frag.path = "shaders/toys/simple/simple_double/stage1.glsl"
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo:
				shader.frag.path = "shaders/toys/simple/simple_double/stage0.glsl"
				recv.data =			false
		
		loop audio:
			center.customer
			center.audio.src.dbg_generator:
				waveform = "noise"
			sdl.audio

*/

NAMESPACE_UPP

void Run04fSdlVideoOglCustomLinkedSide(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04fSdlVideoOglCustomLinkedSide: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/04f_sdl_video_ogl_custom_linked_side.eon"));
		break;
	default:
		throw Exc(Format("Run04fSdlVideoOglCustomLinkedSide: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
