#include "Eon09.h"
#include <api/Graphics/Graphics.h>

namespace Upp {

namespace {

bool FindSceneSkybox(Scene3DTestContext& ctx, GeomSkybox& out) {
	GeomScene& scene = ctx.state->GetActiveScene();
	GeomObjectCollection iter(scene);
	for (GeomObject& obj : iter) {
		GeomSkybox* sky = obj.FindSkybox();
		if (!sky)
			continue;
		out = *sky;
		return true;
	}
	return false;
}

String ResolveAssetPath(Scene3DTestContext& ctx, const String& ref) {
	if (ref.IsEmpty())
		return String();
	String path = ctx.runtime.ResolvePath(ref);
	if (path.IsEmpty())
		path = ref;
	if (!IsFullPath(path)) {
		if (!FileExists(path))
			path = ShareDirFile(path);
	}
	return FileExists(path) ? path : String();
}

String EscapeEonString(const String& s) {
	String out;
	out.Reserve(s.GetCount() + 8);
	for (int i = 0; i < s.GetCount(); i++) {
		char c = s[i];
		if (c == '\\')
			out.Cat("\\\\");
		else if (c == '"')
			out.Cat("\\\"");
		else
			out.Cat(c);
	}
	return out;
}

String BuildEonScript(const GeomSkybox* skybox) {
	String script;
	script << "machine sdl.app:\n";
	script << "\tdriver context:\n";
	script << "\t\tsdl.context\n\n";
	script << "\tchain program:\n";
	script << "\t\tstate event.register\n\n";
	script << "\t\tloop ogl.fbo:\n";
	script << "\t\t\togl.customer\n";
	script << "\t\t\tsdl.ogl.fbo.program:\n";
	script << "\t\t\t\tdrawmem = \"false\"\n";
	script << "\t\t\t\tprogram = \"obj_view_program\"\n";
	if (skybox && skybox->enabled) {
		if (!skybox->display_ref.IsEmpty())
			script << "\t\t\t\tskybox.display = \"" << EscapeEonString(skybox->display_ref) << "\"\n";
		if (!skybox->specular_ref.IsEmpty())
			script << "\t\t\t\tskybox.specular = \"" << EscapeEonString(skybox->specular_ref) << "\"\n";
		if (!skybox->irradiance_ref.IsEmpty())
			script << "\t\t\t\tskybox.irradiance = \"" << EscapeEonString(skybox->irradiance_ref) << "\"\n";
	}
	script << "\t\t\tsdl.ogl.fbo.sink:\n";
	script << "\t\t\t\tclose_machine = true\n";
	script << "\t\t\t\tsizeable = true\n";
	script << "\t\t\t\tenv = event.register\n";
	script << "\t\t\t\trecv.data = true\n";
	return script;
}

struct Scene3DGfxContent : RendererContent {
	Scene3DTestContext* ctx = nullptr;
	VectorMap<hash_t, hash_t> model_ids;
	VectorMap<hash_t, hash_t> skybox_ids;
	ModelLoader loader;

	void Reset() {
		model_ids.Clear();
		skybox_ids.Clear();
		loader.Clear();
	}

