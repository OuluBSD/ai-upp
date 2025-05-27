#include "Eon.h"


NAMESPACE_UPP


void Easing::Visit(Vis& v) {
	v
	 VISN(target_position)
	 VISN(target_orientation)
	 VIS_(position_easing_factor)
	 VIS_(orientation_easing_factor);
	
	VISIT_COMPONENT
}

void Easing::Initialize() {
	Ptr<EasingSystem> sys = GetEngine().TryGet<EasingSystem>();
	if (sys)
		sys->Attach(this);
}

void Easing::Uninitialize() {
	Ptr<EasingSystem> sys = GetEngine().TryGet<EasingSystem>();
	if (sys)
		sys->Detach(this);
}






void EasingSystem::Update(double dt)
{
	for (Easing* easing : comps) {
		Ptr<Transform> transform = easing->GetEntity()->Find<Transform>();
		if (!transform)
			continue;
        
        TODO
    }
}

void EasingSystem::Attach(Easing* e) {
	VectorFindAdd(comps, e);
}

void EasingSystem::Detach(Easing* e) {
	VectorRemoveKey(comps, e);
}

 
END_UPP_NAMESPACE
