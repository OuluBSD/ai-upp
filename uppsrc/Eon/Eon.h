#ifndef _Eon_Eon_h_
#define _Eon_Eon_h_

#include <type_traits>
#include <AI/Core/Core.h>
#include <Geometry/Geometry.h>
#include <Esc/Esc.h>
#include <Core2/Core.h>
#include <Vfs/Vfs.h>

#include <plugin/enet/EnetService.h>

#if defined __GNUG__ && (defined flagGCC || defined flagCLANG)
	#include <cxxabi.h>
#endif

NAMESPACE_UPP

#include <Eon/Core/Core.h>
#include <Eon/Script/Script.h>
#include <Eon/Ecs/Ecs.h>
#include <Eon/Interaction/Interaction.h>

#ifdef flagGUI
#include <Eon/GuiGlue/GuiGlue.h>
#endif

END_UPP_NAMESPACE

#endif
