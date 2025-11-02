#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		
		loop ogl.fbo:
			ogl.customer
			x11.ogl.fbo.standalone:
				shader.frag.path = "shaders/toys/simple/simple_single/stage0.glsl"

*/

NAMESPACE_UPP

void Run03bGlxVideo(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03bGlxVideo: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03b_glx_video.eon"));
		break;
	default:
		throw Exc(Format("Run03bGlxVideo: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
