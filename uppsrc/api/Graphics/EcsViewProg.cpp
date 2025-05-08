#include "Graphics.h"

#include <EcsLib/EcsLib.h>

NAMESPACE_UPP


EcsViewProg::EcsViewProg() {
	
}


void EcsViewProg::Initialize() {
	
}


void EcsViewProg::Uninitialize() {
	
}


bool EcsViewProg::Arg(const String& key, const String& value) {
	
	return true;
}


bool EcsViewProg::Render(Draw& fb) {
	GfxStateDraw* sd = CastPtr<GfxStateDraw>(&fb);
	ASSERT(sd);
	
	GfxDataState& state = sd->GetState();
	ASSERT(state.mach);
	
	Machine& mach = *state.mach;
	Ecs::Engine& eng = mach.GetEngine();
	
	Ecs::RenderingSystemPtr rend = eng.Get<Ecs::RenderingSystem>();
	ASSERT(rend);
	
	
	// Default values
	state.light_dir = vec3 {sinf(DEG2RAD(75)), 0.0, cosf(DEG2RAD(20))};
	
	
	rend->Render(state);
	
	return true;
}


END_UPP_NAMESPACE
