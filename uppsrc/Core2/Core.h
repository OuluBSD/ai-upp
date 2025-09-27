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
#include <Core/EcsEngine/EcsEngine.h>
#include <Vfs/Ecs/Ecs.h>


#ifdef flagFREEBSD
extern char **environ;
#endif

NAMESPACE_UPP


#include "Audio.h"

#include "Verifier.h"
#include "Compat.h"
#include "Ctrl.h"
#include "Coordinate.h"
#include "Color.h"
#include "Crypto.h"


END_UPP_NAMESPACE

#endif
