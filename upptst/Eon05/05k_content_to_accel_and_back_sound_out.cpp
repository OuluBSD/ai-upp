#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
		
	chain program:
		
		loop audio.in:
			id = "audio_gen"
			center.customer
			center.audio.src.dbg_generator
			center.audio.side.src.center.user[][id == "audio_conv_in"]
		
		loop ogl.fbo.audio.conv.in:
			id = "audio_conv_in"
			ogl.customer
			sdl.ogl.center.fbo.audio[id == "audio_gen"][id == "audio_fx"]
		
		loop ogl.fbo.audio.buffer:
			id = "audio_fx"
			ogl.customer
			sdl.ogl.fbo.side[id == "audio_conv_in"][id == "audio_conv_out"]:
				type = "audio"
				shader.frag.path = "shaders/toys/simple/simple_sound_mixer/stage1.glsl"
				retarded.local.time = true
		
		loop ogl.fbo.audio.conv.out:
			id = "audio_conv_out"
			ogl.customer
			sdl.ogl.fbo.center.audio[id == "audio_fx"][id == "audio_out"]
		
		loop audio.out:
			id = "audio_out"
			center.customer
			center.audio.side.sink.center.user[id == "audio_conv_out"]
			sdl.audio
		
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo.standalone:
				shader.frag.path = "shaders/toys/simple/simple_sound_mixer/stage0.glsl"

*/

NAMESPACE_UPP

void Run05kContentToAccelAndBackSoundOut(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05kContentToAccelAndBackSoundOut: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(GetDataFile("05k_content_to_accel_and_back_sound_out.eon"));
		break;
	default:
		throw Exc(Format("Run05kContentToAccelAndBackSoundOut: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
