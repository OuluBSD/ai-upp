#include "ModelerApp.h"

#define IMAGECLASS ImagesImg
#define IMAGEFILE <ModelerApp/Images.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP


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
		for(int i = 0; i < 4; i++)
			v0.rends[i].Refresh();
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
}

void Edit3D::Data() {
	if (view == VIEW_GEOMPROJECT)
		v0.Data();
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
	CreateDefaultInit();
	
	switch (test_i) {
		case 0: LoadTestCirclingCube(); break;
		case 1: LoadTestOctree(); break;
		default: break;
	}
	
	CreateDefaultPostInit();
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
	scene3d_use_json = use_json;
	scene3d_created = doc.created_utc;
	scene3d_modified = doc.modified_utc;
	scene3d_data_dir = doc.data_dir;
	scene3d_external_files = pick(doc.external_files);
	scene3d_meta = pick(doc.meta);
	if (scene3d_data_dir.IsEmpty())
		scene3d_data_dir = "data";
	state->UpdateObjects();
	anim->Reset();
	Data();
	v0.TimelineData();
	v0.tree.OpenDeep(v0.tree_scenes);
	UpdateWindowTitle();
	return true;
}

bool Edit3D::SaveScene3D(const String& path, bool use_json, bool pretty) {
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
