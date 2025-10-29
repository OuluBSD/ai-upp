#include "Eon03.h"

/*
machine glx.app:
	
	driver context:
		x11.sw.context
	
	chain program:
		state event.register
		
		loop video:
			center.customer
			x11.sw.fbo.pipe:
				shader.vtx.name =	"pass"
				shader.frag.name =	"color_test"
			
			x11.sw.fbo.sink:
				close_machine =				true
				sizeable =					true
				env =						event.register
				shader.vtx.name =	"pass"
				shader.frag.name =	"proxy_input0"
				recv.data =					false

*/

NAMESPACE_UPP

void Run03gX11VideoSw3dLinked(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03gX11VideoSw3dLinked: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("03g_x11_video_sw3d_linked.eon"));
		break;
	default:
		throw Exc(Format("Run03gX11VideoSw3dLinked: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
