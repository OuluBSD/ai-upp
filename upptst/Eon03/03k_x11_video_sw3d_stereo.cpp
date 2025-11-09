#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.sw.context
	
	chain program:
		state event.register
		
		loop video:
			center.customer
			x11.sw.fbo.program:
				drawmem =					"false"
				program =					"obj_view"
				shader.default.vtx.name =	"obj_view"
				shader.default.frag.name =	"obj_view"
				shader.stereo.vtx.name =	"stereo"
				shader.stereo.frag.name =	"stereo"
			
			x11.sw.video.pipe:
				close_machine =		true
				sizeable =			true
				env =				event.register
				type =				stereo
				fullscreen =			false
				find.vr.screen =		false

*/

NAMESPACE_UPP

void Run03kX11VideoSw3dStereo(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03kX11VideoSw3dStereo: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03k_x11_video_sw3d_stereo.eon"));
		break;
	default:
		throw Exc(Format("Run03kX11VideoSw3dStereo: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
