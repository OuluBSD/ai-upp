#ifndef _Eon_Eon_h_
#define _Eon_Eon_h_

#include <type_traits>
#include <AICore/AICore.h>
#include <AICore2/AICore.h>
#include <Geometry/Geometry.h>
#include <Esc/Esc.h>
#include <Core2/Core.h>
#include <Vfs/Vfs.h>

#include <plugin/enet/EnetService.h>

#if defined __GNUG__ && (defined flagGCC || defined flagCLANG)
	#include <cxxabi.h>
#endif

NAMESPACE_UPP

#include "Fn.h"
#include "Container.h"
#include "Network.h"
#include "Generated.h"
#include "Types.h"
#include "Util.h"
#include "Machine.h"
#include "Verifier.h"
#include "AtomStore.h"
#include "Atom.h"
#include "Space.h"
#include "FwdTypes.h"
#include "Factory.h"
#include "AtomSystem.h"
#include "Link.h"
#include "LinkUtil.h"
#include "LinkBase.h"
#include "Loop.h"
#include "Audio.h"
#include "Base.h"
#include "BaseAudio.h"
#include "BaseVideo.h"
#include "PacketTracker.h"

#include "SpaceStore.h"
#include "RegistrySystem.h"
#include "LinkStore.h"
#include "LinkSystem.h"
#include "LoopStore.h"
#include "EntitySystem.h"
#include "ShadertoyLoader.h"

#include "EcsComponent.h"
#include "TypeTraits.h"
#include "Entity.h"
#include "EcsPool.h"
#include "EntityStore.h"
#include "Factory.h"
#include "ComponentStore.h"
#include "EntityVisitor.h"
#include "EcsFactory.h"

#include "CommonComponents.h"

#include "EcsCommonComponents.h"
//#include "EcsEngine.h"
#include "WorldLogic.h"
#include "EcsRegistrySystem.h"
#include "EasingSystem.h"
#include "InteractionSystem.h"
#include "Player.h"
#include "EcsPhysicsSystem.h"

#include "ToyLoader.h"
#include "Def.h"
#include "Loader.h"
#include "Ecs.h"
#include "EonLoader.h"
#include "Service.h"

#include "AtomShell.h"

#ifdef flagGUI
#include "FrameT.h"
#include "HandleSystemT.h"
#include "HandleTypes.h"
#include "ScopeT.h"
#endif

END_UPP_NAMESPACE

#endif
