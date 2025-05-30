#ifndef _Core2_Core_h_
#define _Core2_Core_h_

#include <Core/Core.h>

#ifdef flagFREEBSD
extern char **environ;
#endif

NAMESPACE_UPP

#include "Math.h"

#include "VFS.h"
#include "Mount.h"
#include "VCS.h"
#include "Visitor.h"
#include "AtomType.h"
#include "LinkType.h"
#include "WorldState.h"
#include "VfsValue.h"
#include "RTuple.h"
#include "TypeTraits.h"
#include "TypeTraits2.h"
#include "Audio.h"

#include "GEnums.h"

#include "EcsDefs.h"
#include "Debugging.h"
#include "Realtime.h"
#include "Component.h"
#include "Exchange.h"
#include "SampleBase.h"
#include "Keys.h"
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
#include "String.h"
#include "Ctrl.h"
#include "Util.h"
#include "Process.h"
#include "Tokenizer.h"
#include "Container.h"
#include "Index.h"
#include "DCT.h"
#include "Coordinate.h"
#include "Chrono.h"
#include "Record.h"
#include "Color.h"
#include "Geom.h"
#include "Html.h"
#include "Url.h"
#include "Crypto.h"
#include "TokenParser.h"
#include "Random.h"
#include "CKMeans.h"


END_UPP_NAMESPACE

#endif
