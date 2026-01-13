#ifndef _ports_enet_EnetService_h_
#define _ports_enet_EnetService_h_


// This additional custom code is included in this package to get Init functions called
// when the package is included (usually conditionally).

#include <Core/Core.h>

#ifdef flagWIN32
	#define CY win32_CY_
	#define FAR win32_FAR_
#endif
#include "enet.h"
#ifdef flagWIN32
	#undef CY
	#undef FAR
#endif
#include "Service.h"


#endif
