#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06aToyShaderSinglePassClouds(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06a_toyshader_singlepass_clouds/06a_toyshader_singlepass_clouds.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE