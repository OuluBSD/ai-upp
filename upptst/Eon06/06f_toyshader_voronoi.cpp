#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06fToyShaderVoronoi(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06f_toyshader_keyboard/06f_toyshader_keyboard.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE