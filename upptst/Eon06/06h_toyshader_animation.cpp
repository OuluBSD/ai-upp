#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06hToyShaderAnimation(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06h_toyshader_animation/06h_toyshader_animation.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE