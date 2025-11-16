#include "Eon06.h"

#include <Eon/Script/Script.h>

NAMESPACE_UPP

void Run06kToyShaderSound(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	// Try to load a version that may work better with audio availability
	String filepath = "share/eon/tests/06k_toyshader_sound/06k_toyshader_sound.toy";

	// Load the file - this is causing the crash when audio isn't available
	sys->PostLoadFile(RealizeShareFile(filepath));
}

END_UPP_NAMESPACE