	virtual bool Load(GfxDataState& state) override {
		if (!ctx || !ctx->state || !ctx->state->HasActiveScene())
			return true;

		ctx->state->UpdateObjects();

		Size sz((int)state.resolution[0], (int)state.resolution[1]);
		if (sz.cx <= 0 || sz.cy <= 0)
			sz = Size(640, 480);

		GeomCamera& cam_src = ctx->state->GetProgram();
		Camera cam;
		cam_src.LoadCamera(VIEWMODE_PERSPECTIVE, cam, sz);
		cam.SetResolution(sz);
		state.user_view = true;
		state.view = cam.GetViewportMatrix() * cam.GetViewMatrix();
		state.camera_pos = cam_src.position;
		state.camera_dir = VectorTransform(VEC_FWD, cam_src.orientation);

		vec3 light_dir(0.4f, 0.7f, 0.5f);
		GeomScene& scene = ctx->state->GetActiveScene();
		GeomObjectCollection iter(scene);
		for (GeomObject& obj : iter) {
			GeomLight* light = obj.FindLight();
			if (!light || light->type != GeomLight::L_DIRECTIONAL)
				continue;
			light_dir = light->direction;
			break;
		}
		if (light_dir.GetLength() > 0)
			light_dir.Normalize();
		state.light_dir = light_dir;

		GeomSkybox skybox;
		bool have_skybox = FindSceneSkybox(*ctx, skybox);
		if (have_skybox && state.env_material_model == 0) {
			GfxModelState& mdl = state.AddModel();
			hash_t mdl_id = mdl.id;
			ModelBuilder mb;
			Mesh& box_mesh = mb.AddBox(vec3(0), vec3(1e8), true, true);
			Model& box = mb;
			String display_map = ResolveAssetPath(*ctx, skybox.display_ref);
			String specular_map = ResolveAssetPath(*ctx, skybox.specular_ref);
			String irradiance_map = ResolveAssetPath(*ctx, skybox.irradiance_ref);
			if (!display_map.IsEmpty() && !box.LoadCubemapFile(box_mesh, TEXTYPE_CUBE_DISPLAY, display_map))
				LOG("Scene3DGfxContent: failed to load skybox display map");
			if (!specular_map.IsEmpty() && !box.LoadCubemapFile(box_mesh, TEXTYPE_CUBE_DIFFUSE, specular_map))
				LOG("Scene3DGfxContent: failed to load skybox specular map");
			if (!irradiance_map.IsEmpty() && !box.LoadCubemapFile(box_mesh, TEXTYPE_CUBE_IRRADIANCE, irradiance_map))
				LOG("Scene3DGfxContent: failed to load skybox irradiance map");
			loader = mb;
			if (!mdl.LoadModel(loader))
				LOG("Scene3DGfxContent: failed to load skybox model");
			mdl.env_material = mdl.GetMaterialKey(0);
			mdl.env_material_model = mdl.id;
			mdl.SetProgram("sky");
			state.env_material_model = mdl_id;
		}

		for (const GeomObjectState& os : ctx->state->objs) {
			GeomObject* obj = os.obj;
			if (!obj || !obj->is_visible || !obj->IsModel())
				continue;
			if (!obj->mdl) {
				String resolved = ResolveAssetPath(*ctx, obj->asset_ref);
				if (!resolved.IsEmpty() && loader.LoadModel(resolved))
					obj->mdl = pick(loader.model);
			}
			if (!obj->mdl)
				continue;

			hash_t id = model_ids.Find(obj->key) >= 0 ? model_ids.Get(obj->key) : 0;
			if (id == 0) {
				GfxModelState& mdl = state.AddModel();
				id = mdl.id;
				model_ids.Add(obj->key, id);
				mdl.LoadModel(*obj->mdl);
				mdl.SetProgram("default");
			}

			GfxModelState& mdl_state = state.GetModel(id);
			mat4 model_mat = Translate(os.position) * QuatMat(os.orientation) * Scale(os.scale);
			int obj_count = mdl_state.GetObjectCount();
			for (int i = 0; i < obj_count; i++) {
				GfxDataObject& dobj = mdl_state.GetObject(i);
				dobj.is_visible = true;
				dobj.model = model_mat;
				dobj.color = vec4(1, 1, 1, 1);
			}
		}

		return true;
	}
};

struct EngineGuard {
	Engine* eng = nullptr;
	~EngineGuard() {
		if (eng)
			Engine::Uninstall(true, eng);
	}
};

void ConfigureEngine(Engine& eng, const String& script, Scene3DGfxContent& content, Scene3DTestContext& ctx) {
	eng.ClearCallbacks();
	eng.WhenInitialize << callback(MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	eng.WhenUserInitialize << [=, &eng, &content, &ctx] {
		content.ctx = &ctx;
		RendererContent::AddContent(&content);
		Eon::PostLoadString(eng, script);
	};
	eng.WhenShutdown << [&content] {
		RendererContent::RemoveContent(&content);
		content.ctx = nullptr;
	};
}

void RunScriptWithEngine(const String& script, Scene3DGfxContent& content, Scene3DTestContext& ctx) {
	EngineGuard guard;
	guard.eng = &ShellMainEngine();
	Engine& eng = *guard.eng;

	ConfigureEngine(eng, script, content, ctx);

	ValueMap args;
	args.Add("MACHINE_TIME_LIMIT", 3);
	if (!eng.StartLoad("Shell", String(), args))
		throw Exc("Run09cScene3DGraphicsOgl: engine failed to start");

	eng.MainLoop();
	Engine::Uninstall(true, guard.eng);
	guard.eng = nullptr;
}

}

void Run09cScene3DGraphicsOgl(int method) {
	String manifest_path = ShareDirFile("scene3d/projects/driving_softphys/project.exec.json");
	Scene3DTestContext ctx;
	if (!LoadScene3DTestProject(manifest_path, ctx))
		throw Exc("Run09cScene3DGraphicsOgl: load failed");
	LoadScene3DTestModels(ctx);

	GeomSkybox skybox;
	bool have_skybox = FindSceneSkybox(ctx, skybox);

	String script = BuildEonScript(have_skybox ? &skybox : nullptr);
	Scene3DGfxContent content;
	RunScriptWithEngine(script, content, ctx);
}

} // namespace Upp
