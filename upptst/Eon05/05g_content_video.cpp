#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
	
	chain program:
		loop center.video:
			id = "cam"
			center.customer
			center.video.loader [][id == "texture"]:
				vflip = "true"
				filepath = "videos/35c87bcb8d7af24c54d41122dadb619dd920646a0bd0e477e7bdc6d12876df17.webm"
		
		loop ogl.fbo.tex:
			id = "texture"
			ogl.customer
			sdl.ogl.fbo.image[id == "cam"][id == "screen"]
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo[id == "texture"]:
				shader.frag.path = "shaders/toys/simple/simple_video/stage0.glsl"

*/

NAMESPACE_UPP

void Run05gContentVideo(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05gContentVideo: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("05g_content_video.eon"));
		break;
	default:
		throw Exc(Format("Run05gContentVideo: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
