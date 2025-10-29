#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
		
	chain program:
		
		loop center.image:
			id = "image"
			center.customer
			center.image.loader[][id == "texture"]:
				vflip = true
				cubemap = true
				swap_top_bottom = true
				filepath = "imgs/488bd40303a2e2b9a71987e48c66ef41f5e937174bf316d3ed0e86410784b919.jpg"
		
		loop ogl.fbo.tex:
			id = "texture"
			ogl.customer
			sdl.ogl.fbo.image[id == "image"][id == "screen"]
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo[id == "texture"]:
				buf0 = "cubemap"
				shader.frag.path = "shaders/toys/cubemap/copper_mirror_ball/stage0.glsl"

*/

NAMESPACE_UPP

void Run05eContentCubemap(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05eContentCubemap: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("05e_content_cubemap.eon"));
		break;
	default:
		throw Exc(Format("Run05eContentCubemap: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
