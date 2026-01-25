#include "GuboLib.h"

NAMESPACE_UPP


void TopGubo::CreateGeom3DComponent() {
#ifdef flagEON
	using namespace Ecs;
	
	
	
	Engine& mach = GetActiveMachine();
	Gu::GuboSystemRef wins = mach.Get<Gu::GuboSystem>();
	Gu::GuboManager& mgr = wins->GetActiveScope();
	mgr.AddInterface(*this);
#endif
}


END_UPP_NAMESPACE
