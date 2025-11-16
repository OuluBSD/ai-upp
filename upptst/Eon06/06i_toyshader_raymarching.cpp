#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06iToyShaderRaymarching(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	String filepath = "share/eon/tests/06i_toyshader_volume/06i_toyshader_volume.toy";

	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE