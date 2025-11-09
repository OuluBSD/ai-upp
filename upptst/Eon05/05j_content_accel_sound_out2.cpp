#include "Eon05.h"

/*
machine sdl.app:
	
	driver context:
		sdl.context
		
	chain program:
		
		loop ogl.fbo.audio.buffer:
			id = "audio_gen"
			ogl.customer
			sdl.ogl.fbo.side[][id == "audio_conv"]:
				type = "audio"
				retarded.local.time = true
				shader.frag.path = "shaders/toys/fm/synth_fm/stage1.glsl"
				
		loop ogl.fbo.screen:
			id = "screen"
			ogl.customer
			sdl.fbo.standalone:
				shader.frag.path = "shaders/toys/fm/synth_fm/stage0.glsl"
		
		loop ogl.fbo.audio.conv:
			id = "audio_conv"
			ogl.customer
			sdl.ogl.fbo.center.audio[id == "audio_gen"][id == "audio_out"]
		
		loop audio:
			id = "audio_out"
			center.customer
			center.audio.side.sink.center.user[id == "audio_conv"]
			sdl.audio

*/

NAMESPACE_UPP

void Run05jContentAccelSoundOut2(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run05jContentAccelSoundOut2: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/05j_content_accel_sound_out2.eon"));
		break;
	default:
		throw Exc(Format("Run05jContentAccelSoundOut2: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
