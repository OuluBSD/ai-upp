#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06cToyShaderWithTexture(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06c_toyshader_with_texture/06c_toyshader_with_texture.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE