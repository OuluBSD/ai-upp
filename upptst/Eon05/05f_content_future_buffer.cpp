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
				filepath = "uk_street.jpg"
		
		loop ogl.fbo.tex:
			id = "texture"
			ogl.customer
			sdl.ogl.fbo.image[id == "image"][id == "buffer2"]
		
		loop ogl.fbo.buffer2:
			id = "buffer2"
			ogl.customer
			sdl.ogl.fbo.side[id == "texture", id == "buffer1"][id == "screen", id == "buffer1"]:
				shader.frag.path = "shaders/toys/simple/simple_future_buffer/stage2.glsl"
		
		loop ogl.fbo.buffer1:
			id = "buffer1"
			ogl.customer
			sdl.ogl.fbo.side[id == "buffer2"][id == "buffer2"]:
				shader.frag.path = "shaders/toys/simple/simple_future_buffer/stage1.glsl"
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo[id == "buffer2"]:
				shader.frag.path = "shaders/toys/simple/simple_future_buffer/stage0.glsl"
		
		loop audio:
			center.customer
			center.audio.src.dbg_generator:
				waveform = "noise"
			sdl.audio

*/

NAMESPACE_UPP

void Run05fContentFutureBuffer(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05fContentFutureBuffer: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("05f_content_future_buffer.eon"));
		break;
	default:
		throw Exc(Format("Run05fContentFutureBuffer: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
