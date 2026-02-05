#ifndef _Scene3D_Render_Render_h_
#define _Scene3D_Render_Render_h_

#include <CtrlLib/CtrlLib.h>
#include <Scene3D/Core/Core.h>
#include <Scene3D/IO/IO.h>

NAMESPACE_UPP

struct Scene3DRenderConfig {
	Color background_clr;
	bool show_grid = true;
	float grid_major_step = 1.0f;
	float grid_extent = 20.0f;
	int grid_minor_divs = 5;
	Color grid_major_clr;
	Color grid_minor_clr;
	String dump_grid_path;
	bool dump_grid_done = false;
	float mouse_move_sensitivity = 0.01f;

	Scene3DRenderConfig() {
		background_clr = Color(43, 44, 46);
		grid_major_clr = Color(70, 72, 76);
		grid_minor_clr = Color(56, 58, 62);
	}
};

struct Scene3DRenderContext {
	Scene3DRenderConfig* conf = 0;
	GeomWorldState* state = 0;
	GeomAnim* anim = 0;
	GeomVideo* video = 0;
	VfsValue* selected_bone = 0;
};

#include "Renderer.h"
#include "SoftRendCtrl.h"

END_UPP_NAMESPACE

#endif
