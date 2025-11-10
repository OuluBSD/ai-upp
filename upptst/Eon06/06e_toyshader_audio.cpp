#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06eToyShaderAudio(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06e_toyshader_audio/06e_toyshader_audio.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE