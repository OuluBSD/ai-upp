#include "Eon.h"

NAMESPACE_UPP


bool SingleMachine::Open(void(*arg_fn)()) {
	
	
	
	const AppFlags& flags = GetAppFlags();
	Machine& mach = GetActiveMachine();
	
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
    #endif
    
    #if IS_TS_CORE && defined flagGUI
    mach.Add<Gu::SurfaceSystem>();
    #endif
    
    #if IS_UPP_CORE && defined flagGUI
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
	
	
	Machine& mach = GetActiveMachine();
	mach.Stop();
}


END_UPP_NAMESPACE
