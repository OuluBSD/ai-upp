#ifndef _Core2_Core_h_
#define _Core2_Core_h_

#include <Core/Core.h>
#include <Core/MetaTraits/MetaTraits.h>
#include <Core/DataStructures/DataStructures.h>
#include <Core/ProcessUtil/ProcessUtil.h>
#include <Core/TextParsing/TextParsing.h>
#include <Core/MathNumeric/MathNumeric.h>
#include <Core/VfsBase/VfsBase.h>
#include <Core/EcsFoundation/EcsFoundation.h>
#include <Vfs/Core/Core.h>
#include <Vfs/Factory/Factory.h>
#include <Vfs/Overlay/Overlay.h>
#include <Vfs/Core/VfsValueExt.h>
#include <Vfs/Factory/VfsFactory.h>
#include <Vfs/Overlay/VfsOverlay.h>
#include <Vfs/Overlay/Precedence.h>


#ifdef flagFREEBSD
extern char **environ;
#endif

NAMESPACE_UPP


// Transitional: legacy VfsValue implementation remains here while new
// Vfs/Core headers are introduced to decouple dependencies.
#include "VfsValue.h"
#include "VfsEnum.h"
#include "Audio.h"


#include "Debugging.h"
#include "Realtime.h"
#include "Component.h"
#include "Exchange.h"
#include "SampleBase.h"
#include "GeomEvent.h"
#include "Samples.h"
#include "Formats.h"
#include "PacketBuffer.h"
#include "ValDevScope.h"
#include "Interface.h"
#include "Entity.h"
#include "Atom.h"
#include "Link.h"
#include "Engine.h"
#include "Engine2.h"
#include "Verifier.h"
#include "PacketTracker.h"
#include "LinkSystem.h"
#include "Util2.h"

#include "Compat.h"
#include "Ctrl.h"
#include "Coordinate.h"
#include "Color.h"
#include "Geom.h"
#include "Crypto.h"


END_UPP_NAMESPACE

#endif
