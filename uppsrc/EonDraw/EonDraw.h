#ifndef _EonDraw_EonDraw_h_
#define _EonDraw_EonDraw_h_

#include <Draw/Draw.h>
#include <Eon/Eon.h>

#ifdef flagMSC
	#pragma warning( disable : 4250 ) // C4250: useless Inheritance via dominance warning
#endif

NAMESPACE_UPP

#include "GEnums.h"
#include "DrawCommand.h"
#include "ProgPainter.h"
#include "ProgDraw.h"
#include "Binder.h"
#include "Rendering.h"
#include "Camera.h"
#include "Model.h"
#include "RenderingSystem.h"
#include "EventSystem.h"
#include "Prefab.h"
#include "PaintStrokeSystem.h"
#include "ToolboxSystem.h"
#include "ToolSystem.h"
#include "PaintingSystem.h"
#include "ShootingSystem.h"
#include "ThrowingSystem.h"
#include "Prefabs.h"
#include "DesktopSystem.h"

END_UPP_NAMESPACE

#endif
