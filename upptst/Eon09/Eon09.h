#ifndef _Eon09_Eon09_h_
#define _Eon09_Eon09_h_

#include <Core/Core.h>
#include <Shell/Shell.h>
#include <Scene3D/Core/Core.h>
#include <Scene3D/IO/IO.h>
#include <Scene3D/Render/Render.h>
#include <Scene3D/Exec/Exec.h>

NAMESPACE_UPP

struct Scene3DTestContext {
	Scene3DDocument doc;
	ExecutionManifest manifest;
	VfsValue state_val;
	GeomWorldState* state = nullptr;
	VfsValue anim_val;
	GeomAnim* anim = nullptr;
	ExecScriptRuntime runtime;
	String base_dir;
};

bool LoadScene3DTestProject(const String& manifest_path, Scene3DTestContext& out);
void LoadScene3DTestModels(Scene3DTestContext& ctx);

void Run09aScene3DHeadless(int method);
void Run09bScene3DSdlOgl(int method);
void Run09cScene3DGraphicsOgl(int method);

END_UPP_NAMESPACE

#endif
