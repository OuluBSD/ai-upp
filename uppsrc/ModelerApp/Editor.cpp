#include "ModelerApp.h"
#include <ByteVM/PyBindings.h>

#define IMAGECLASS ImagesImg
#define IMAGEFILE <ModelerApp/Images.iml>
#include <Draw/iml_source.h>

namespace Upp { class PyVM; }
void RegisterGeometry(Upp::PyVM& vm);

NAMESPACE_UPP

namespace {

void FillOctree(Octree& octree, const Vector<vec3>& points, int scale_level) {
	octree.Initialize(-3, 8);
	for (const vec3& p : points) {
		OctreeNode* node = octree.GetAddNode(p, scale_level);
		if (!node)
			continue;
		Pointcloud::Point& pt = node->Add<Pointcloud::Point>();
		pt.SetPosition(p);
	}
}

Vector<vec3> TransformPointsToWorld(const PointcloudPose& pose, const Vector<vec3>& points) {
	Vector<vec3> out;
	out.SetCount(points.GetCount());
	for (int i = 0; i < points.GetCount(); i++)
		out[i] = VectorTransform(points[i], pose.orientation) + pose.position;
	return out;
}

void UpdateCameraObject(GeomObject& cam, const PointcloudPose& pose) {
	GeomTimeline& tl = cam.GetTimeline();
	tl.keypoints.Clear();
	GeomKeypoint& kp = tl.keypoints.Add(0);
	kp.position = pose.position;
	kp.orientation = pose.orientation;
	if (GeomTransform* tr = cam.FindTransform()) {
		tr->position = pose.position;
		tr->orientation = pose.orientation;
	}
}

void UpdateCameraObjectRender(GeomObject& cam, const PointcloudPose& pose, bool flip_z) {
	PointcloudPose p = pose;
	if (flip_z) {
		p.orientation = p.orientation * MatQuat(YRotation(M_PI));
	}
	UpdateCameraObject(cam, p);
}

bool CompilePySource(const String& code, const String& filename, Vector<PyIR>& out_ir, String& err) {
	Tokenizer tk;
	tk.SkipPythonComments();
	if (!tk.Process(code, filename.IsEmpty() ? "<script>" : filename)) {
		err = "Tokenize failed";
		return false;
	}
	tk.NewlineToEndStatement();
	tk.CombineTokens();
	PyCompiler compiler(tk.GetTokens());
	try {
		compiler.Compile(out_ir);
	} catch (Exc& e) {
		err = e;
		return false;
	}
	return true;
}

bool RunPyIR(PyVM& vm, const Vector<PyIR>& ir, String& err) {
	Vector<PyIR> run = ir;
	vm.SetIR(run);
	try {
		vm.Run();
	} catch (Exc& e) {
		err = e;
		return false;
	}
	return true;
}

Vector<String> SplitPathParts(const String& path) {
	Vector<String> parts;
	Vector<String> raw = Split(path, '/');
	for (const String& part : raw) {
		if (!part.IsEmpty())
			parts.Add(part);
	}
	return parts;
}

GeomDirectory* FindDirectoryByName(GeomDirectory& dir, const String& name) {
	for (auto& s : dir.val.sub) {
		if (!IsVfsType(s, AsTypeHash<GeomDirectory>()))
			continue;
		GeomDirectory& sub = s.GetExt<GeomDirectory>();
		if (sub.name == name || sub.val.id == name)
			return &sub;
	}
	return 0;
}

GeomDirectory* ResolveDirectoryPath(Edit3D& e, const String& path, bool create) {
	GeomScene& scene = e.GetActiveScene();
	Vector<String> parts = SplitPathParts(path);
	if (parts.IsEmpty())
		return &scene;
	if (parts[0] == scene.name || parts[0] == scene.val.id)
		parts.Remove(0);
	GeomDirectory* dir = &scene;
	for (int i = 0; i < parts.GetCount(); i++) {
		GeomDirectory* next = FindDirectoryByName(*dir, parts[i]);
		if (!next && create)
			next = &dir->GetAddDirectory(parts[i]);
		if (!next)
			return 0;
		dir = next;
	}
	return dir;
}

GeomObject* ResolveObjectPath(Edit3D& e, const String& path) {
	Vector<String> parts = SplitPathParts(path);
	if (parts.IsEmpty())
		return 0;
	GeomScene& scene = e.GetActiveScene();
	if (parts[0] == scene.name || parts[0] == scene.val.id)
		parts.Remove(0);
	if (parts.IsEmpty())
		return 0;
	String obj_name = parts.Pop();
	GeomDirectory* dir = ResolveDirectoryPath(e, Join(parts, "/"), false);
	if (!dir)
		return 0;
	return dir->FindObject(obj_name);
}

PyValue ModelerLog(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() > 0)
		LOG(args[0].ToString());
	return PyValue::None();
}

PyValue ModelerDebugGeneratePointcloud(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugGeneratePointcloud();
	return PyValue::None();
}

PyValue ModelerDebugSimulateObservation(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugSimulateObservation();
	return PyValue::None();
}

PyValue ModelerDebugRunLocalization(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugRunLocalization();
	return PyValue::None();
}

PyValue ModelerDebugSimulateControllerObservations(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugSimulateControllerObservations();
	return PyValue::None();
}

PyValue ModelerDebugRunControllerLocalization(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugRunControllerLocalization();
	return PyValue::None();
}

PyValue ModelerDebugClearSynthetic(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->DebugClearSynthetic();
	return PyValue::None();
}

PyValue ModelerDebugRunFullSynthetic(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (e)
		e->RunSyntheticPointcloudSimDialog();
	return PyValue::None();
}

PyValue ModelerSetPosition(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e || args.GetCount() < 4)
		return PyValue::False();
	String path = args[0].ToString();
	GeomObject* obj = ResolveObjectPath(*e, path);
	if (!obj)
		return PyValue::False();
	GeomTransform& tr = obj->GetTransform();
	tr.position = vec3((float)args[1].AsDouble(),
	                   (float)args[2].AsDouble(),
	                   (float)args[3].AsDouble());
	e->state->UpdateObjects();
	e->v0.RefreshAll();
	return PyValue::True();
}

PyValue ModelerSetOrientation(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e || args.GetCount() < 5)
		return PyValue::False();
	String path = args[0].ToString();
	GeomObject* obj = ResolveObjectPath(*e, path);
	if (!obj)
		return PyValue::False();
	quat q((float)args[1].AsDouble(),
	       (float)args[2].AsDouble(),
	       (float)args[3].AsDouble(),
	       (float)args[4].AsDouble());
	q.Normalize();
	GeomTransform& tr = obj->GetTransform();
	tr.orientation = q;
	e->state->UpdateObjects();
	e->v0.RefreshAll();
	return PyValue::True();
}

PyValue ModelerGetPosition(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e || args.GetCount() < 1)
		return PyValue::None();
	String path = args[0].ToString();
	GeomObject* obj = ResolveObjectPath(*e, path);
	if (!obj)
		return PyValue::None();
	vec3 pos = vec3(0);
	if (GeomTransform* tr = obj->FindTransform()) {
		pos = tr->position;
	}
	else if (const GeomObjectState* os = e->state->FindObjectStateByKey(obj->key)) {
		pos = os->position;
	}
	Vector<PyValue> out;
	out.Add(pos[0]);
	out.Add(pos[1]);
	out.Add(pos[2]);
	return PyValue::FromVector(out, true);
}

PyValue ModelerCreateObject(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e || args.GetCount() < 2)
		return PyValue::False();
	String path = args[0].ToString();
	String type = ToLower(args[1].ToString());
	Vector<String> parts = SplitPathParts(path);
	if (parts.IsEmpty())
		return PyValue::False();
	GeomScene& scene = e->GetActiveScene();
	if (parts[0] == scene.name || parts[0] == scene.val.id)
		parts.Remove(0);
	if (parts.IsEmpty())
		return PyValue::False();
	String obj_name = parts.Pop();
	GeomDirectory* dir = ResolveDirectoryPath(*e, Join(parts, "/"), true);
	if (!dir)
		return PyValue::False();
	GeomObject* obj = 0;
	if (type == "camera")
		obj = &dir->GetAddCamera(obj_name);
	else if (type == "model")
		obj = &dir->GetAddModel(obj_name);
	else if (type == "pointcloud" || type == "octree")
		obj = &dir->GetAddOctree(obj_name);
	if (!obj)
		return PyValue::False();
	e->state->UpdateObjects();
	e->RefreshData();
	return PyValue::True();
}

PyValue ModelerCreateDirectory(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e || args.GetCount() < 1)
		return PyValue::False();
	String path = args[0].ToString();
	GeomDirectory* dir = ResolveDirectoryPath(*e, path, true);
	if (!dir)
		return PyValue::False();
	e->state->UpdateObjects();
	e->RefreshData();
	return PyValue::True();
}

PyValue ModelerGetProjectDir(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e)
		return PyValue::None();
	return PyValue(e->GetProjectDir());
}

vec3 ApplyInversePoseSimple(const PointcloudPose& pose, const vec3& p) {
	quat inv = pose.orientation.GetInverse();
	return VectorTransform(p - pose.position, inv);
}

bool VisibleInFrustumSimple(const vec3& cam, const SyntheticPointcloudConfig& cfg) {
	float z = cam[2] * SCALAR_FWD_Zf;
	if (z < cfg.hmd_min_dist || z > cfg.max_range)
		return false;
	float half = (cfg.hmd_fov_deg * 0.5f) * (float)M_PI / 180.0f;
	float limit = tan(half) * z;
	if (fabs(cam[0]) > limit || fabs(cam[1]) > limit)
		return false;
	return true;
}

float RandRange(float a, float b) {
	return a + (b - a) * (float)Randomf();
}

PointcloudPose RandomHmdPose(const SyntheticPointcloudConfig& cfg) {
	PointcloudPose pose;
	pose.position[0] = RandRange(cfg.bounds_min[0], cfg.bounds_max[0]);
	pose.position[1] = RandRange(cfg.bounds_min[1], cfg.bounds_max[1]);
	pose.position[2] = RandRange(cfg.bounds_min[2], cfg.bounds_max[2]);
	vec3 to_origin = -pose.position;
	if (to_origin.GetLength() < 1e-3f)
		to_origin = VEC_FWD;
	vec3 axes = GetDirAxes(to_origin);
	float yaw = axes[0] + RandRange(-0.2f, 0.2f);
	float pitch = axes[1] + RandRange(-0.15f, 0.15f);
	float roll = RandRange(-M_PIf * 0.05f, M_PIf * 0.05f);
	pose.orientation = AxesQuat(yaw, pitch, roll);
	return pose;
}

PointcloudPose RandomControllerPoseInFrustum(const PointcloudPose& hmd_pose,
                                             const SyntheticPointcloudConfig& cfg,
                                             float min_dist,
                                             float max_dist) {
	PointcloudPose pose;
	float dist = RandRange(min_dist, max_dist);
	float half = (cfg.hmd_fov_deg * 0.5f) * (float)M_PI / 180.0f;
	float limit = tan(half) * dist;
	float horiz = RandRange(-limit * 0.8f, limit * 0.8f);
	float vert = RandRange(-limit * 0.6f, limit * 0.6f);
	vec3 fwd = VectorTransform(VEC_FWD, hmd_pose.orientation);
	vec3 right = VectorTransform(VEC_RIGHT, hmd_pose.orientation);
	vec3 up = VectorTransform(VEC_UP, hmd_pose.orientation);
	pose.position = hmd_pose.position + fwd * dist + right * horiz + up * vert;
	float yaw = RandRange(-M_PIf, M_PIf);
	float pitch = RandRange(-M_PIf * 0.25f, M_PIf * 0.25f);
	float roll = RandRange(-M_PIf * 0.1f, M_PIf * 0.1f);
	pose.orientation = AxesQuat(yaw, pitch, roll);
	return pose;
}

}

FilePoolCtrl::FilePoolCtrl(Edit3D* e) {
	owner = e;
	Title("Scene3D File Pool");
	Sizeable().MaximizeBox();
	Add(files.SizePos());
	files.AddColumn("Path");
	files.AddColumn("Type");
	files.AddColumn("Info");
	files.AddColumn("Usage");
	files.AddColumn("Size");
	files.AddColumn("Modified");
	files.EvenRowColor();
	files.SetLineCy(EditField::GetStdHeight());
}

void FilePoolCtrl::Data() {
	files.Clear();
	if (!owner || !owner->prj)
		return;
	
	VectorMap<String, int> usage;
	for (GeomScene& scene : owner->prj->val.Sub<GeomScene>()) {
		GeomObjectCollection objs(scene);
		for (GeomObject& o : objs) {
			if (!o.asset_ref.IsEmpty())
				usage.GetAdd(o.asset_ref, 0)++;
			if (!o.pointcloud_ref.IsEmpty())
				usage.GetAdd(o.pointcloud_ref, 0)++;
		}
	}
	
	const Array<Scene3DExternalFile>& files_list = owner->scene3d_external_files;
	files.SetCount(files_list.GetCount());
	for (int i = 0; i < files_list.GetCount(); i++) {
		const Scene3DExternalFile& f = files_list[i];
		String info = f.note;
		if (info.IsEmpty())
			info = f.id;
		String size = f.size >= 0 ? FormatInt64(f.size) : String();
		String usage_str;
		int usage_count = usage.Find(f.path) >= 0 ? usage.Get(f.path) : 0;
		if (usage_count > 0)
			usage_str = IntStr(usage_count);
		files.Set(i, 0, f.path);
		files.Set(i, 1, f.type);
		files.Set(i, 2, info);
		files.Set(i, 3, usage_str);
		files.Set(i, 4, size);
		files.Set(i, 5, f.modified_utc);
	}
}

ScriptEditorDlg::ScriptEditorDlg() {
	Title("Script Editor");
	Sizeable().Zoomable();
	AddFrame(tool);
	Add(editor.SizePos());
	editor.WhenAction << THISBACK(OnChange);
	tool.Set([=](Bar& bar) {
		bar.Add(t_("Save"), THISBACK(Save));
		bar.Add(t_("Reload"), [=] { if (!path.IsEmpty()) OpenFile(path); });
		bar.Add(t_("Close"), [=] { Close(); });
	});
}

void ScriptEditorDlg::OpenFile(const String& p) {
	path = p;
	String data;
	if (FileExists(path))
		data = LoadFile(path);
	editor.Set(data);
	editor.SetFocus();
	dirty = false;
	SetTitle("Script Editor - " + GetFileName(path));
}

void ScriptEditorDlg::Save() {
	if (path.IsEmpty())
		return;
	SaveFile(path, editor.Get());
	dirty = false;
}

void ScriptEditorDlg::SaveAs(const String& p) {
	path = p;
	Save();
}

void ScriptEditorDlg::OnChange() {
	dirty = true;
}


bool HmdCapture::Start() {
	if (running)
		return true;
	if (!sys.Initialise())
		return false;
	source = CreateStereoSource("hmd");
	if (source.IsEmpty() || !source->Start()) {
		source.Clear();
		sys.Uninitialise();
		return false;
	}
	ResetTracking();
	running = true;
	return true;
}

void HmdCapture::Stop() {
	if (!running)
		return;
	recording = false;
	if (source)
		source->Stop();
	source.Clear();
	sys.Uninitialise();
	running = false;
	bright = Image();
	dark = Image();
	bright_serial = -1;
	dark_serial = -1;
}

void HmdCapture::ResetTracking() {
	fusion.Reset();
	fusion.GetBrightTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
	fusion.GetDarkTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
}

const Octree* HmdCapture::GetPointcloud(bool use_bright) const {
	if (use_bright)
		return const_cast<HMD::SoftHmdFusion&>(fusion).GetBrightTracker().GetPointcloud();
	return const_cast<HMD::SoftHmdFusion&>(fusion).GetDarkTracker().GetPointcloud();
}

void HmdCapture::Poll() {
	if (!running)
		return;
	sys.UpdateData();
	if(sys.hmd) {
		ImuSample imu;
		imu.timestamp_us = usecs();
		bool has_imu = false;
		if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_ACCELEROMETER_VECTOR, imu.accel.data) == HMD::HMD_S_OK)
			has_imu = true;
		if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_GYROSCOPE_VECTOR, imu.gyro.data) == HMD::HMD_S_OK)
			has_imu = true;
		if(has_imu)
			fusion.PutImu(imu);
	}
	if (!source || !source->IsRunning())
		return;
	CameraFrame lf, rf;
	if (!source->ReadFrame(lf, rf, false))
		return;
	Image combined;
	if (!JoinStereoImage(lf.img, rf.img, combined))
		return;
	if (lf.is_bright) {
		bright = combined;
		bright_serial = lf.serial;
	} else {
		dark = combined;
		dark_serial = lf.serial;
	}
	if (!recording)
		return;
	VisualFrame vf;
	vf.timestamp_us = usecs();
	vf.format = GEOM_EVENT_CAM_RGBA8;
	vf.width = combined.GetWidth();
	vf.height = combined.GetHeight();
	vf.stride = vf.width * (int)sizeof(RGBA);
	vf.img = combined;
	vf.data = 0;
	vf.data_bytes = combined.GetLength() * (int)sizeof(RGBA);
	vf.flags = lf.is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
	fusion.PutVisual(vf);
}



Edit3D::Edit3D() :
	v0(this),
	v1(this),
	file_pool(this)
{
	prj = &prj_val.CreateExt<GeomProject>();
	state = &state_val.CreateExt<GeomWorldState>();
	anim = &anim_val.CreateExt<GeomAnim>();
	state->prj = prj;
	anim->state = state;
	video.anim = anim;
	render_ctx.conf = &conf;
	render_ctx.state = state;
	render_ctx.anim = anim;
	render_ctx.video = &video;
	
	anim->WhenSceneEnd << THISBACK(OnSceneEnd);
	
	Sizeable().MaximizeBox();
	Title("Edit3D");
	scene3d_data_dir = "data";
	SetProjectDir(GetCurrentDirectory());
	
	SetView(VIEW_GEOMPROJECT);
	Add(v0.hsplit.SizePos());
	
	AddFrame(menu);
	menu.Set([this](Bar& bar) {
		bar.Sub(t_("File"), [this](Bar& bar) {
			bar.Add(t_("New"), THISBACK(LoadEmptyProject)).Key(K_CTRL|K_N);
			bar.Add(t_("Open..."), THISBACK(OpenScene3D)).Key(K_CTRL|K_O);
			bar.Add(t_("Save"), THISBACK(SaveScene3DInteractive)).Key(K_CTRL|K_S);
			bar.Add(t_("Save As..."), THISBACK(SaveScene3DAs)).Key(K_CTRL|K_SHIFT|K_S);
			bar.Add(t_("Save as JSON..."), THISBACK(SaveScene3DAsJson));
			bar.Add(t_("Save as Binary..."), THISBACK(SaveScene3DAsBinary));
			bar.Separator();
			bar.Sub(t_("Format"), [this](Bar& bar) {
				bar.Add(t_("JSON (default)"), THISBACK1(SetScene3DFormat, true))
					.Check(scene3d_use_json);
				bar.Add(t_("Binary (default)"), THISBACK1(SetScene3DFormat, false))
					.Check(!scene3d_use_json);
			});
			bar.Separator();
			bar.Add(t_("Exit"), THISBACK(Exit));
		});
		bar.Sub(t_("View"), [this](Bar& bar) {
			bar.Add(t_("Geometry"), THISBACK1(SetView, VIEW_GEOMPROJECT)).Key(K_ALT|K_1);
			bar.Add(t_("Video import"), THISBACK1(SetView, VIEW_VIDEOIMPORT)).Key(K_ALT|K_2);
			bar.Separator();
			bar.Add(t_("File Pool"), THISBACK(OpenFilePool));
		});
		bar.Sub(t_("Pointcloud"), [this](Bar& bar) {
			bar.Add(t_("Record"), THISBACK(TogglePointcloudRecording))
				.Check(record_pointcloud);
			bar.Add(t_("Run Synthetic Sim"), THISBACK(RunSyntheticPointcloudSimDialog));
		});
		bar.Sub(t_("Debug"), [this](Bar& bar) {
			bar.Sub(t_("Pointcloud"), [this](Bar& bar) {
				bar.Add(t_("Generate Source Pointcloud"), THISBACK(DebugGeneratePointcloud));
				bar.Add(t_("Simulate HMD Observation"), THISBACK(DebugSimulateObservation));
				bar.Add(t_("Run Localization"), THISBACK(DebugRunLocalization));
				bar.Add(t_("Simulate Controller Observations"), THISBACK(DebugSimulateControllerObservations));
				bar.Add(t_("Run Controller Localization"), THISBACK(DebugRunControllerLocalization));
				bar.Add(t_("Run Full Synthetic Sim"), THISBACK(RunSyntheticPointcloudSimDialog));
				bar.Separator();
				bar.Add(t_("Clear Synthetic Data"), THISBACK(DebugClearSynthetic));
			});
		});
		
	});
	
	AddFrame(tool);
	RefrehToolbar();

	LoadEmptyProject();
	UpdateWindowTitle();
	
	tc.Set(-1000/60, THISBACK(Update));
	
}

void Edit3D::SetView(ViewType view) {
	
	if (this->view == VIEW_GEOMPROJECT)
		RemoveChild(&v0.hsplit);
	else if (this->view == VIEW_VIDEOIMPORT)
		RemoveChild(&v1);
	
	this->view = view;
	
	if (this->view == VIEW_GEOMPROJECT)
		Add(v0.hsplit.SizePos());
	else if (this->view == VIEW_VIDEOIMPORT)
		Add(v1.SizePos());
	
}

void Edit3D::RefrehToolbar() {
	tool.Set(THISBACK(Toolbar));
}

void Edit3D::Toolbar(Bar& bar) {
	bar.Add(true,  t_("Stop"),  ImagesImg::Stop(),  THISBACK(Stop)).Key(K_F6);
	
	if (anim->is_playing)
		bar.Add(true, t_("Pause"), ImagesImg::Pause(), THISBACK(Pause)).Key(K_F5);
	else
		bar.Add(true,  t_("Play"),  ImagesImg::Play(),  THISBACK(Play)).Key(K_F5);
	bar.Separator();
	bar.Add(t_("Repeat"), THISBACK(ToggleRepeatPlayback)).Check(repeat_playback);
	
}

GeomScene& Edit3D::GetActiveScene() {
	return state->GetActiveScene();
}

void Edit3D::Exit() {
	Close();
}

void Edit3D::RefreshData() {
	PostCallback(THISBACK(Data));
}

void Edit3D::Stop() {
	anim->Reset();
	RefrehToolbar();
}

void Edit3D::Pause() {
	anim->Pause();
	RefrehToolbar();
}

void Edit3D::Play() {
	anim->Play();
	RefrehToolbar();
}

void Edit3D::OnSceneEnd() {
	if (repeat_playback) {
		anim->Reset();
		anim->Play();
	}
	RefrehToolbar();
}

void Edit3D::RefrehRenderers() {
	if (view == VIEW_GEOMPROJECT) {
		v0.RefreshAll();
	}
	else if (view == VIEW_VIDEOIMPORT) {
		v1.RefreshRenderers();
	}
}

void Edit3D::Update() {
	double dt = ts.Seconds();
	ts.Reset();
	
	if (view == VIEW_GEOMPROJECT)
		v0.Update(dt);
	else if (view == VIEW_VIDEOIMPORT)
		v1.Update(dt);
	
	if (hmd.IsRunning()) {
		hmd.Poll();
		if (record_pointcloud)
			UpdateHmdCameraPose();
	}

	EnsureScriptInstances();
	for (auto& inst : script_instances)
		RunScriptFrame(inst, dt);
}

void Edit3D::Data() {
	if (view == VIEW_GEOMPROJECT)
		v0.Data();
}

void Edit3D::SetProjectDir(String dir) {
	if (dir.IsEmpty())
		dir = GetCurrentDirectory();
	project_dir = NormalizePath(dir);
	EnsureScriptInstances();
}

String Edit3D::GetScriptAbsPath(const String& rel) const {
	if (IsFullPath(rel))
		return NormalizePath(rel);
	if (project_dir.IsEmpty())
		return NormalizePath(AppendFileName(GetCurrentDirectory(), rel));
	return NormalizePath(AppendFileName(project_dir, rel));
}

String Edit3D::EnsureScriptFile(GeomScript& script, String base_name) {
	if (base_name.IsEmpty())
		base_name = "script";
	String safe = ToVarName(base_name, '_');
	if (safe.IsEmpty())
		safe = "script";
	if (project_dir.IsEmpty())
		SetProjectDir(GetCurrentDirectory());
	String rel = script.file;
	if (rel.IsEmpty()) {
		String dir = AppendFileName(project_dir, "scripts");
		RealizeDirectory(dir);
		String name = safe;
		String rel_try = AppendFileName("scripts", name + ".py");
		String abs_try = AppendFileName(project_dir, rel_try);
		int idx = 1;
		while (FileExists(abs_try)) {
			rel_try = AppendFileName("scripts", name + "_" + IntStr(idx++) + ".py");
			abs_try = AppendFileName(project_dir, rel_try);
		}
		rel = rel_try;
		script.file = rel;
	}
	String abs = GetScriptAbsPath(rel);
	if (!FileExists(abs)) {
		String header = "# Script: " + base_name + "\n";
		SaveFile(abs, header);
	}
	return rel;
}

GeomScript& Edit3D::AddScriptComponent(GeomObject& obj) {
	String id = "script";
	int idx = 1;
	while (obj.val.Find(id, AsTypeHash<GeomScript>()) >= 0)
		id = "script_" + IntStr(idx++);
	VfsValue& node = obj.val.Add(id, AsTypeHash<GeomScript>());
	GeomScript& script = node.GetExt<GeomScript>();
	EnsureScriptFile(script, obj.name.IsEmpty() ? id : obj.name);
	EnsureScriptInstances();
	return script;
}

void Edit3D::OpenScriptEditor(GeomScript& script) {
	EnsureScriptFile(script, "script");
	String abs = GetScriptAbsPath(script.file);
	if (script_editor.IsEmpty())
		script_editor.Create();
	script_editor->OpenFile(abs);
	script_editor->Open();
}

void Edit3D::RunScriptOnce(GeomScript& script) {
	EnsureScriptInstances();
	for (auto& inst : script_instances) {
		if (inst.script == &script) {
			UpdateScriptInstance(inst, true);
			RunScriptOnStart(inst, !script.run_on_load);
			return;
		}
	}
}

void Edit3D::RegisterScriptVM(PyVM& vm) {
	::RegisterGeometry(vm);
	PY_MODULE(modeler, vm)
	PY_MODULE_FUNC(log, ModelerLog, this);
	PY_MODULE_FUNC(debug_generate_pointcloud, ModelerDebugGeneratePointcloud, this);
	PY_MODULE_FUNC(debug_simulate_observation, ModelerDebugSimulateObservation, this);
	PY_MODULE_FUNC(debug_run_localization, ModelerDebugRunLocalization, this);
	PY_MODULE_FUNC(debug_simulate_controller_observations, ModelerDebugSimulateControllerObservations, this);
	PY_MODULE_FUNC(debug_run_controller_localization, ModelerDebugRunControllerLocalization, this);
	PY_MODULE_FUNC(debug_clear_synthetic, ModelerDebugClearSynthetic, this);
	PY_MODULE_FUNC(debug_run_full_synthetic, ModelerDebugRunFullSynthetic, this);
	PY_MODULE_FUNC(set_position, ModelerSetPosition, this);
	PY_MODULE_FUNC(set_orientation, ModelerSetOrientation, this);
	PY_MODULE_FUNC(get_position, ModelerGetPosition, this);
	PY_MODULE_FUNC(create_object, ModelerCreateObject, this);
	PY_MODULE_FUNC(create_directory, ModelerCreateDirectory, this);
	PY_MODULE_FUNC(get_project_dir, ModelerGetProjectDir, this);
}

void Edit3D::EnsureScriptInstances() {
	GeomScene& scene = GetActiveScene();
	Vector<GeomScript*> scripts;
	for (GeomObject& obj : GeomObjectCollection(scene)) {
		for (auto& sub : obj.val.sub) {
			if (IsVfsType(sub, AsTypeHash<GeomScript>()))
				scripts.Add(&sub.GetExt<GeomScript>());
		}
	}
	for (int i = script_instances.GetCount() - 1; i >= 0; i--) {
		bool found = false;
		for (int j = 0; j < scripts.GetCount(); j++) {
			if (scripts[j] == script_instances[i].script) {
				found = true;
				break;
			}
		}
		if (!found)
			script_instances.Remove(i);
	}
	for (GeomScript* script : scripts) {
		bool found = false;
		for (auto& inst : script_instances) {
			if (inst.script == script) {
				found = true;
				break;
			}
		}
		if (!found) {
			ScriptInstance& inst = script_instances.Add();
			inst.script = script;
			RegisterScriptVM(inst.vm);
		}
	}
}

void Edit3D::UpdateScriptInstance(ScriptInstance& inst, bool force_reload) {
	if (!inst.script)
		return;
	GeomScript& script = *inst.script;
	if (!script.enabled)
		return;
	EnsureScriptFile(script, "script");
	String abs = GetScriptAbsPath(script.file);
	Time mod = FileGetTime(abs);
	bool needs_reload = force_reload || !inst.loaded || mod != inst.file_time;
	if (!needs_reload)
		return;
	inst.file_time = mod;
	inst.loaded = false;
	inst.has_start = false;
	inst.has_frame = false;
	String code = LoadFile(abs);
	String err;
	Vector<PyIR> ir;
	if (!CompilePySource(code, abs, ir, err)) {
		LOG("Script compile failed: " + err);
		return;
	}
	inst.vm = PyVM();
	RegisterScriptVM(inst.vm);
	inst.vm.GetGlobals().GetAdd(PyValue("__project_dir__")) = PyValue(project_dir);
	inst.vm.GetGlobals().GetAdd(PyValue("__script_path__")) = PyValue(abs);
	if (!RunPyIR(inst.vm, ir, err)) {
		LOG("Script run failed: " + err);
		return;
	}
	inst.loaded = true;
	inst.main_ir = pick(ir);
	inst.has_start = !inst.vm.GetGlobals().GetItem(PyValue("on_start")).IsNone();
	inst.has_frame = !inst.vm.GetGlobals().GetItem(PyValue("on_frame")).IsNone();
	if (inst.has_start) {
		Vector<PyIR> start_ir;
		if (CompilePySource("on_start()", abs, start_ir, err))
			inst.start_ir = pick(start_ir);
	}
	if (inst.has_frame) {
		Vector<PyIR> frame_ir;
		if (CompilePySource("on_frame(__dt__)", abs, frame_ir, err))
			inst.frame_ir = pick(frame_ir);
	}
	RunScriptOnStart(inst, false);
}

void Edit3D::RunScriptOnStart(ScriptInstance& inst, bool force) {
	if (!inst.script || !inst.loaded)
		return;
	if (!force && !inst.script->run_on_load)
		return;
	if (!inst.has_start)
		return;
	String err;
	if (!RunPyIR(inst.vm, inst.start_ir, err))
		LOG("Script on_start failed: " + err);
}

void Edit3D::RunScriptFrame(ScriptInstance& inst, double dt) {
	if (!inst.script || !inst.script->enabled || !inst.script->run_every_frame)
		return;
	UpdateScriptInstance(inst, false);
	if (!inst.loaded || !inst.has_frame)
		return;
	inst.vm.GetGlobals().GetAdd(PyValue("__dt__")) = PyValue(dt);
	String err;
	if (!RunPyIR(inst.vm, inst.frame_ir, err))
		LOG("Script on_frame failed: " + err);
}

void Edit3D::CreateDefaultInit() {
	
	// Cler project
	prj->Clear();
	
	// Add scene
	GeomScene& scene = prj->AddScene();
	
	
}

void Edit3D::CreateDefaultPostInit() {
	GeomScene& scene = prj->GetScene(0);
	GeomObject* cam = scene.FindCamera("camera");
	
	if (cam) {
		GeomTimeline* tl = cam->FindTimeline();
		if (tl && !tl->keypoints.IsEmpty()) {
			GeomKeypoint& kp = tl->keypoints.Get(0);
			GeomCamera& program = state->GetProgram();
			program.position = kp.position;
			program.orientation = kp.orientation;
		}
	}
	else {
		GeomCamera& program = state->GetProgram();
		program.position = vec3(0,0,0);
		program.orientation = Identity<quat>();
	}
	
	state->active_scene = 0;
	
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
}

void Edit3D::LoadEmptyProject() {
	scene3d_path.Clear();
	scene3d_created.Clear();
	scene3d_modified.Clear();
	scene3d_data_dir = "data";
	scene3d_external_files.Clear();
	scene3d_meta.Clear();
	scene3d_use_json = true;
	repeat_playback = false;
	if (project_dir.IsEmpty())
		SetProjectDir(GetCurrentDirectory());
	script_instances.Clear();
	CreateDefaultInit();
	CreateDefaultPostInit();
	UpdateWindowTitle();
	
}

void Edit3D::LoadTestCirclingCube() {
	GeomScene& scene = prj->GetScene(0);
	GeomObject& cam = scene.GetAddCamera("camera");
	GeomObject& mdl = scene.GetAddModel("some model");
	
	ModelBuilder mb;
	mb.AddBox(0, 1, 1);
	
	mdl.mdl = mb.Detach();
	
	scene.length = prj->kps * 4 + 1;
	float step = M_PIf * 2 / 4;
	float angle = 0;
	float cam_radius = 2;
	for(int i = 0; i < 5; i++) {
		GeomKeypoint& kp = cam.GetTimeline().GetAddKeypoint(i * prj->kps);
		kp.position = vec3(sin(angle), 0, cos(angle)) * cam_radius;
		kp.orientation = AxesQuat(angle, 0, 0);
		angle += step;
	}
}

void Edit3D::LoadTestOctree() {
	GeomScene& scene = prj->GetScene(0);
	GeomObject& cam = scene.GetAddCamera("camera");
	
	// Create octree
	GeomObject& omodel = scene.GetAddOctree("octree");
	Octree& o = omodel.octree.octree;
	o.Initialize(-3, 8); // 1 << 6 = 32x32x32 meters
	
	// Create points in form of sphere
	float radius = 100;
	bool random_points = 1;
	int points = 256;
	for(int i = 0; i < points; i++) {
		float yaw, pitch;
		if (random_points) {
			yaw = Randomf() * M_PIf * 2;
			float f = (Randomf() * 2 - 1);
			pitch = f * (M_PI / 2);
		}
		else {
			float f = ((float)i / (float)points);
			yaw = f * M_PI * 2;
			pitch = 0;//fmodf(f * M_PI * 40, M_PI) - M_PI/2;
		}
		
		vec3 pos = AxesDir(yaw, pitch) * radius;
		OctreeNode* n = o.GetAddNode(pos, -3);
		Pointcloud::Point& p = n->Add<Pointcloud::Point>();
		p.SetPosition(pos);
		//LOG(pos.ToString() + ": " + HexStr(n));
	}
	
	// Move camera linearly around sphere
	int seconds = 3;
	scene.length = prj->kps * seconds;
	int kp_step = 3;
	float step = M_PIf * 2 / (scene.length / kp_step - 1);
	float angle = 0;
	float cam_radius = radius + 2;
	for(int i = 0; i < scene.length; i += kp_step) {
		GeomKeypoint& kp = cam.GetTimeline().GetAddKeypoint(i);
		kp.position = vec3(sin(angle), 0, cos(angle)) * cam_radius;
		kp.orientation = AxesQuat(angle, 0, 0);
		angle += step;
	}

}

void Edit3D::LoadTestProject(int test_i) {
	script_instances.Clear();
	CreateDefaultInit();
	
	switch (test_i) {
		case 0: LoadTestCirclingCube(); break;
		case 1: LoadTestOctree(); break;
		case 2: LoadTestHmdPointcloud(); break;
		default: break;
	}
	
	CreateDefaultPostInit();
}

void Edit3D::LoadTestHmdPointcloud() {
	GeomScene& scene = prj->GetScene(0);
	scene.name = "HMD Pointcloud";
	scene.GetAddCamera("hmd_camera");
	scene.GetAddOctree("hmd_pointcloud");
}

void Edit3D::LoadWmrStereoPointcloud(String directory) {
	video.SetWmrCamera();
	video.LoadDirectory(directory, 30);
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
	
	SetView(VIEW_VIDEOIMPORT);
}

void Edit3D::UpdateWindowTitle() {
	String format = scene3d_use_json ? "JSON" : "Binary";
	String filename;
	if (!scene3d_path.IsEmpty())
		filename = " - " + GetFileName(scene3d_path);
	Title(Format("ModelerApp - Scene3D v%d (%s)%s", SCENE3D_VERSION, format, filename));
}

void Edit3D::SetScene3DFormat(bool use_json) {
	scene3d_use_json = use_json;
	UpdateWindowTitle();
}

void Edit3D::ToggleRepeatPlayback() {
	repeat_playback = !repeat_playback;
	RefrehToolbar();
}

void Edit3D::EnsureHmdSceneObjects() {
	GeomScene& scene = state->GetActiveScene();
	GeomDirectory& fusion_room = scene.GetAddDirectory("fusion_room");
	fusion_room.GetAddCamera("hmd_camera");
	hmd_pointcloud = &fusion_room.GetAddOctree("hmd_pointcloud");
	state->UpdateObjects();
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
}

void Edit3D::EnsureSimSceneObjects() {
	GeomScene& scene = state->GetActiveScene();
	for (int i = scene.val.sub.GetCount() - 1; i >= 0; i--) {
		VfsValue& n = scene.val.sub[i];
		if (!IsVfsType(n, AsTypeHash<GeomObject>()))
			continue;
		GeomObject& o = n.GetExt<GeomObject>();
		if (o.name == "sim_fake_camera" || o.name == "sim_localized_camera")
			scene.val.sub.Remove(i);
	}
	GeomDirectory& sim_raw = scene.GetAddDirectory("sim_raw_space");
	GeomDirectory& fusion_room = scene.GetAddDirectory("fusion_room");
	GeomObject& fake_cam = sim_raw.GetAddCamera("sim_fake_camera");
	GeomObject& localized_cam = fusion_room.GetAddCamera("hmd_camera");
	sim_pointcloud_obj = &scene.GetAddOctree("sim_pointcloud");
	sim_observation_obj = &sim_raw.GetAddOctree("sim_observation");
	sim_controller_obj[0] = &fusion_room.GetAddOctree("sim_controller_0");
	sim_controller_obj[1] = &fusion_room.GetAddOctree("sim_controller_1");
	sim_controller_model_obj[0] = &fusion_room.GetAddModel("sim_controller_model_0");
	sim_controller_model_obj[1] = &fusion_room.GetAddModel("sim_controller_model_1");
	sim_hmd_pointcloud_obj = &fusion_room.GetAddOctree("hmd_pointcloud");
	for (int i = 0; i < 2; i++) {
		if (!sim_controller_model_obj[i])
			continue;
		GeomObject& obj = *sim_controller_model_obj[i];
		if (!obj.mdl) {
			ModelBuilder builder;
			builder.AddBox(vec3(0, 0, 0), vec3(0.06f, 0.02f, 0.12f), true);
			obj.mdl = builder.Detach();
		}
	}
	UpdateCameraObjectRender(fake_cam, sim_fake_hmd_pose, true);
	UpdateCameraObjectRender(localized_cam, sim_localized_pose, true);
	state->UpdateObjects();
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
}

void Edit3D::TogglePointcloudRecording() {
	if (record_pointcloud)
		StopPointcloudRecording();
	else
		StartPointcloudRecording();
}

void Edit3D::StartPointcloudRecording() {
	if (record_pointcloud)
		return;
	if (!hmd.Start()) {
		PromptOK("Failed to initialise HMD camera.");
		return;
	}
	hmd.ResetTracking();
	hmd.recording = true;
	record_pointcloud = true;
	EnsureHmdSceneObjects();
	if (hmd_pointcloud) {
		const Octree* octree = hmd.GetPointcloud(true);
		hmd_pointcloud->octree_ptr = octree ? const_cast<Octree*>(octree) : 0;
	}
}

void Edit3D::StopPointcloudRecording() {
	if (!record_pointcloud)
		return;
	record_pointcloud = false;
	hmd.recording = false;
	if (hmd_pointcloud)
		hmd_pointcloud->octree_ptr = 0;
	hmd.Stop();
}

void Edit3D::UpdateHmdCameraPose() {
	GeomCamera& cam = state->GetProgram();
	FusionState fs;
	if (hmd.fusion.GetState(fs)) {
		cam.position = fs.position;
		cam.orientation = fs.orientation;
		return;
	}
	HMD::SoftHmdVisualTracker& tracker = hmd.fusion.GetBrightTracker();
	if (tracker.HasPose()) {
		cam.position = tracker.GetPosition();
		cam.orientation = tracker.GetOrientation();
	}
}

void Edit3D::RunSyntheticPointcloudSimDialog() {
	String log;
	bool ok = RunSyntheticPointcloudSim(log);
	log << "\nResult: " << (ok ? "OK" : "FAIL");
	String safe = log;
	safe.Replace("[", "(");
	safe.Replace("]", ")");
	PromptOK(safe);
}

void Edit3D::DebugGeneratePointcloud() {
	sim_state = BuildSyntheticPointcloud(sim_cfg);
	sim_has_state = true;
	sim_has_obs = false;
	sim_has_ctrl_obs = false;
	sim_fake_hmd_pose = PointcloudPose::MakeIdentity();
	sim_localized_pose = PointcloudPose::MakeIdentity();
	EnsureSimSceneObjects();
	GeomScene& scene = state->GetActiveScene();
	GeomDirectory& sim_raw = scene.GetAddDirectory("sim_raw_space");
	GeomDirectory& fusion_room = scene.GetAddDirectory("fusion_room");
	GeomTransform& raw_tr = sim_raw.GetTransform();
	raw_tr.position = sim_fake_hmd_pose.position;
	raw_tr.orientation = sim_fake_hmd_pose.orientation;
	GeomTransform& fusion_tr = fusion_room.GetTransform();
	fusion_tr.position = vec3(0);
	fusion_tr.orientation = Identity<quat>();
	if (sim_pointcloud_obj) {
		FillOctree(sim_pointcloud_obj->octree.octree, sim_state.reference.points, -3);
		sim_pointcloud_obj->octree_ptr = 0;
	}
	if (sim_observation_obj) {
		sim_observation_obj->octree.octree.Initialize(-3, 8);
		sim_observation_obj->octree_ptr = 0;
	}
	for (int i = 0; i < 2; i++) {
		if (sim_controller_obj[i]) {
			sim_controller_obj[i]->octree.octree.Initialize(-3, 8);
			sim_controller_obj[i]->octree_ptr = 0;
		}
	}
	GeomCamera& focus = state->GetFocus();
	focus.position = sim_fake_hmd_pose.position;
	focus.orientation = sim_fake_hmd_pose.orientation;
	GeomCamera& program = state->GetProgram();
	program.position = sim_localized_pose.position;
	program.orientation = sim_localized_pose.orientation;
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, true);
	UpdateCameraObjectRender(scene.GetAddCamera("hmd_camera"), sim_localized_pose, true);
	RefrehRenderers();
}

void Edit3D::GenerateSyntheticPointcloudFor(GeomObject& obj) {
	if (!obj.IsOctree())
		return;
	SyntheticPointcloudState synth_state = BuildSyntheticPointcloud(sim_cfg);
	FillOctree(obj.octree.octree, synth_state.reference.points, -3);
	obj.octree_ptr = 0;
	state->UpdateObjects();
	v0.RefreshAll();
}

void Edit3D::DebugSimulateObservation() {
	if (!sim_has_state)
		DebugGeneratePointcloud();
	sim_fake_hmd_pose = RandomHmdPose(sim_cfg);
	sim_state.hmd_pose_world = sim_fake_hmd_pose;
	sim_obs = SimulateHmdObservation(sim_state, sim_cfg);
	sim_has_obs = true;
	EnsureSimSceneObjects();
	GeomScene& scene = state->GetActiveScene();
	GeomDirectory& sim_raw = scene.GetAddDirectory("sim_raw_space");
	GeomTransform& raw_tr = sim_raw.GetTransform();
	raw_tr.position = sim_fake_hmd_pose.position;
	raw_tr.orientation = sim_fake_hmd_pose.orientation;
	state->UpdateObjects();
	RefreshSimObservation();
	GeomCamera& cam = state->GetFocus();
	cam.position = sim_fake_hmd_pose.position;
	cam.orientation = sim_fake_hmd_pose.orientation;
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, true);
	RefrehRenderers();
}

String Edit3D::RunLocalizationLog(bool show_dialog) {
	if (!sim_has_state)
		DebugGeneratePointcloud();
	if (!sim_has_obs)
		DebugSimulateObservation();
	PointcloudLocalizerStub localizer;
	PointcloudLocalizationResult loc = localizer.Locate(sim_state.reference, sim_obs);
	if (loc.ok) {
		sim_localized_pose = loc.pose;
		GeomScene& scene = state->GetActiveScene();
		GeomCamera& cam = state->GetProgram();
		cam.position = sim_localized_pose.position;
		cam.orientation = sim_localized_pose.orientation;
		UpdateCameraObjectRender(scene.GetAddCamera("hmd_camera"), sim_localized_pose, true);
		if (sim_hmd_pointcloud_obj) {
			GeomPointcloudEffectTransform& fx = sim_hmd_pointcloud_obj->GetAddPointcloudEffect("localization");
			if (!fx.locked) {
				fx.position = sim_localized_pose.position;
				fx.orientation = sim_localized_pose.orientation;
			}
			fx.enabled = true;
		}
		state->UpdateObjects();
		RefreshSimObservation();
	}
	String log;
	log << "Localization: ok=" << (int)loc.ok;
	log << " pos=" << Format("(%.3f, %.3f, %.3f)",
		loc.pose.position[0], loc.pose.position[1], loc.pose.position[2]);
	log << "\n";
	String safe = log;
	safe.Replace("[", "(");
	safe.Replace("]", ")");
	if (show_dialog)
		PromptOK(safe);
	return safe;
}

void Edit3D::RefreshSimObservation() {
	if (!sim_observation_obj || !sim_has_obs)
		return;
	if (!sim_obs_octree)
		sim_obs_octree = MakeOne<Octree>();
	FillOctree(*sim_obs_octree, sim_obs.points, -3);
	sim_observation_obj->octree_ptr = sim_obs_octree.Get();
	if (sim_hmd_pointcloud_obj)
		sim_hmd_pointcloud_obj->octree_ptr = sim_obs_octree.Get();
	RefrehRenderers();
}

String Edit3D::RunControllerLocalizationLog(bool show_dialog) {
	if (!sim_has_state)
		DebugGeneratePointcloud();
	if (!sim_has_ctrl_obs)
		DebugSimulateControllerObservations();
	ControllerPatternTrackerStub tracker;
	ControllerFusionStub fusion;
	String log;
	for (int i = 0; i < sim_ctrl_obs.GetCount() && i < sim_state.controllers.GetCount(); i++) {
		if (sim_ctrl_obs[i].points.IsEmpty()) {
			log << "controller[" << i << "] ok=0 pos=(0.000, 0.000, 0.000)\n";
			continue;
		}
		ControllerPoseResult pose = tracker.Track(sim_state.controllers[i], sim_ctrl_obs[i]);
		ControllerFusionSample sample;
		sample.has_orientation = true;
		sample.orientation = pose.pose.orientation;
		pose = fusion.Fuse(pose, sample);
		log << "controller[" << i << "] ok=" << (int)pose.ok;
		log << " pos=" << Format("(%.3f, %.3f, %.3f)",
			pose.pose.position[0], pose.pose.position[1], pose.pose.position[2]) << "\n";
	}
	String safe = log;
	safe.Replace("[", "(");
	safe.Replace("]", ")");
	if (show_dialog)
		PromptOK(safe);
	return safe;
}

void Edit3D::DebugRunLocalization() {
	RunLocalizationLog(true);
	RefrehRenderers();
}

void Edit3D::DebugRunControllerLocalization() {
	RunControllerLocalizationLog(true);
}

void Edit3D::DebugSimulateControllerObservations() {
	if (!sim_has_state)
		DebugGeneratePointcloud();
	sim_state.controller_poses_world.SetCount(2);
	sim_state.hmd_pose_world = sim_fake_hmd_pose;
	sim_state.controller_poses_world[0] = RandomControllerPoseInFrustum(sim_fake_hmd_pose, sim_cfg, 0.4f, 1.2f);
	sim_state.controller_poses_world[1] = RandomControllerPoseInFrustum(sim_fake_hmd_pose, sim_cfg, 0.4f, 1.2f);
	sim_ctrl_obs = SimulateControllerObservations(sim_state, sim_cfg);
	sim_has_ctrl_obs = true;
	EnsureSimSceneObjects();
	for (int i = 0; i < sim_ctrl_obs.GetCount() && i < 2; i++) {
		if (!sim_controller_obj[i])
			continue;
		Vector<vec3> world_points = TransformPointsToWorld(sim_state.hmd_pose_world, sim_ctrl_obs[i].points);
		FillOctree(sim_controller_obj[i]->octree.octree, world_points, -3);
		sim_controller_obj[i]->octree_ptr = 0;
		if (sim_controller_model_obj[i]) {
			GeomObject& obj = *sim_controller_model_obj[i];
			GeomTimeline& tl = obj.GetTimeline();
			tl.keypoints.Clear();
			GeomKeypoint& kp = tl.keypoints.Add(0);
			kp.position = sim_state.controller_poses_world[i].position;
			kp.orientation = sim_state.controller_poses_world[i].orientation;
		}
	}
	GeomCamera& cam = state->GetFocus();
	cam.position = sim_fake_hmd_pose.position;
	cam.orientation = sim_fake_hmd_pose.orientation;
	GeomScene& scene = state->GetActiveScene();
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, true);
	RefrehRenderers();
}

void Edit3D::DebugClearSynthetic() {
	sim_has_state = false;
	sim_has_obs = false;
	sim_has_ctrl_obs = false;
	sim_state.reference.Clear();
	sim_ctrl_obs.Clear();
	EnsureSimSceneObjects();
	if (sim_pointcloud_obj)
		sim_pointcloud_obj->octree.octree.Initialize(-3, 8);
	if (sim_observation_obj)
		sim_observation_obj->octree.octree.Initialize(-3, 8);
	for (int i = 0; i < 2; i++) {
		if (sim_controller_obj[i])
			sim_controller_obj[i]->octree.octree.Initialize(-3, 8);
	}
	RefrehRenderers();
}

void Edit3D::RunSyntheticSimVisual(bool log_stdout, bool verbose) {
	DebugGeneratePointcloud();
	DebugSimulateObservation();
	String log;
	log << "Synthetic sim (visual)\n";
	log << "reference points=" << sim_state.reference.points.GetCount() << "\n";
	log << "observation points=" << sim_obs.points.GetCount() << "\n";
	int obs_in_frustum = 0;
	int ref_in_range = 0;
	int ref_z_pos = 0;
	int ref_z_neg = 0;
	float min_z = 1e9f;
	float max_z = -1e9f;
	for (int i = 0; i < sim_obs.points.GetCount(); i++) {
		if (VisibleInFrustumSimple(sim_obs.points[i], sim_cfg))
			obs_in_frustum++;
	}
	for (int i = 0; i < sim_state.reference.points.GetCount(); i++) {
		vec3 cam = ApplyInversePoseSimple(sim_fake_hmd_pose, sim_state.reference.points[i]);
		float len = cam.GetLength();
		if (len <= sim_cfg.max_range)
			ref_in_range++;
		min_z = min(min_z, cam[2]);
		max_z = max(max_z, cam[2]);
		if (cam[2] >= 0)
			ref_z_pos++;
		else
			ref_z_neg++;
	}
	log << "observation in-frustum=" << obs_in_frustum << "\n";
	if (verbose) {
		log << "reference in-range=" << ref_in_range << "\n";
		log << "reference z>=0=" << ref_z_pos << " z<0=" << ref_z_neg << "\n";
		log << "reference z min=" << min_z << " max=" << max_z << "\n";
	}
	if (verbose && sim_state.reference.points.GetCount() > 0) {
		vec3 cam0 = ApplyInversePoseSimple(sim_fake_hmd_pose, sim_state.reference.points[0]);
		vec3 ref0 = sim_state.reference.points[0];
		log << "reference0 world=(" << ref0[0] << " " << ref0[1] << " " << ref0[2] << ")\n";
		log << "fake camera pos=(" << sim_fake_hmd_pose.position[0] << " "
			<< sim_fake_hmd_pose.position[1] << " " << sim_fake_hmd_pose.position[2] << ")\n";
		log << "fake camera orient=(" << sim_fake_hmd_pose.orientation[0] << " "
			<< sim_fake_hmd_pose.orientation[1] << " " << sim_fake_hmd_pose.orientation[2]
			<< " " << sim_fake_hmd_pose.orientation[3] << ")\n";
		vec3 rel0 = ref0 - sim_fake_hmd_pose.position;
		log << "reference0 rel=(" << rel0[0] << " " << rel0[1] << " " << rel0[2] << ")\n";
		vec3 cam0_inv = VectorTransform(rel0, sim_fake_hmd_pose.orientation.GetInverse());
		vec3 cam0_fwd = VectorTransform(rel0, sim_fake_hmd_pose.orientation);
		log << "reference0 cam_inv=(" << cam0_inv[0] << " " << cam0_inv[1] << " " << cam0_inv[2] << ")\n";
		log << "reference0 cam_fwd=(" << cam0_fwd[0] << " " << cam0_fwd[1] << " " << cam0_fwd[2] << ")\n";
		log << "reference0 cam=(" << cam0[0] << " " << cam0[1] << " " << cam0[2] << ")\n";
	}
	log << RunLocalizationLog(false);
	DebugSimulateControllerObservations();
	log << "controller observations=" << sim_ctrl_obs.GetCount() << "\n";
	int ctrl_points_total = 0;
	for (int i = 0; i < sim_ctrl_obs.GetCount(); i++)
		ctrl_points_total += sim_ctrl_obs[i].points.GetCount();
	log << "controller points=" << ctrl_points_total << "\n";
	int ctrl_centers_in_frustum = 0;
	for (int i = 0; i < sim_state.controller_poses_world.GetCount(); i++) {
		vec3 center_cam = ApplyInversePoseSimple(sim_fake_hmd_pose, sim_state.controller_poses_world[i].position);
		bool visible = VisibleInFrustumSimple(center_cam, sim_cfg);
		if (verbose) {
			log << "controller[" << i << "] cam=(" << center_cam[0] << " " << center_cam[1] << " " << center_cam[2]
				<< ") visible=" << (int)visible << "\n";
		}
		if (visible)
			ctrl_centers_in_frustum++;
	}
	log << "controller centers in-frustum=" << ctrl_centers_in_frustum << "\n";
	if (sim_state.controller_poses_world.GetCount() >= 2) {
		vec3 delta = sim_state.controller_poses_world[0].position - sim_state.controller_poses_world[1].position;
		if (verbose)
			log << "controller separation=" << delta.GetLength() << "\n";
	}
	if (verbose) {
		vec3 cam_to_origin = sim_fake_hmd_pose.position;
		log << "fake camera dist to origin=" << cam_to_origin.GetLength() << "\n";
	}
	log << RunControllerLocalizationLog(false);
	bool ok = !sim_obs.points.IsEmpty();
	ok = ok && (obs_in_frustum == sim_obs.points.GetCount());
	ok = ok && (ctrl_centers_in_frustum == sim_state.controller_poses_world.GetCount());
	ok = ok && (ctrl_points_total > 0);
	log << "sanity ok=" << (int)ok << "\n";
	if (log_stdout)
		Cout() << log;
}

bool Edit3D::IsScene3DBinaryPath(const String& path) const {
	String ext = ToLower(GetFileExt(path));
	if (ext == ".scene3db")
		return true;
	if (ext == ".bin") {
		String base = GetFileExt(GetFileTitle(path));
		return ToLower(base) == ".scene3d";
	}
	return false;
}

bool Edit3D::IsScene3DJsonPath(const String& path) const {
	return ToLower(GetFileExt(path)) == ".scene3d";
}

String Edit3D::EnsureScene3DExtension(const String& path, bool use_json) const {
	String ext = ToLower(GetFileExt(path));
	if (use_json) {
		if (ext == ".scene3d")
			return path;
		return AppendFileName(GetFileFolder(path), GetFileTitle(path) + ".scene3d");
	}
	if (ext == ".scene3db")
		return path;
	if (ext == ".bin" && ToLower(GetFileExt(GetFileTitle(path))) == ".scene3d")
		return path;
	return AppendFileName(GetFileFolder(path), GetFileTitle(path) + ".scene3db");
}

void Edit3D::OpenScene3D() {
	LoadScene3DWithDialog();
}

void Edit3D::OpenFilePool() {
	file_pool.Data();
	if (!file_pool.IsOpen())
		file_pool.Open();
	file_pool.SetFocus();
}

void Edit3D::SaveScene3DInteractive() {
	if (scene3d_path.IsEmpty())
		SaveScene3DAs();
	else
		SaveScene3D(scene3d_path, scene3d_use_json, true);
}

void Edit3D::SaveScene3DAs() {
	SaveScene3DWithDialog(scene3d_use_json);
}

void Edit3D::SaveScene3DAsJson() {
	SaveScene3DWithDialog(true);
}

void Edit3D::SaveScene3DAsBinary() {
	SaveScene3DWithDialog(false);
}

bool Edit3D::SaveScene3DWithDialog(bool use_json) {
	FileSel fs;
	fs.Type(t_("Scene3D (JSON)"), "*.scene3d");
	fs.Type(t_("Scene3D (Binary)"), "*.scene3db");
	fs.AllFilesType();
	if (!scene3d_path.IsEmpty())
		fs.Set(scene3d_path);
	if (!fs.ExecuteSaveAs())
		return false;
	String path = EnsureScene3DExtension(~fs, use_json);
	return SaveScene3D(path, use_json, true);
}

bool Edit3D::LoadScene3DWithDialog() {
	FileSel fs;
	fs.Type(t_("Scene3D (JSON)"), "*.scene3d");
	fs.Type(t_("Scene3D (Binary)"), "*.scene3db");
	fs.AllFilesType();
	if (!fs.ExecuteOpen())
		return false;
	return LoadScene3D(~fs);
}

static String Scene3DIsoTime(Time t) {
	String date = Format("%04d-%02d-%02d", t.year, t.month, t.day);
	String clock = Format("%02d:%02d:%02d", t.hour, t.minute, t.second);
	return date + "T" + clock + "Z";
}

void Edit3D::SyncPointcloudDatasetsExternalFiles() {
	GeomScene& scene = GetActiveScene();
	Vector<Scene3DExternalFile> kept;
	for (const Scene3DExternalFile& file : scene3d_external_files) {
		if (file.type != "pointcloud.dataset")
			kept.Add(file);
	}
	for (auto& s : scene.val.sub) {
		if (!IsVfsType(s, AsTypeHash<GeomPointcloudDataset>()))
			continue;
		GeomPointcloudDataset& ds = s.GetExt<GeomPointcloudDataset>();
		if (ds.source_ref.IsEmpty())
			continue;
		Scene3DExternalFile f;
		f.id = ds.GetId();
		f.type = "pointcloud.dataset";
		f.path = ds.source_ref;
		String abs = IsFullPath(ds.source_ref)
			? ds.source_ref
			: AppendFileName(AppendFileName(project_dir, scene3d_data_dir), ds.source_ref);
		if (FileExists(abs)) {
			f.size = FileLength(abs);
			f.modified_utc = Scene3DIsoTime(FileGetTime(abs));
		}
		kept.Add(f);
	}
	scene3d_external_files.Clear();
	for (const Scene3DExternalFile& f : kept)
		scene3d_external_files.Add(f);
}

bool Edit3D::LoadScene3D(const String& path) {
	Scene3DDocument doc;
	bool use_json = !IsScene3DBinaryPath(path);
	if (use_json) {
		if (!LoadScene3DJson(path, doc))
			return false;
	}
	else if (!LoadScene3DBin(path, doc)) {
		return false;
	}
	for (GeomScene& scene : doc.project->val.Sub<GeomScene>()) {
		GeomObjectCollection objects(scene);
		for (GeomObject& obj : objects) {
			if (!obj.IsModel())
				continue;
			if (obj.mdl || !obj.asset_ref.IsEmpty())
				continue;
			ModelBuilder mb;
			mb.AddBox(0, 1, 1);
			obj.mdl = mb.Detach();
		}
	}
	VisitCopy(*doc.project, *prj);
	state->prj = prj;
	state->active_scene = doc.active_scene;
	VisitCopy(*doc.focus, state->GetFocus());
	VisitCopy(*doc.program, state->GetProgram());
	scene3d_path = path;
	SetProjectDir(GetFileFolder(path));
	scene3d_use_json = use_json;
	scene3d_created = doc.created_utc;
	scene3d_modified = doc.modified_utc;
	scene3d_data_dir = doc.data_dir;
	scene3d_external_files = pick(doc.external_files);
	scene3d_meta = pick(doc.meta);
	if (scene3d_data_dir.IsEmpty())
		scene3d_data_dir = "data";
	script_instances.Clear();
	state->UpdateObjects();
	anim->Reset();
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
	UpdateWindowTitle();
	return true;
}

bool Edit3D::SaveScene3D(const String& path, bool use_json, bool pretty) {
	SyncPointcloudDatasetsExternalFiles();
	Scene3DDocument doc;
	doc.version = SCENE3D_VERSION;
	doc.name = "ModelerApp";
	doc.project = prj;
	doc.active_scene = state->active_scene;
	doc.focus = &state->GetFocus();
	doc.program = &state->GetProgram();
	if (scene3d_created.IsEmpty())
		scene3d_created = Scene3DIsoTime(GetUtcTime());
	scene3d_modified = Scene3DIsoTime(GetUtcTime());
	if (scene3d_data_dir.IsEmpty())
		scene3d_data_dir = "data";
	doc.created_utc = scene3d_created;
	doc.modified_utc = scene3d_modified;
	doc.data_dir = scene3d_data_dir;
	doc.external_files.Clear();
	for (const Scene3DExternalFile& file : scene3d_external_files)
		doc.external_files.Add(file);
	doc.meta.Clear();
	for (const Scene3DMetaEntry& entry : scene3d_meta)
		doc.meta.Add(entry);
	bool ok = use_json ? SaveScene3DJson(path, doc, pretty) : SaveScene3DBin(path, doc);
	if (ok) {
		scene3d_path = path;
		SetProjectDir(GetFileFolder(path));
		scene3d_use_json = use_json;
		UpdateWindowTitle();
	}
	return ok;
}

#if 0
void Edit3D::LoadRemote(EditClientService* svc, bool debug) {
	this->svc = svc;
	this->debug_remote = debug;
	
	if (svc) {
		svc->sync.SetTarget(*prj, *state, *anim, video);
	}
	
	if (debug_remote) {
		svc->SetDebuggingMode();
		
		LoadEmptyProject();
		SetView(VIEW_REMOTE_DEBUG);
	}
	else
		LoadTestProject(0);
	
	svc->edit = this;
	svc->sync.SetRequireAllSync();
	svc->SetReady();
}
#endif

void Edit3D::OnDebugMetadata() {
	
	TODO
	
}



END_UPP_NAMESPACE
