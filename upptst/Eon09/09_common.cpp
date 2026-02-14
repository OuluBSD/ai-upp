#include "Eon09.h"

namespace Upp {

namespace {

void FixCamera(GeomCamera& cam) {
	auto fixf = [&](float& v, float def) {
		if (IsNull(v))
			v = def;
	};
	auto fixv3 = [&](vec3& v, const vec3& def) {
		for (int i = 0; i < 3; i++)
			if (IsNull(v[i]))
				v[i] = def[i];
	};
	auto fixq = [&](quat& q) {
		bool bad = false;
		for (int i = 0; i < 4; i++)
			if (IsNull(q[i]))
				bad = true;
		if (bad)
			q = Identity<quat>();
	};
	fixv3(cam.position, vec3(0));
	fixq(cam.orientation);
	fixf(cam.scale, 1.0f);
	fixf(cam.fov, 120.0f);
	if (cam.fov <= 0.1f)
		cam.fov = 120.0f;
	if (cam.scale == 0)
		cam.scale = 1.0f;
	bool is_identity = fabs(cam.orientation[0]) < 1e-4f &&
	                   fabs(cam.orientation[1]) < 1e-4f &&
	                   fabs(cam.orientation[2]) < 1e-4f &&
	                   fabs(cam.orientation[3] - 1.0f) < 1e-4f;
	if (is_identity && cam.position.GetLength() > 0.1f) {
		mat4 view = LookAt(cam.position, vec3(0, 0, 0), vec3(0, 1, 0));
		mat4 world = view.GetInverse();
		cam.orientation = MatQuat(world);
	}
}

}

bool LoadScene3DTestProject(const String& manifest_path, Scene3DTestContext& out) {
	String path = manifest_path;
	if (path.IsEmpty())
		return false;
	out.state = &out.state_val.CreateExt<GeomWorldState>();
	out.anim = &out.anim_val.CreateExt<GeomAnim>();
	if (!LoadExecutionManifest(path, out.manifest)) {
		Cout() << "Eon09: failed to load execution manifest.\n";
		return false;
	}
	out.base_dir = GetFileFolder(path);
	String scene_path = out.manifest.scene3d;
	if (!IsFullPath(scene_path))
		scene_path = AppendFileName(out.base_dir, scene_path);
	bool loaded_scene = LoadScene3DJson(scene_path, out.doc);
	if (!loaded_scene)
		loaded_scene = LoadScene3DBin(scene_path, out.doc);
	if (!loaded_scene) {
		Cout() << "Eon09: failed to load scene3d.\n";
		return false;
	}
	if (!out.doc.project) {
		Cout() << "Eon09: scene3d document missing project.\n";
		return false;
	}
	out.state->prj = out.doc.project;
	out.state->active_scene = out.doc.active_scene;
	if (out.state->active_scene < 0 && out.state->prj->GetSceneCount() > 0)
		out.state->active_scene = 0;
	if (out.doc.focus)
		VisitCopy(*out.doc.focus, out.state->GetFocus());
	if (out.doc.program)
		VisitCopy(*out.doc.program, out.state->GetProgram());
	FixCamera(out.state->GetFocus());
	FixCamera(out.state->GetProgram());
	{
		GeomCamera& program = out.state->GetProgram();
		if (program.position.GetLength() < 0.001f) {
			program.position = vec3(0, 5, 14);
			mat4 view = LookAt(program.position, vec3(0, 0, 5), vec3(0, 1, 0));
			mat4 world = view.GetInverse();
			program.orientation = MatQuat(world);
		}
	}
	out.state->UpdateObjects();
	out.anim->state = out.state;
	out.anim->Reset();
	out.anim->Play();
	out.runtime.Init(out.state, out.anim);
	out.runtime.SetManifest(out.manifest, out.base_dir);
	out.runtime.ReloadScripts(true);
	return true;
}

void LoadScene3DTestModels(Scene3DTestContext& ctx) {
	for (GeomScene& scene : ctx.doc.project->val.Sub<GeomScene>()) {
		GeomObjectCollection objects(scene);
		for (GeomObject& obj : objects) {
			if (!obj.IsModel())
				continue;
			if (obj.mdl)
				continue;
			if (obj.asset_ref.IsEmpty()) {
				ModelBuilder mb;
				mb.AddBox(0, 1, 1);
				obj.mdl = mb.Detach();
				continue;
			}
			String resolved = ctx.runtime.ResolvePath(obj.asset_ref);
			ModelLoader loader;
			if (!resolved.IsEmpty() && loader.LoadModel(resolved))
				obj.mdl = pick(loader.model);
			else {
				ModelBuilder mb;
				mb.AddBox(0, 1, 1);
				obj.mdl = mb.Detach();
			}
		}
	}
	ctx.state->UpdateObjects();
}

} // namespace Upp
