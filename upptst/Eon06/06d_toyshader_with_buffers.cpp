#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06dToyShaderWithBuffers(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06d_toyshader_with_buffers/06d_toyshader_with_buffers.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE