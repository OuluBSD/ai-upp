#include "Eon03.h"

/*
machine sdl.app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register
		
		loop ogl.fbo:
			ogl.customer
			x11.ogl.fbo.program:
				drawmem =						"false"
				program =						"obj_view"
				shader.default.frag.path =		"shaders/tests/03n_fragment.glsl"
				shader.default.vtx.path =		"shaders/tests/03n_vertex.glsl"
				shader.sky.frag.path =			"shaders/tests/03n_skybox_fragment.glsl"
				shader.sky.vtx.path =			"shaders/tests/03n_skybox_vertex.glsl"
				program.arg.use.pbr =			true
				program.arg.model =				"ms/Gun.obj"
				program.arg.skybox.diffuse =	"bg5"
				program.arg.skybox.irradiance =	"bg1"
			x11.ogl.fbo.sink:
				close_machine =	true
				sizeable =		true
				env =			event.register

*/

NAMESPACE_UPP

void Run03mX11VideoOglPbr(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03mX11VideoOglPbr: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03m_x11_video_ogl_pbr.eon"));
		break;
	default:
		throw Exc(Format("Run03mX11VideoOglPbr: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
