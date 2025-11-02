#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.sw.context
	
	chain program:
		state event.register
		
		loop video:
			center.customer
			x11.sw.fbo.standalone:
				close_machine =			true
				sizeable =				true
				env =					event.register
				type =					custom
				stages =				3
				s0.shader.vtx.name =	"pass"
				s0.shader.frag.name =	"color_test"
				s1.shader.vtx.name =	"pass"
				s1.shader.frag.name =	"color_test"
				s2.shader.vtx.name =	"stereo"
				s2.shader.frag.name =	"stereo"
				s2.quad.count =			2

*/

NAMESPACE_UPP

void Run03iX11VideoSw3dBufferstages(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03iX11VideoSw3dBufferstages: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03i_x11_video_sw3d_bufferstages.eon"));
		break;
	default:
		throw Exc(Format("Run03iX11VideoSw3dBufferstages: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
