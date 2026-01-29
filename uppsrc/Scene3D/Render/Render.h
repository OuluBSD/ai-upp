#ifndef _Scene3D_Render_Render_h_
#define _Scene3D_Render_Render_h_

#include <CtrlLib/CtrlLib.h>
#include <Scene3D/Core/Core.h>
#include <Scene3D/IO/IO.h>

NAMESPACE_UPP

struct Scene3DRenderConfig {
	Color background_clr;
	float mouse_move_sensitivity = 0.01f;

	Scene3DRenderConfig() {
		background_clr = Color(43, 44, 46);
	}
};

struct Scene3DRenderContext {
	Scene3DRenderConfig* conf = 0;
	GeomWorldState* state = 0;
	GeomAnim* anim = 0;
	GeomVideo* video = 0;
};

#include "Renderer.h"
#include "SoftRendCtrl.h"

END_UPP_NAMESPACE

#endif
