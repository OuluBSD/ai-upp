#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06kToyShaderFire(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06k_toyshader_fire/06k_toyshader_fire.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE