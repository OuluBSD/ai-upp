#include "Eon03.h"

/*
machine x11.app:
	
	driver context:
		x11.sw.context
	
	chain program:
		state event.register
		
		loop video:
			center.customer
			x11.sw.fbo.program:
				drawmem =		"false"
				program =		"obj_view"
				shader.default.frag.name =	"obj_view"
				shader.default.vtx.name =	"obj_view"
			
			x11.sw.video.pipe:
				close_machine =	true
				sizeable =		true
				env =			event.register

*/

NAMESPACE_UPP

void Run03eX11VideoSw3d(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03eX11VideoSw3d: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/03e_x11_video_sw3d.eon"));
		break;
	case 4:
		// Method 4: Python script needs X11 context created first
		// The Python script only creates the video loop, not the machine/driver
		sys->PostLoadString(
			"machine x11.app:\n"
			"	driver context:\n"
			"		x11.sw.context\n"
			"	chain program:\n"
			"		state event.register\n"
		);
		sys->PostLoadPythonFile(ShareDirFile("py/eon/03e_x11_video_sw3d_method4.py"));
		break;
	default:
		throw Exc(Format("Run03eX11VideoSw3d: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
