#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register

		loop ogl.fbo:
			ogl.customer
			x11.ogl.fbo.pipe:
				shader.frag.path =	"shaders/toys/simple/simple_double/stage1.glsl"
			
			x11.ogl.fbo.sink:
				close_machine =		true
				sizeable =			true
				env =				event.register
				shader.frag.path =	"shaders/toys/simple/simple_double/stage0.glsl"
				recv.data =			false

*/

NAMESPACE_UPP

void Run03hX11VideoOglLinked(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03hX11VideoOglLinked: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03h_x11_video_ogl_linked.eon"));
		break;
	default:
		throw Exc(Format("Run03hX11VideoOglLinked: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
