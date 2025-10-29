#include "Eon04.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		state event.register

		loop ogl.fbo:
			ogl.customer
			sdl.ogl.fbo.pipe:
				shader.frag.path = "shaders/toys/simple/simple_double/stage1.glsl"
			
			sdl.fbo.sink:
				close_machine =		true
				sizeable =			true
				env =				event.register
				shader.frag.path =	"shaders/toys/simple/simple_double/stage0.glsl"
				recv.data =			false

*/

NAMESPACE_UPP

void Run04eSdlVideoOglCustomLinked(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run04eSdlVideoOglCustomLinked: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("04e_sdl_video_ogl_custom_linked.eon"));
		break;
	default:
		throw Exc(Format("Run04eSdlVideoOglCustomLinked: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
