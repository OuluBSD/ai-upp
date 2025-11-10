#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06gToyShaderFractal(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);
	
	String filepath = "share/eon/tests/06g_toyshader_fractal/06g_toyshader_fractal.toy";
	
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE