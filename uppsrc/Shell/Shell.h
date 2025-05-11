#ifndef _Shell_Shell_h_
#define _Shell_Shell_h_


#include <AICore2/AICore.h>
#include <Geometry/Geometry.h>
#include <Meta/Meta.h>
#include <Eon/Eon.h>
#include <EonDraw/EonDraw.h>
#include <EonLib/EonLib.h>


#ifdef flagGUI
#include <EonCtrl/EonCtrl.h>

#if defined flagUWP && defined flagDX12
	#include <EcsWin/EcsWin.h>
#endif
#endif

#ifdef flagGUI
	#include <GuboLib/GuboLib.h>
#endif

#ifdef flagGUI
	#include <EcsCtrl/EcsCtrl.h>
#endif

#ifdef flagVR
	#include <EcsVR/EcsVR.h>
#endif

#ifdef flagGUBO
	#include <GuboSuite/GuboSuite.h>
#endif



NAMESPACE_UPP

void MachineEcsInit(Machine& mach);
void EngineEcsInit(Ecs::Engine& eng);
void BindEcsToSerial();
void DebugMain(String script_content, String eon_file, VectorMap<String,Value>& args, MachineVerifier* ver=0, bool dbg_ref_visits=false, uint64 dbg_ref=0);

template <class T> void DefaultCreate() {TODO /*Ecs::GetActiveEngine().Get<EntityStore>()->GetRoot()->Create<T>();*/}
template <class T> void DefaultCreateOnStart() {TODO /*Ecs::Engine::WhenPreFirstUpdate << callback(DefaultCreate<T>);*/}

END_UPP_NAMESPACE

#endif

