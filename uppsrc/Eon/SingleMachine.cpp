#include "Eon.h"

#if 0
NAMESPACE_UPP

bool SingleMachine::Open(void(*arg_fn)()) {
	
	
	
	const AppFlags& flags = GetAppFlags();
	Engine& mach = GetActiveMachine();
	
	RegistrySystemPtr reg = mach.Add<RegistrySystem>();
	
	mach.Add<AtomStore>();
	mach.Add<LinkStore>();
	mach.Add<SpaceStore>();
	mach.Add<LoopStore>();
	
    mach.Add<AtomSystem>();
	mach.Add<LinkSystem>();
	
    mach.Add<ScriptLoader>();
    
    mach.Add<ModelCache>();
    
	#ifdef flagGUI
    mach.Add<Gu::GuboSystem>();
    mach.Add<Gu::SurfaceSystem>();
    #endif
    
    #if defined flagGUI
    mach.Add<WindowSystem>();
    #endif
    
	
    reg->SetAppName("Non-screen machine");
    
    if (arg_fn)
        arg_fn();
    
    if (!mach.Start())
		return false;
    
	return true;
}

void SingleMachine::Close() {
	
	
	Engine& mach = GetActiveMachine();
	mach.Stop();
}

END_UPP_NAMESPACE

#endif
