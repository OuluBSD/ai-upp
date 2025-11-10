#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06jToyShaderWater(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06j_toyshader_water/06j_toyshader_water.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE