#include "EonLib.h"

NAMESPACE_UPP


bool Open_NonGUI_ECS() {
	using namespace Upp;
	
	TODO
	#if 0
	Engine mach;
	
	Ecs::RegistrySystem& reg = *mach.Add<Ecs::RegistrySystem>();
	EntityStore& ents = *mach.Add<EntityStore>();
    mach.Add<ComponentStore>();
    
    reg.SetAppName("ECS machine");
    
    if (!mach.Start())
		return false;
    #endif
    
	return true;
}

void Close_NonGUI_ECS() {
	TODO
	#if 0
	using namespace Upp;
	Engine mach;
	
	mach.Stop();
	#endif
}


END_UPP_NAMESPACE

