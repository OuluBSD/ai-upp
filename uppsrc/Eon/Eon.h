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

#include "Container.h"
#include "Network.h"
#include "Types.h"
#include "Atom.h"
#include "LinkUtil.h"
#include "LinkBase.h"
#include "Base.h"
#include "BaseAudio.h"
#include "BaseVideo.h"
#include "ShadertoyLoader.h"

#include "EcsFactory.h"

#include "CommonComponents.h"

#include "EcsCommonComponents.h"
#include "WorldLogic.h"
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


#ifdef flagGUI
#include "FrameT.h"
#include "HandleSystemT.h"
#include "HandleTypes.h"
#include "ScopeT.h"
#endif

END_UPP_NAMESPACE

#endif
