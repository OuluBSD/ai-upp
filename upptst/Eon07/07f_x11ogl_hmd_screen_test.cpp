#include "Eon07.h"

/*
machine app:
	
	driver context:
		x11.ogl.context
	
	chain program:
		state event.register
		
		loop ogl.fbo:
			ogl.customer
			x11.ogl.fbo.standalone:
				close_machine =			true
				fullscreen =			true
				find.vr.screen =		true
				env =					event.register
				shader.frag.path =		"shaders/toys/stereo/mario_cross_eye_3d/stage0.glsl"

*/

NAMESPACE_UPP

void Run07fX11oglHmdScreenTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07fX11oglHmdScreenTest: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/07f_x11ogl_hmd_screen_test.eon"));
		break;
	default:
		throw Exc(Format("Run07fX11oglHmdScreenTest: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
