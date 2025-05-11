#include "Shell.h"

NAMESPACE_UPP




void MachineEcsInit(Machine& mach) {
	
	#ifdef flagPHYSICS
	#ifdef flagODE
	mach.FindAdd<SystemT<OdeFys>>();
	#else
	mach.FindAdd<SystemT<TosFys>>();
	#endif
	#endif
	
	Ecs::Engine::WhenInitialize << callback(EngineEcsInit);
}

void EngineEcsInit(Ecs::Engine& eng) {
    
	eng.GetAdd<Ecs::RegistrySystem>();
	eng.GetAdd<Ecs::InteractionSystem>();
	
	eng.GetAdd<Ecs::RenderingSystem>();
	//eng.GetAdd<Ecs::ComponentStore>();
	eng.GetAdd<Ecs::EventSystem>();
	
	#if 0
	#if HAVE_WINDOWSYSTEM
	eng.GetAdd<Ecs::VirtualGuiSystem>();
	eng.GetAdd<Ecs::WindowSystem>();
	#endif
	#endif
	
	//DefaultSerialInitializerInternalEon();
}


END_UPP_NAMESPACE
