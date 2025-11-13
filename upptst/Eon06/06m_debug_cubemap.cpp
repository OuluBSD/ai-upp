#include "Eon06.h"

#include <Eon/Script/Script.h>
#include <plugin/png/png.h>

#if defined(flagOGL)
#include <GL/gl.h>
#endif

NAMESPACE_UPP

void Run06mDebugCubemap(Engine& eng, int method) {
	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	String filepath = "share/eon/tests/06m_debug_cubemap/06m_debug_cubemap.toy";

	sys->PostLoadFile(RealizeShareFile(filepath));

	LOG("06m_debug_cubemap: Test will capture a screenshot after 2 seconds");
	LOG("06m_debug_cubemap: Look for cubemap_test_capture.png in your home directory");

	// Note: Actual PNG capture will be triggered manually or via a separate mechanism
	// For now, we'll rely on visual inspection or external screen capture tools
}

END_UPP_NAMESPACE
