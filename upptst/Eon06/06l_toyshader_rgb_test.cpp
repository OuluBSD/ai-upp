#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06lToyShaderRgbTest(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	String filepath = "share/eon/tests/06l_toyshader_rgb_test/06l_toyshader_rgb_test.toy";

	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE
