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

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run03eX11VideoSw3d: method %d not implemented yet", method));
	case 0:
		sys->SetEagerChainBuild(true);
		sys->PostLoadFile(ShareDirFile("eon/tests/03e_x11_video_sw3d.eon"));
		break;
	case 4: {
		// Method 4: Python script defines the video loop
		// Machine/driver/state must be defined first for X11 context
		sys->SetEagerChainBuild(true);
		LOG("Method 4: Queueing machine/driver string");
		sys->PostLoadString(
			"machine x11.app:\n"
			"	driver context:\n"
			"		x11.sw.context\n"
			"	chain program:\n"
			"		state event.register\n"
		);
		String py_path = ShareDirFile("py/eon/03e_x11_video_sw3d_method4.py");
		LOG("Method 4: Queueing Python file: " << py_path);
		sys->PostLoadPythonFile(py_path);
		LOG("Method 4: Python file queued, returning from runner");
		break;
	}
	default:
		throw Exc(Format("Run03eX11VideoSw3d: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
