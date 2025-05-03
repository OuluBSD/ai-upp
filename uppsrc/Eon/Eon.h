#ifndef _Eon_Eon_h_
#define _Eon_Eon_h_

#include <type_traits>
#include <AICore/AICore.h>
#include <Geometry/Geometry.h>
#include <Esc/Esc.h>

#if defined __GNUG__ && (defined flagGCC || defined flagCLANG)
	#include <cxxabi.h>
#endif

NAMESPACE_UPP

#include "Defs.h"
#include "Fn.h"
#include "CtrlEvent.h"
#include "Container.h"
#include "Process.h"
#include "Debugging.h"
#include "WorldState.h"
#include "SampleBase.h"
#include "Samples.h"
#include "Generated.h"
#include "Types.h"
#include "Formats.h"
#include "Exchange.h"
#include "PacketBuffer.h"
#include "Util.h"
#include "Machine.h"
#include "Verifier.h"
#include "DefaultFormat.h"
#include "Realtime.h"
#include "ValDevScope.h"
#include "Interface.h"
#include "AtomStore.h"
#include "Atom.h"
#include "Space.h"
#include "SpaceStore.h"
#include "Factory.h"
#include "AtomSystem.h"
#include "FwdTypes.h"
#include "Link.h"
#include "LinkUtil.h"
#include "LinkBase.h"
#include "Loop.h"
#include "Audio.h"
#include "Base.h"
#include "BaseAudio.h"
#include "BaseVideo.h"
#include "PacketTracker.h"

#include "Factory.h"
#include "SpaceStore.h"
#include "RegistrySystem.h"
#include "LinkStore.h"
#include "LinkSystem.h"
#include "LinkFactory.h"
#include "LoopStore.h"
#include "EntitySystem.h"
#include "AtomShell.h"
#include "Rendering.h"
#include "ModelCache.h"
#include "ShadertoyLoader.h"

#include "EntityStore.h"
#include "EcsComponent.h"
#include "Factory.h"
#include "Entity.h"
#include "ComponentStore.h"
#include "EcsPool.h"
#include "EntityVisitor.h"

#include "CommonComponents.h"
#include "Camera.h"
#include "EcsRegistrySystem.h"
#include "Model.h"
#include "RenderingSystem.h"
#include "EventSystem.h"
#include "Prefab.h"
#include "EonLoader.h"
#include "WorldLogic.h"

#include "EcsCommonComponents.h"
#include "EasingSystem.h"
#include "PaintStrokeSystem.h"
#include "InteractionSystem.h"
#include "ToolboxSystem.h"
#include "ToolSystem.h"
#include "Player.h"
#include "EcsPhysicsSystem.h"
#include "PaintingSystem.h"
#include "ShootingSystem.h"
#include "ThrowingSystem.h"
#include "Prefabs.h"
#include "DesktopSystem.h"

#include "ToyLoader.h"
#include "Def.h"
#include "Loader.h"
#include "Ecs.h"

END_UPP_NAMESPACE

#endif
