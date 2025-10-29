#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register
		
		loop ogl.fbo:
			ogl.customer
			x11.ogl.fbo.program:
				drawmem =				"false"
				program =				"obj_view"
				shader.default.vtx.path =	"shaders/tests/03f_vertex.glsl"
				shader.default.frag.path =	"shaders/tests/03f_fragment.glsl"
			
			x11.ogl.fbo.sink:
				close_machine =		true
				sizeable =			true
				env =				event.register
				type =				stereo
				fullscreen =		false
				find.vr.screen =	false

*/

NAMESPACE_UPP

void Run03lX11VideoOglStereo(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03lX11VideoOglStereo: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03l_x11_video_ogl_stereo.eon"));
		break;
	default:
		throw Exc(Format("Run03lX11VideoOglStereo: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
