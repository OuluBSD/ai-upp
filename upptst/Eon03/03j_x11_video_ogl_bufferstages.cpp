#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register
		
		loop ogl.video:
			ogl.customer
			x11.ogl.fbo.standalone:
				close_machine =			true
				sizeable =				true
				env =					event.register
				type =					custom
				stages =				3
				s0.shader.vtx.path =	"shaders/tests/03j_0_vertex.glsl"
				s0.shader.frag.path =	"shaders/tests/03j_0_fragment.glsl"
				s1.shader.vtx.path =	"shaders/tests/03j_1_vertex.glsl"
				s1.shader.frag.path =	"shaders/tests/03j_1_fragment.glsl"
				s2.shader.vtx.path =	"shaders/tests/03j_2_vertex.glsl"
				s2.shader.frag.path =	"shaders/tests/03j_2_fragment.glsl"

*/

NAMESPACE_UPP

void Run03jX11VideoOglBufferstages(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03jX11VideoOglBufferstages: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03j_x11_video_ogl_bufferstages.eon"));
		break;
	default:
		throw Exc(Format("Run03jX11VideoOglBufferstages: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
