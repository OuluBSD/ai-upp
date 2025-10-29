#include "Eon05.h"

/*
machine sdl.app:

	driver context:
		sdl.context
	
	chain program:
	
		loop ogl.fbo:
			ogl.customer
			sdl.fbo.standalone:
				shader.frag.path = "shaders/toys/simple/simple_single/stage0.glsl"

*/

NAMESPACE_UPP

void Run05aContentSingle(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05aContentSingle: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("05a_content_single.eon"));
		break;
	default:
		throw Exc(Format("Run05aContentSingle: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
