#include "Eon07.h"
#include <Eon/Interaction/Player.h>
#include <Eon/Draw/ToolboxSystem.h>
#include <Eon/Draw/PaintingSystem.h>
#include <Eon/Draw/ShootingSystem.h>
#include <Eon/Draw/ThrowingSystem.h>
#include <Eon/Draw/PaintStrokeSystem.h>

NAMESPACE_UPP

void Run07lDemoroomFakespatialX11Sw3d(Engine& eng, int method) {
	// Add required systems for DemoRoom
	eng.GetAdd<InteractionSystem>();
	eng.GetAdd<RenderingSystem>();
	eng.GetAdd<EventSystem>();
	eng.GetAdd<ModelCache>();
	eng.GetAdd<PhysicsSystem>();
	eng.GetAdd<PlayerBodySystem>();
	eng.GetAdd<ToolboxSystemBase>();
	eng.GetAdd<PaintStrokeSystemBase>();
	eng.GetAdd<PaintingInteractionSystemBase>();
	eng.GetAdd<ShootingInteractionSystemBase>();
	eng.GetAdd<ThrowingInteractionSystemBase>();

	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07lDemoroomFakespatialX11Sw3d: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/07l_demoroom_fakespatial_x11_sw3d.eon"));
		break;
	default:
		throw Exc(Format("Run07lDemoroomFakespatialX11Sw3d: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
