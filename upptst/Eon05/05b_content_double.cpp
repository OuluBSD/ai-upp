#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		
		loop ogl.fbo:
			ogl.customer
			sdl.ogl.fbo.pipe:
				shader.frag.path = "shaders/toys/simple/simple_double/stage1.glsl"
			sdl.fbo.sink:
				shader.frag.path = "shaders/toys/simple/simple_double/stage0.glsl"
				recv.data = false

*/

NAMESPACE_UPP

void Run05bContentDouble(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05bContentDouble: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/05b_content_double.eon"));
		break;
	default:
		throw Exc(Format("Run05bContentDouble: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
