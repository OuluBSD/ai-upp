#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06bToyShaderMultiPass(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06b_toyshader_multipass/06b_toyshader_multipass.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE