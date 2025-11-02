#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		loop center.webcam:
			id = "cam"
			center.customer
			center.video.webcam[][id == "texture"]:
				vflip = "true"
		
		loop ogl.fbo.tex:
			id = "texture"
			ogl.customer
			sdl.ogl.fbo.image[id == "cam"][id == "screen"]
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo[id == "texture"]:
				shader.frag.path = "shaders/toys/simple/simple_webcam/stage0.glsl"

*/

NAMESPACE_UPP

void Run05hContentWebcam(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05hContentWebcam: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/05h_content_webcam.eon"));
		break;
	default:
		throw Exc(Format("Run05hContentWebcam: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
