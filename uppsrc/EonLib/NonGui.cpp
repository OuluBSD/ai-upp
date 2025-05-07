#include "EonLib.h"

NAMESPACE_UPP


bool Open_NonGUI_ECS() {
	using namespace Upp;
	using namespace Upp::ECS;
	Engine mach;
	
	Ecs::RegistrySystem& reg = *mach.Add<Ecs::RegistrySystem>();
	EntityStore& ents = *mach.Add<EntityStore>();
    mach.Add<ComponentStore>();
    
    reg.SetAppName("ECS machine");
    
    if (!mach.Start())
		return false;
    
	return true;
}

void Close_NonGUI_ECS() {
	using namespace Upp;
	using namespace Upp::ECS;
	Engine mach;
	
	mach.Stop();
}


END_UPP_NAMESPACE

