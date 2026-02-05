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
	Vector<PyIR> run;
	run.Append(ir);
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

PyValue ModelerTrace(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() > 0)
		LOG(args[0].ToString());
	return PyValue::None();
}

PyValue ModelerGetTimer(const Vector<PyValue>& args, void* user_data) {
	Edit3D* e = (Edit3D*)user_data;
	if (!e)
		return PyValue(0);
	return PyValue((int64)e->script_timer.Elapsed());
}

PyValue ModelerRandom(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() == 0)
		return PyValue(Randomf());
	if (args.GetCount() == 1)
		return PyValue(Randomf() * args[0].AsDouble());
	double minv = args[0].AsDouble();
	double maxv = args[1].AsDouble();
	return PyValue(minv + (maxv - minv) * Randomf());
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

static hash_t HashGeomDirectory() { static hash_t h = TypedStringHasher<GeomDirectory>("GeomDirectory"); return h; }
static hash_t HashGeomScene() { static hash_t h = TypedStringHasher<GeomScene>("GeomScene"); return h; }
static hash_t HashGeomObject() { static hash_t h = TypedStringHasher<GeomObject>("GeomObject"); return h; }
static hash_t HashGeomDataset() { static hash_t h = TypedStringHasher<GeomPointcloudDataset>("GeomPointcloudDataset"); return h; }

static String GetNodeName(VfsValue& n) {
	if (IsVfsType(n, HashGeomDirectory())) {
		const GeomDirectory& dir = n.GetExt<GeomDirectory>();
		return dir.name.IsEmpty() ? n.id : dir.name;
	}
	if (IsVfsType(n, HashGeomScene())) {
		const GeomScene& scene = n.GetExt<GeomScene>();
		return scene.name.IsEmpty() ? n.id : scene.name;
	}
	if (IsVfsType(n, HashGeomObject())) {
		const GeomObject& obj = n.GetExt<GeomObject>();
		return obj.name.IsEmpty() ? n.id : obj.name;
	}
	if (IsVfsType(n, HashGeomDataset())) {
		const GeomPointcloudDataset& ds = n.GetExt<GeomPointcloudDataset>();
		return ds.name.IsEmpty() ? n.id : ds.name;
	}
	return n.id;
}

static GeomTransform* GetNodeTransform(VfsValue& n) {
	if (IsVfsType(n, HashGeomObject()))
		return &n.GetExt<GeomObject>().GetTransform();
	if (IsVfsType(n, HashGeomScene()))
		return &n.GetExt<GeomScene>().GetTransform();
	if (IsVfsType(n, HashGeomDirectory()))
		return &n.GetExt<GeomDirectory>().GetTransform();
	return nullptr;
}

static GeomObject* GetNodeObject(VfsValue& n) {
	return IsVfsType(n, HashGeomObject()) ? &n.GetExt<GeomObject>() : nullptr;
}

static GeomDirectory* GetNodeDirectory(VfsValue& n) {
	if (IsVfsType(n, HashGeomScene()))
		return &n.GetExt<GeomScene>();
	if (IsVfsType(n, HashGeomDirectory()))
		return &n.GetExt<GeomDirectory>();
	return nullptr;
}

static bool PyValueToVec3(const PyValue& v, vec3& out) {
	if (v.IsNumber()) {
		double d = v.AsDouble();
		out = vec3(d, d, d);
		return true;
	}
	int n = v.GetCount();
	if (n >= 3) {
		out[0] = v.GetItem(0).AsDouble();
		out[1] = v.GetItem(1).AsDouble();
		out[2] = v.GetItem(2).AsDouble();
		return true;
	}
	return false;
}

static PyValue MakeVec3Value(const vec3& v) {
	PyValue t = PyValue::Tuple();
	t.Add(PyValue(v[0]));
	t.Add(PyValue(v[1]));
	t.Add(PyValue(v[2]));
	return t;
}

static GeomDynamicProperties* FindDynamicProps(VfsValue& node) {
	for (auto& sub : node.sub) {
		if (IsVfsType(sub, AsTypeHash<GeomDynamicProperties>()) && sub.id == "props")
			return &sub.GetExt<GeomDynamicProperties>();
	}
	return nullptr;
}

static GeomDynamicProperties& GetDynamicProps(VfsValue& node) {
	return node.GetAdd<GeomDynamicProperties>("props");
}

static PyValue ValueToPyValue(const Value& v) {
	return PyValue::FromValue(v);
}

static Value PyValueToValue(const PyValue& v) {
	return v.ToValue();
}

struct DisplayObjectProxy : PyUserData {
	Edit3D* app = nullptr;
	VfsValue* node = nullptr;
	PyVM* vm = nullptr;

	DisplayObjectProxy(Edit3D* a, VfsValue* n, PyVM* v) : app(a), node(n), vm(v) {}
	String GetTypeName() const override { return "DisplayObject"; }

	PyValue GetAttr(const String& name) override;
	bool SetAttr(const String& name, const PyValue& v) override;
};

static PyValue MakeDisplayObject(Edit3D* app, VfsValue* node, PyVM* vm) {
	if (!app || !node || !vm)
		return PyValue::None();
	return PyValue(new DisplayObjectProxy(app, node, vm));
}

static VfsValue* FindChildByName(VfsValue& node, const String& name) {
	for (auto& sub : node.sub) {
		if (!IsVfsType(sub, HashGeomDirectory()) && !IsVfsType(sub, HashGeomObject()) && !IsVfsType(sub, HashGeomScene()))
			continue;
		if (GetNodeName(sub) == name || sub.id == name)
			return &sub;
	}
	return nullptr;
}

static VfsValue* ResolvePath(VfsValue& root, const String& path) {
	String p = path;
	if (p.StartsWith("/"))
		p = p.Mid(1);
	if (p.IsEmpty())
		return &root;
	Vector<String> parts = Split(p, '/');
	VfsValue* cur = &root;
	for (const String& part : parts) {
		if (part.IsEmpty())
			continue;
		VfsValue* next = FindChildByName(*cur, part);
		if (!next)
			return nullptr;
		cur = next;
	}
	return cur;
}

static bool NodeTypeMatches(VfsValue& n, const String& type) {
	String t = ToLower(type);
	if (t.IsEmpty())
		return false;
	if (t == "directory" || t == "dir")
		return IsVfsType(n, HashGeomDirectory()) || IsVfsType(n, HashGeomScene());
	if (t == "model") {
		if (!IsVfsType(n, HashGeomObject()))
			return false;
		return n.GetExt<GeomObject>().type == GeomObject::O_MODEL;
	}
	if (t == "camera") {
		if (!IsVfsType(n, HashGeomObject()))
			return false;
		return n.GetExt<GeomObject>().type == GeomObject::O_CAMERA;
	}
	if (t == "pointcloud" || t == "octree") {
		if (!IsVfsType(n, HashGeomObject()))
			return false;
		return n.GetExt<GeomObject>().type == GeomObject::O_OCTREE;
	}
	if (t == "dataset" || t == "pointcloud_dataset")
		return IsVfsType(n, HashGeomDataset());
	if (t == "object")
		return IsVfsType(n, HashGeomObject());
	return false;
}

static int FindChildIndex(VfsValue& parent, VfsValue* child) {
	if (!child)
		return -1;
	for (int i = 0; i < parent.sub.GetCount(); i++)
		if (&parent.sub[i] == child)
			return i;
	return -1;
}

static bool ReorderChild(VfsValue& parent, VfsValue* child, int new_index) {
	int old_index = FindChildIndex(parent, child);
	if (old_index < 0)
		return false;
	if (new_index < 0)
		new_index = 0;
	if (new_index >= parent.sub.GetCount())
		new_index = parent.sub.GetCount() - 1;
	if (new_index == old_index)
		return true;
	parent.sub.Move(old_index, new_index);
	for (auto& sub : parent.sub)
		sub.owner = &parent;
	return true;
}

static PyValue DisplayObject_AddChild(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	String type = args[1].ToString();
	String name = args.GetCount() > 2 ? args[2].ToString() : String();
	if (!self.app || !self.node)
		return PyValue::None();
	GeomDirectory* dir = GetNodeDirectory(*self.node);
	if (!dir)
		return PyValue::None();
	if (name.IsEmpty())
		name = type.IsEmpty() ? "node" : type;
	VfsValue* out = nullptr;
	String t = ToLower(type);
	if (t == "directory" || t == "dir")
		out = &dir->GetAddDirectory(name).val;
	else if (t == "model")
		out = &dir->GetAddModel(name).val;
	else if (t == "camera")
		out = &dir->GetAddCamera(name).val;
	else if (t == "pointcloud" || t == "octree")
		out = &dir->GetAddOctree(name).val;
	else if (t == "dataset" || t == "pointcloud_dataset") {
		GeomPointcloudDataset& ds = dir->GetAddPointcloudDataset(name);
		out = &ds.val;
	}
	if (!out)
		return PyValue::None();
	self.app->state->UpdateObjects();
	self.app->RefreshData();
	return MakeDisplayObject(self.app, out, self.vm);
}

static PyValue DisplayObject_GetChildByName(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	String name = args[1].ToString();
	VfsValue* found = FindChildByName(*self.node, name);
	return MakeDisplayObject(self.app, found, self.vm);
}

static PyValue DisplayObject_GetChildrenByType(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	String type = args[1].ToString();
	PyValue list = PyValue::List();
	for (auto& sub : self.node->sub) {
		if (NodeTypeMatches(sub, type))
			list.Add(MakeDisplayObject(self.app, &sub, self.vm));
	}
	return list;
}

static PyValue DisplayObject_GetChildIndex(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.node)
		return PyValue::None();
	if (!args[1].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& child = (DisplayObjectProxy&)args[1].GetUserData();
	if (!child.node)
		return PyValue::None();
	return PyValue(FindChildIndex(*self.node, child.node));
}

static PyValue DisplayObject_SetChildIndex(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 3 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	if (!args[1].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& child = (DisplayObjectProxy&)args[1].GetUserData();
	if (!child.node)
		return PyValue::None();
	int idx = (int)args[2].AsInt();
	if (!ReorderChild(*self.node, child.node, idx))
		return PyValue::False();
	self.app->RefreshData();
	return PyValue::True();
}

static PyValue DisplayObject_On(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 3 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	String ev = args[1].ToString();
	PyValue fn = args[2];
	self.app->AddScriptEventHandler(ev, self.vm, self.node, fn);
	return PyValue::True();
}

static PyValue DisplayObject_OnFrame(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	PyValue fn = args[1];
	self.app->AddScriptEventHandler("enterFrame", self.vm, self.node, fn);
	return PyValue::True();
}

static bool SetTimelinePosition(Edit3D* app, int frame, bool play) {
	if (!app || !app->anim || !app->prj)
		return false;
	app->anim->position = frame;
	double frame_time = 1.0 / max(1, app->prj->kps);
	app->anim->time = frame * frame_time;
	app->anim->is_playing = play;
	app->RefrehRenderers();
	return true;
}

static PyValue DisplayObject_GotoAndPlay(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	int frame = (int)args[1].AsInt();
	return PyValue(SetTimelinePosition(self.app, frame, true));
}

static PyValue DisplayObject_GotoAndStop(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	int frame = (int)args[1].AsInt();
	return PyValue(SetTimelinePosition(self.app, frame, false));
}

static PyValue DisplayObject_Remove(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 1 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	if (!self.node->owner)
		return PyValue::None();
	self.node->owner->Remove(self.node);
	self.app->state->UpdateObjects();
	self.app->RefreshData();
	return PyValue::True();
}

static PyValue DisplayObject_Find(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	String path = args[1].ToString();
	VfsValue* found = ResolvePath(*self.node, path);
	return MakeDisplayObject(self.app, found, self.vm);
}

static PyValue DisplayObject_Children(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 1 || !args[0].IsUserData())
		return PyValue::None();
	DisplayObjectProxy& self = (DisplayObjectProxy&)args[0].GetUserData();
	if (!self.app || !self.node)
		return PyValue::None();
	PyValue list = PyValue::List();
	for (auto& sub : self.node->sub) {
		if (!IsVfsType(sub, HashGeomDirectory()) && !IsVfsType(sub, HashGeomObject()) && !IsVfsType(sub, HashGeomScene()))
			continue;
		list.Add(MakeDisplayObject(self.app, &sub, self.vm));
	}
	return list;
}

PyValue DisplayObjectProxy::GetAttr(const String& name) {
	if (!node)
		return PyValue::None();
	GeomTransform* tr = GetNodeTransform(*node);
	if (name == "name")
		return PyValue(GetNodeName(*node));
	if (name == "x" && tr)
		return PyValue(tr->position[0]);
	if (name == "y" && tr)
		return PyValue(tr->position[1]);
	if (name == "z" && tr)
		return PyValue(tr->position[2]);
	if (name == "rotation" && tr)
		return MakeVec3Value(GetQuatAxes(tr->orientation));
	if (name == "scale" && tr)
		return MakeVec3Value(tr->scale);
	if (name == "visible") {
		if (GeomObject* obj = GetNodeObject(*node))
			return PyValue(obj->is_visible);
	}
	if (name == "addChild")
		return PyValue::BoundMethod(PyValue::Function("addChild", DisplayObject_AddChild, nullptr), PyValue(this));
	if (name == "remove")
		return PyValue::BoundMethod(PyValue::Function("remove", DisplayObject_Remove, nullptr), PyValue(this));
	if (name == "find")
		return PyValue::BoundMethod(PyValue::Function("find", DisplayObject_Find, nullptr), PyValue(this));
	if (name == "children")
		return PyValue::BoundMethod(PyValue::Function("children", DisplayObject_Children, nullptr), PyValue(this));
	if (name == "getChildByName")
		return PyValue::BoundMethod(PyValue::Function("getChildByName", DisplayObject_GetChildByName, nullptr), PyValue(this));
	if (name == "getChildrenByType")
		return PyValue::BoundMethod(PyValue::Function("getChildrenByType", DisplayObject_GetChildrenByType, nullptr), PyValue(this));
	if (name == "getChildIndex")
		return PyValue::BoundMethod(PyValue::Function("getChildIndex", DisplayObject_GetChildIndex, nullptr), PyValue(this));
	if (name == "setChildIndex")
		return PyValue::BoundMethod(PyValue::Function("setChildIndex", DisplayObject_SetChildIndex, nullptr), PyValue(this));
	if (name == "on")
		return PyValue::BoundMethod(PyValue::Function("on", DisplayObject_On, nullptr), PyValue(this));
	if (name == "onFrame" || name == "onEnterFrame")
		return PyValue::BoundMethod(PyValue::Function("onFrame", DisplayObject_OnFrame, nullptr), PyValue(this));
	if (name == "gotoAndPlay")
		return PyValue::BoundMethod(PyValue::Function("gotoAndPlay", DisplayObject_GotoAndPlay, nullptr), PyValue(this));
	if (name == "gotoAndStop")
		return PyValue::BoundMethod(PyValue::Function("gotoAndStop", DisplayObject_GotoAndStop, nullptr), PyValue(this));
	if (GeomDynamicProperties* props = FindDynamicProps(*node)) {
		int idx = props->props.Find(name);
		if (idx >= 0)
			return ValueToPyValue(props->props[idx]);
	}
	return PyValue::None();
}

bool DisplayObjectProxy::SetAttr(const String& name, const PyValue& v) {
	if (!node)
		return false;
	GeomTransform* tr = GetNodeTransform(*node);
	if (tr) {
		if (name == "x") { tr->position[0] = v.AsDouble(); if (app) { app->state->UpdateObjects(); app->RefrehRenderers(); } return true; }
		else if (name == "y") { tr->position[1] = v.AsDouble(); if (app) { app->state->UpdateObjects(); app->RefrehRenderers(); } return true; }
		else if (name == "z") { tr->position[2] = v.AsDouble(); if (app) { app->state->UpdateObjects(); app->RefrehRenderers(); } return true; }
		else if (name == "rotation") {
			vec3 axes;
			if (PyValueToVec3(v, axes)) {
				tr->orientation = AxesQuat(axes);
				if (app) { app->state->UpdateObjects(); app->RefrehRenderers(); }
				return true;
			}
		}
		else if (name == "scale") {
			vec3 scale;
			if (PyValueToVec3(v, scale)) {
				tr->scale = scale;
				if (app) { app->state->UpdateObjects(); app->RefrehRenderers(); }
				return true;
			}
		}
	}
	if (name == "onFrame" || name == "onEnterFrame") {
		if (app)
			app->AddScriptEventHandler("enterFrame", vm, node, v);
		return true;
	}
	if (name == "visible") {
		if (GeomObject* obj = GetNodeObject(*node))
			obj->is_visible = v.IsTrue();
		if (GeomDirectory* dir = GetNodeDirectory(*node)) {
			bool vis = v.IsTrue();
			for (auto& sub : dir->val.sub) {
				if (IsVfsType(sub, HashGeomObject()))
					sub.GetExt<GeomObject>().is_visible = vis;
			}
		}
	}
	else if (GeomDynamicProperties* props = node ? &GetDynamicProps(*node) : nullptr) {
		props->props.GetAdd(name) = PyValueToValue(v);
	}
	if (app) {
		app->state->UpdateObjects();
		app->RefrehRenderers();
	}
	return true;
}

struct StageProxy : PyUserData {
	Edit3D* app = nullptr;
	PyVM* vm = nullptr;
	StageProxy(Edit3D* a, PyVM* v) : app(a), vm(v) {}
	String GetTypeName() const override { return "Stage"; }
	PyValue GetAttr(const String& name) override;
	bool SetAttr(const String& name, const PyValue& v) override;
};

static PyValue Stage_Find(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	if (!self.app)
		return PyValue::None();
	GeomScene& scene = self.app->state->GetActiveScene();
	String path = args[1].ToString();
	VfsValue* found = ResolvePath(scene.val, path);
	return MakeDisplayObject(self.app, found, self.vm);
}

static PyValue Stage_Children(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 1 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	if (!self.app)
		return PyValue::None();
	VfsValue* root = &self.app->state->GetActiveScene().val;
	if (args.GetCount() > 1 && args[1].IsUserData())
		root = ((DisplayObjectProxy&)args[1].GetUserData()).node;
	PyValue list = PyValue::List();
	if (root) {
		for (auto& sub : root->sub) {
			if (!IsVfsType(sub, HashGeomDirectory()) && !IsVfsType(sub, HashGeomObject()) && !IsVfsType(sub, HashGeomScene()))
				continue;
			list.Add(MakeDisplayObject(self.app, &sub, self.vm));
		}
	}
	return list;
}

static PyValue Stage_Create(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	if (!self.app)
		return PyValue::None();
	String type = args[1].ToString();
	String name = args.GetCount() > 2 ? args[2].ToString() : String();
	VfsValue* parent = &self.app->state->GetActiveScene().val;
	if (args.GetCount() > 3 && args[3].IsUserData())
		parent = ((DisplayObjectProxy&)args[3].GetUserData()).node;
	if (!parent)
		return PyValue::None();
	GeomDirectory* dir = GetNodeDirectory(*parent);
	if (!dir)
		return PyValue::None();
	if (name.IsEmpty())
		name = type.IsEmpty() ? "node" : type;
	VfsValue* out = nullptr;
	String t = ToLower(type);
	if (t == "directory" || t == "dir")
		out = &dir->GetAddDirectory(name).val;
	else if (t == "model")
		out = &dir->GetAddModel(name).val;
	else if (t == "camera")
		out = &dir->GetAddCamera(name).val;
	else if (t == "pointcloud" || t == "octree")
		out = &dir->GetAddOctree(name).val;
	else if (t == "dataset" || t == "pointcloud_dataset") {
		GeomPointcloudDataset& ds = dir->GetAddPointcloudDataset(name);
		out = &ds.val;
	}
	if (!out)
		return PyValue::None();
	self.app->state->UpdateObjects();
	self.app->RefreshData();
	return MakeDisplayObject(self.app, out, self.vm);
}

static PyValue Stage_Goto(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	int frame = (int)args[1].AsInt();
	return PyValue(SetTimelinePosition(self.app, frame, self.app && self.app->anim ? self.app->anim->is_playing : false));
}

static PyValue Stage_GotoAndPlay(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	int frame = (int)args[1].AsInt();
	return PyValue(SetTimelinePosition(self.app, frame, true));
}

static PyValue Stage_GotoAndStop(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	int frame = (int)args[1].AsInt();
	return PyValue(SetTimelinePosition(self.app, frame, false));
}

static PyValue Stage_On(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 3 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	if (!self.app)
		return PyValue::None();
	String ev = args[1].ToString();
	PyValue fn = args[2];
	self.app->AddScriptEventHandler(ev, self.vm, nullptr, fn);
	return PyValue::True();
}

static PyValue Stage_OnFrame(const Vector<PyValue>& args, void* user_data) {
	if (args.GetCount() < 2 || !args[0].IsUserData())
		return PyValue::None();
	StageProxy& self = (StageProxy&)args[0].GetUserData();
	if (!self.app)
		return PyValue::None();
	PyValue fn = args[1];
	self.app->AddScriptEventHandler("enterFrame", self.vm, nullptr, fn);
	return PyValue::True();
}

PyValue StageProxy::GetAttr(const String& name) {
	if (!app)
		return PyValue::None();
	if (name == "root")
		return MakeDisplayObject(app, &app->state->GetActiveScene().val, vm);
	if (name == "time")
		return PyValue(app->anim ? app->anim->time : 0.0);
	if (name == "frame")
		return PyValue(app->anim ? app->anim->position : 0);
	if (name == "fps")
		return PyValue(app->prj ? app->prj->fps : 0);
	if (name == "find")
		return PyValue::BoundMethod(PyValue::Function("find", Stage_Find, nullptr), PyValue(this));
	if (name == "children")
		return PyValue::BoundMethod(PyValue::Function("children", Stage_Children, nullptr), PyValue(this));
	if (name == "create")
		return PyValue::BoundMethod(PyValue::Function("create", Stage_Create, nullptr), PyValue(this));
	if (name == "goto")
		return PyValue::BoundMethod(PyValue::Function("goto", Stage_Goto, nullptr), PyValue(this));
	if (name == "gotoAndPlay")
		return PyValue::BoundMethod(PyValue::Function("gotoAndPlay", Stage_GotoAndPlay, nullptr), PyValue(this));
	if (name == "gotoAndStop")
		return PyValue::BoundMethod(PyValue::Function("gotoAndStop", Stage_GotoAndStop, nullptr), PyValue(this));
	if (name == "on")
		return PyValue::BoundMethod(PyValue::Function("on", Stage_On, nullptr), PyValue(this));
	if (name == "onFrame" || name == "onEnterFrame")
		return PyValue::BoundMethod(PyValue::Function("onFrame", Stage_OnFrame, nullptr), PyValue(this));
	return PyValue::None();
}

bool StageProxy::SetAttr(const String& name, const PyValue& v) {
	if (!app || !app->anim || !app->prj)
		return false;
	if (name == "frame") {
		int frame = (int)v.AsInt();
		app->anim->position = frame;
		double frame_time = 1.0 / max(1, app->prj->kps);
		app->anim->time = frame * frame_time;
		app->RefrehRenderers();
		return true;
	}
	if (name == "onFrame" || name == "onEnterFrame") {
		app->AddScriptEventHandler("enterFrame", vm, nullptr, v);
		return true;
	}
	return false;
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
	script_timer.Reset();
	
	anim->WhenSceneEnd << THISBACK(OnSceneEnd);
	
	Sizeable().MaximizeBox();
	Title("Edit3D");
	scene3d_data_dir = "data";
	SetProjectDir(GetCurrentDirectory());
	view = VIEW_GEOMPROJECT;
	Add(v0.grid.SizePos());
	
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
			bar.Separator();
			bar.Sub(t_("Tree"), [this](Bar& bar) { v0.TreeMenu(bar); });
		});
		bar.Sub(t_("Edit"), [this](Bar& bar) {
			bar.Add(t_("Create Editable Mesh"), THISBACK(CreateEditableMeshObject));
			bar.Add(t_("Create 2D Layer"), THISBACK(Create2DLayerObject));
			bar.Separator();
			bar.Add(t_("Select Tool"), THISBACK1(SetEditTool, TOOL_SELECT))
				.Check(edit_tool == TOOL_SELECT);
			bar.Add(t_("Point Tool"), THISBACK1(SetEditTool, TOOL_POINT))
				.Check(edit_tool == TOOL_POINT);
			bar.Add(t_("Line Tool"), THISBACK1(SetEditTool, TOOL_LINE))
				.Check(edit_tool == TOOL_LINE);
			bar.Add(t_("Face Tool"), THISBACK1(SetEditTool, TOOL_FACE))
				.Check(edit_tool == TOOL_FACE);
			bar.Add(t_("Erase Tool"), THISBACK1(SetEditTool, TOOL_ERASE))
				.Check(edit_tool == TOOL_ERASE);
			bar.Add(t_("Join Tool"), THISBACK1(SetEditTool, TOOL_JOIN))
				.Check(edit_tool == TOOL_JOIN);
			bar.Add(t_("Split Tool"), THISBACK1(SetEditTool, TOOL_SPLIT))
				.Check(edit_tool == TOOL_SPLIT);
			bar.Separator();
			bar.Sub(t_("2D Tools"), [this](Bar& bar) {
				bar.Add(t_("2D Line"), THISBACK1(SetEditTool, TOOL_2D_LINE))
					.Check(edit_tool == TOOL_2D_LINE);
				bar.Add(t_("2D Rectangle"), THISBACK1(SetEditTool, TOOL_2D_RECT))
					.Check(edit_tool == TOOL_2D_RECT);
				bar.Add(t_("2D Circle"), THISBACK1(SetEditTool, TOOL_2D_CIRCLE))
					.Check(edit_tool == TOOL_2D_CIRCLE);
				bar.Add(t_("2D Polygon"), THISBACK1(SetEditTool, TOOL_2D_POLY))
					.Check(edit_tool == TOOL_2D_POLY);
				bar.Add(t_("2D Erase"), THISBACK1(SetEditTool, TOOL_2D_ERASE))
					.Check(edit_tool == TOOL_2D_ERASE);
			});
			bar.Separator();
			bar.Sub(t_("Plane"), [this](Bar& bar) {
				bar.Add(t_("View Plane"), [this] { edit_plane = PLANE_VIEW; })
					.Check(edit_plane == PLANE_VIEW);
				bar.Add(t_("XY"), [this] { edit_plane = PLANE_XY; })
					.Check(edit_plane == PLANE_XY);
				bar.Add(t_("XZ"), [this] { edit_plane = PLANE_XZ; })
					.Check(edit_plane == PLANE_XZ);
				bar.Add(t_("YZ"), [this] { edit_plane = PLANE_YZ; })
					.Check(edit_plane == PLANE_YZ);
				bar.Add(t_("Local"), [this] { edit_plane = PLANE_LOCAL; })
					.Check(edit_plane == PLANE_LOCAL);
			});
			bar.Sub(t_("Snap"), [this](Bar& bar) {
				bar.Add(t_("Enable"), [this] { edit_snap_enable = !edit_snap_enable; })
					.Check(edit_snap_enable);
				bar.Add(t_("Local Axes"), [this] { edit_snap_local = !edit_snap_local; })
					.Check(edit_snap_local);
				bar.Separator();
				bar.Add(t_("Step 0.1"), [this] { edit_snap_step = 0.1; })
					.Check(fabs(edit_snap_step - 0.1) < 1e-6);
				bar.Add(t_("Step 0.5"), [this] { edit_snap_step = 0.5; })
					.Check(fabs(edit_snap_step - 0.5) < 1e-6);
				bar.Add(t_("Step 1.0"), [this] { edit_snap_step = 1.0; })
					.Check(fabs(edit_snap_step - 1.0) < 1e-6);
			});
			bar.Sub(t_("Sculpt"), [this](Bar& bar) {
				bar.Add(t_("Enable"), [this] { sculpt_mode = !sculpt_mode; })
					.Check(sculpt_mode);
				bar.Separator();
				bar.Add(t_("Add"), [this] { sculpt_add = true; })
					.Check(sculpt_add);
				bar.Add(t_("Subtract"), [this] { sculpt_add = false; })
					.Check(!sculpt_add);
				bar.Separator();
				bar.Add(t_("Radius 0.25"), [this] { sculpt_radius = 0.25; })
					.Check(fabs(sculpt_radius - 0.25) < 1e-6);
				bar.Add(t_("Radius 0.5"), [this] { sculpt_radius = 0.5; })
					.Check(fabs(sculpt_radius - 0.5) < 1e-6);
				bar.Add(t_("Radius 1.0"), [this] { sculpt_radius = 1.0; })
					.Check(fabs(sculpt_radius - 1.0) < 1e-6);
				bar.Separator();
				bar.Add(t_("Strength 0.05"), [this] { sculpt_strength = 0.05; })
					.Check(fabs(sculpt_strength - 0.05) < 1e-6);
				bar.Add(t_("Strength 0.2"), [this] { sculpt_strength = 0.2; })
					.Check(fabs(sculpt_strength - 0.2) < 1e-6);
				bar.Add(t_("Strength 0.5"), [this] { sculpt_strength = 0.5; })
					.Check(fabs(sculpt_strength - 0.5) < 1e-6);
			});
			bar.Sub(t_("Mesh Animation"), [this](Bar& bar) {
				bar.Add(t_("Add Keyframe"), THISBACK(AddMeshKeyframeAtCursor));
				bar.Add(t_("Clear Keyframes"), THISBACK(ClearMeshKeyframes));
			});
			bar.Sub(t_("2D Animation"), [this](Bar& bar) {
				bar.Add(t_("Add Keyframe"), THISBACK(Add2DKeyframeAtCursor));
				bar.Add(t_("Clear Keyframes"), THISBACK(Clear2DKeyframes));
			});
			bar.Sub(t_("Skeleton"), [this](Bar& bar) {
				bar.Add(t_("Create Skeleton"), THISBACK(CreateSkeletonForSelected));
				bar.Add(t_("Add Bone"), THISBACK(AddBoneToSelectedSkeleton));
				bar.Add(t_("Remove Bone"), THISBACK(RemoveSelectedBone));
			});
			bar.Sub(t_("Weights"), [this](Bar& bar) {
				bar.Add(t_("Enable Paint"), [this] { SetWeightPaintMode(!weight_paint_mode); })
					.Check(weight_paint_mode);
				bar.Separator();
				bar.Add(t_("Add"), [this] { weight_add = true; })
					.Check(weight_add);
				bar.Add(t_("Subtract"), [this] { weight_add = false; })
					.Check(!weight_add);
				bar.Separator();
				bar.Add(t_("Radius 0.25"), [this] { weight_radius = 0.25; })
					.Check(fabs(weight_radius - 0.25) < 1e-6);
				bar.Add(t_("Radius 0.5"), [this] { weight_radius = 0.5; })
					.Check(fabs(weight_radius - 0.5) < 1e-6);
				bar.Add(t_("Radius 1.0"), [this] { weight_radius = 1.0; })
					.Check(fabs(weight_radius - 1.0) < 1e-6);
				bar.Separator();
				bar.Add(t_("Strength 0.05"), [this] { weight_strength = 0.05; })
					.Check(fabs(weight_strength - 0.05) < 1e-6);
				bar.Add(t_("Strength 0.2"), [this] { weight_strength = 0.2; })
					.Check(fabs(weight_strength - 0.2) < 1e-6);
				bar.Add(t_("Strength 0.5"), [this] { weight_strength = 0.5; })
					.Check(fabs(weight_strength - 0.5) < 1e-6);
			});
		});
		bar.Sub(t_("Windows"), [this](Bar& bar) { DockWindowMenu(bar); });
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

void Edit3D::DockInit() {
	dock_tree = &Dockable(v0.tree, "Scene Tree");
	dock_props = &Dockable(v0.props, "Properties");
	dock_time = &Dockable(v0.time, "Timeline");
	dock_video = &Dockable(v1, "Video Import");
	dock_pool = &Dockable(file_pool, "File Pool");

	dock_tree->SizeHint(Size(260, 600));
	dock_props->SizeHint(Size(360, 600));
	dock_time->SizeHint(Size(900, 200));
	dock_video->SizeHint(Size(900, 300));
	dock_pool->SizeHint(Size(320, 600));

	DockLeft(*dock_tree, 0);
	DockLeft(*dock_props, 1);
	DockBottom(*dock_time);
	DockBottom(*dock_video);
	DockRight(*dock_pool);

	Close(*dock_video);
	Close(*dock_pool);
	v0.grid.SetFocus();
}

void Edit3D::SetView(ViewType view) {
	this->view = view;
	if (this->view == VIEW_GEOMPROJECT) {
		if (dock_video)
			Close(*dock_video);
		v0.grid.Show();
		v0.grid.SetFocus();
	}
	else if (this->view == VIEW_VIDEOIMPORT) {
		v0.grid.Hide();
		if (dock_video)
			ActivateDockableChild(v1);
	}
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
	TopWindow::Close();
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
	
	v0.Update(dt);
	if (dock_video && !dock_video->IsHidden())
		v1.Update(dt);
	if (conf.dump_grid_done && !conf.dump_grid_path.IsEmpty()) {
		conf.dump_grid_path.Clear();
		PostCallback(THISBACK(Exit));
	}
	
	if (hmd.IsRunning()) {
		hmd.Poll();
		if (record_pointcloud)
			UpdateHmdCameraPose();
	}

	if (state && state->HasActiveScene()) {
		GeomScene& scene = state->GetActiveScene();
		GeomSceneTimeline& tl = scene.GetTimeline();
		bool was_playing = tl.is_playing;
		if (tl.is_playing)
			tl.Update(*state, dt);
		if (tl.is_playing || was_playing)
			RefrehRenderers();
	}

	EnsureScriptInstances();
	for (auto& inst : script_instances)
		UpdateScriptInstance(inst, false);
	for (auto& inst : script_instances)
		RunScriptFrame(inst, dt);
	DispatchFrameEvents(dt);
}

void Edit3D::Data() {
	v0.Data();
	if (dock_video && !dock_video->IsHidden())
		v1.Data();
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
		String header;
		header << "# Script: " << base_name << "\n";
		header << "# Example:\n";
		header << "# def on_start():\n";
		header << "#     modeler.debug_generate_pointcloud()\n";
		header << "#     modeler.debug_simulate_observation()\n";
		header << "#     modeler.debug_run_localization()\n";
		header << "#\n";
		header << "# def on_frame(dt):\n";
		header << "#     pass\n";
		SaveFile(abs, header);
	}
	return rel;
}

GeomScript& Edit3D::AddScriptComponent(GeomObject& obj) {
	String id = "script";
	int idx = 1;
	hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
	while (obj.val.Find(id, script_hash) >= 0)
		id = "script_" + IntStr(idx++);
	VfsValue& node = obj.val.Add(id, script_hash);
	GeomScript& script = node.GetExt<GeomScript>();
	EnsureScriptFile(script, obj.name.IsEmpty() ? id : obj.name);
	EnsureScriptInstances();
	return script;
}

GeomScript& Edit3D::AddScriptComponent(GeomDirectory& dir) {
	String id = "script";
	int idx = 1;
	hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
	while (dir.val.Find(id, script_hash) >= 0)
		id = "script_" + IntStr(idx++);
	VfsValue& node = dir.val.Add(id, script_hash);
	GeomScript& script = node.GetExt<GeomScript>();
	String base = dir.name.IsEmpty() ? id : dir.name;
	EnsureScriptFile(script, base);
	EnsureScriptInstances();
	return script;
}

GeomScript& Edit3D::AddScriptComponent(GeomScene& scene) {
	String id = "script";
	int idx = 1;
	hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
	while (scene.val.Find(id, script_hash) >= 0)
		id = "script_" + IntStr(idx++);
	VfsValue& node = scene.val.Add(id, script_hash);
	GeomScript& script = node.GetExt<GeomScript>();
	String base = scene.name.IsEmpty() ? id : scene.name;
	EnsureScriptFile(script, base);
	EnsureScriptInstances();
	return script;
}

void Edit3D::GetScriptsFromNode(VfsValue& node, Vector<GeomScript*>& out) {
	hash_t script_hash = TypedStringHasher<GeomScript>("GeomScript");
	for (auto& sub : node.sub) {
		if (IsVfsType(sub, script_hash))
			out.Add(&sub.GetExt<GeomScript>());
	}
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
			RunScriptOnLoad(inst, !script.run_on_load);
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
	PY_MODULE_FUNC(trace, ModelerTrace, this);
	PY_MODULE_FUNC(get_timer, ModelerGetTimer, this);
	PY_MODULE_FUNC(random, ModelerRandom, this);
	PyValue stage_obj = PyValue(new StageProxy(this, &vm));
	vm.GetGlobals().GetAdd(PyValue("stage")) = stage_obj;
	vm.GetGlobals().GetAdd(PyValue("trace")) = PyValue::Function("trace", ModelerTrace, this);
	vm.GetGlobals().GetAdd(PyValue("getTimer")) = PyValue::Function("getTimer", ModelerGetTimer, this);
	vm.GetGlobals().GetAdd(PyValue("random")) = PyValue::Function("random", ModelerRandom, this);
	PyValue root_obj = PyValue::None();
	if (state)
		root_obj = MakeDisplayObject(this, &state->GetActiveScene().val, &vm);
	vm.GetGlobals().GetAdd(PyValue("root")) = root_obj;
	vm.GetGlobals().GetAdd(PyValue("this")) = root_obj;
}

void Edit3D::EnsureScriptInstances() {
	if (!state || !state->prj || state->active_scene < 0 || state->active_scene >= state->prj->GetSceneCount())
		return;
	GeomScene& scene = GetActiveScene();
	Vector<GeomScript*> scripts;
	GetScriptsFromNode(scene.val, scripts);
	for (auto& sub : scene.val.sub) {
		hash_t dir_hash = TypedStringHasher<GeomDirectory>("GeomDirectory");
		if (IsVfsType(sub, dir_hash))
			GetScriptsFromNode(sub, scripts);
	}
	for (GeomObject& obj : GeomObjectCollection(scene))
		GetScriptsFromNode(obj.val, scripts);
	for (int i = script_instances.GetCount() - 1; i >= 0; i--) {
		bool found = false;
		for (int j = 0; j < scripts.GetCount(); j++) {
			if (scripts[j] == script_instances[i].script) {
				found = true;
				break;
			}
		}
		if (!found) {
			RemoveScriptEventHandlers(&script_instances[i].vm);
			script_instances.Remove(i);
		}
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
			inst.owner = script->val.owner;
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
	RemoveScriptEventHandlers(&inst.vm);
	inst.file_time = mod;
	inst.loaded = false;
	inst.has_load = false;
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
	if (state) {
		PyValue root_obj = MakeDisplayObject(this, &state->GetActiveScene().val, &inst.vm);
		inst.vm.GetGlobals().GetAdd(PyValue("root")) = root_obj;
		PyValue this_obj = root_obj;
		if (inst.owner) {
			PyValue owner_obj = MakeDisplayObject(this, inst.owner, &inst.vm);
			if (!owner_obj.IsNone())
				this_obj = owner_obj;
		}
		inst.vm.GetGlobals().GetAdd(PyValue("this")) = this_obj;
	}
	inst.vm.GetGlobals().GetAdd(PyValue("__project_dir__")) = PyValue(project_dir);
	inst.vm.GetGlobals().GetAdd(PyValue("__script_path__")) = PyValue(abs);
	if (!RunPyIR(inst.vm, ir, err)) {
		LOG("Script run failed: " + err);
		return;
	}
	inst.loaded = true;
	inst.main_ir = pick(ir);
	int on_load_idx = inst.vm.GetGlobals().Find(PyValue("on_load"));
	int on_start_idx = inst.vm.GetGlobals().Find(PyValue("on_start"));
	int on_frame_idx = inst.vm.GetGlobals().Find(PyValue("on_frame"));
	inst.has_load = on_load_idx >= 0 && !inst.vm.GetGlobals()[on_load_idx].IsNone();
	inst.has_start = on_start_idx >= 0 && !inst.vm.GetGlobals()[on_start_idx].IsNone();
	inst.has_frame = on_frame_idx >= 0 && !inst.vm.GetGlobals()[on_frame_idx].IsNone();
	if (inst.has_load) {
		Vector<PyIR> load_ir;
		if (CompilePySource("on_load()", abs, load_ir, err))
			inst.load_ir = pick(load_ir);
	}
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
	RunScriptOnLoad(inst, false);
	RunScriptOnStart(inst, false);
}

void Edit3D::RunScriptOnLoad(ScriptInstance& inst, bool force) {
	if (!inst.script || !inst.loaded)
		return;
	if (!force && !inst.script->run_on_load)
		return;
	if (!inst.has_load)
		return;
	String err;
	if (!RunPyIR(inst.vm, inst.load_ir, err))
		LOG("Script on_load failed: " + err);
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
	if (!inst.loaded || !inst.has_frame)
		return;
	inst.vm.GetGlobals().GetAdd(PyValue("__dt__")) = PyValue(dt);
	String err;
	if (!RunPyIR(inst.vm, inst.frame_ir, err))
		LOG("Script on_frame failed: " + err);
}

void Edit3D::AddScriptEventHandler(const String& event, PyVM* vm, VfsValue* node, const PyValue& func) {
	if (!vm || (!func.IsFunction() && !func.IsBoundMethod()))
		return;
	ScriptEventHandler& h = script_event_handlers.Add();
	h.event = ToLower(event);
	h.func = func;
	h.vm = vm;
	h.node = node;
}

void Edit3D::RemoveScriptEventHandlers(PyVM* vm) {
	if (!vm)
		return;
	for (int i = script_event_handlers.GetCount() - 1; i >= 0; i--) {
		if (script_event_handlers[i].vm == vm)
			script_event_handlers.Remove(i);
	}
}

static bool RunPyCallback(PyVM& vm, const PyValue& func, const Vector<PyValue>& args, String& err) {
	if (!func.IsFunction() && !func.IsBoundMethod())
		return false;
	Vector<PyIR> ir;
	ir.Add(PyIR(PY_LOAD_CONST, func));
	for (const PyValue& a : args)
		ir.Add(PyIR(PY_LOAD_CONST, a));
	ir.Add(PyIR(PY_CALL_FUNCTION, (int)args.GetCount(), 0));
	ir.Add(PyIR(PY_RETURN_VALUE));
	return RunPyIR(vm, ir, err);
}

void Edit3D::DispatchScriptEvent(const String& event, VfsValue* node, const PyValue& payload) {
	String key = ToLower(event);
	for (const ScriptEventHandler& h : script_event_handlers) {
		if (h.event != key)
			continue;
		if (node && h.node && h.node != node)
			continue;
		if (!node && h.node) {
			// allow global dispatch to reach node handlers (enterFrame).
		}
		if (!h.vm)
			continue;
		Vector<PyValue> args;
		args.Add(payload);
		String err;
		if (!RunPyCallback(*h.vm, h.func, args, err))
			LOG("Script event failed: " + err);
	}
}

void Edit3D::DispatchInputEvent(const String& type, const Point& p, dword flags, int key, int view_i) {
	auto is_2d_tool = [&](EditTool t) {
		return t == TOOL_2D_LINE || t == TOOL_2D_RECT || t == TOOL_2D_CIRCLE ||
		       t == TOOL_2D_POLY || t == TOOL_2D_ERASE;
	};
	auto get_2d_local = [&](GeomObject& obj, vec2& out) -> bool {
		vec3 plane_origin = vec3(0);
		vec3 plane_normal = vec3(0, 0, 1);
		vec3 obj_pos = vec3(0);
		quat obj_ori = Identity<quat>();
		vec3 obj_scale = vec3(1);
		if (state) {
			if (const GeomObjectState* os = state->FindObjectStateByKey(obj.key)) {
				obj_pos = os->position;
				obj_ori = os->orientation;
				obj_scale = os->scale;
			}
		}
		else if (GeomTransform* tr = obj.FindTransform()) {
			obj_pos = tr->position;
			obj_ori = tr->orientation;
			obj_scale = tr->scale;
		}
		plane_origin = obj_pos;
		auto set_plane_from_view = [&] {
			if (view_i < 0 || view_i >= 4)
				return;
			EditRendererBase* rend = v0.rends[view_i];
			if (!rend)
				return;
			switch (rend->view_mode) {
			case VIEWMODE_XY: plane_normal = vec3(0, 0, 1); break;
			case VIEWMODE_XZ: plane_normal = vec3(0, 1, 0); break;
			case VIEWMODE_YZ: plane_normal = vec3(1, 0, 0); break;
			case VIEWMODE_PERSPECTIVE:
				plane_normal = VectorTransform(VEC_FWD, obj_ori);
				break;
			default: break;
			}
		};
		switch (edit_plane) {
		case PLANE_XY: plane_normal = vec3(0, 0, 1); break;
		case PLANE_XZ: plane_normal = vec3(0, 1, 0); break;
		case PLANE_YZ: plane_normal = vec3(1, 0, 0); break;
		case PLANE_LOCAL: plane_normal = VectorTransform(VEC_FWD, obj_ori); break;
		default: set_plane_from_view(); break;
		}
		vec3 world;
		if (!ScreenToPlaneWorldPoint(view_i, p, plane_origin, plane_normal, world))
			return false;
		quat inv = obj_ori.GetInverse();
		vec3 v = VectorTransform(world - obj_pos, inv);
		if (obj_scale[0] != 0) v[0] /= obj_scale[0];
		if (obj_scale[1] != 0) v[1] /= obj_scale[1];
		if (obj_scale[2] != 0) v[2] /= obj_scale[2];
		out = vec2(v[0], v[1]);
		return true;
	};

	if (view == VIEW_GEOMPROJECT && is_2d_tool(edit_tool)) {
		GeomObject* obj = v0.selected_obj;
		if (!obj)
			return;
		Geom2DLayer* layer = obj->Find2DLayer();
		if (!layer)
			return;
		vec2 local;
		if (!get_2d_local(*obj, local))
			return;
		if (type == "mouseDown") {
			if (edit_tool == TOOL_2D_ERASE) {
				double best = edit_pick_radius_px * edit_pick_radius_px;
				int best_idx = -1;
				auto dist2_seg = [](const vec2& a, const vec2& b, const vec2& p) {
					vec2 d = b - a;
					float len2 = Dot(d, d);
					if (len2 <= 1e-6f) {
						vec2 s = p - a;
						return (double)Dot(s, s);
					}
					float t = Dot(p - a, d) / len2;
					t = Clamp(t, 0.0f, 1.0f);
					vec2 c = a + d * t;
					vec2 s = p - c;
					return (double)Dot(s, s);
				};
				for (int i = 0; i < layer->shapes.GetCount(); i++) {
					const Geom2DShape& s = layer->shapes[i];
					if (s.type == Geom2DShape::S_LINE && s.points.GetCount() >= 2) {
						double d2 = dist2_seg(s.points[0], s.points[1], local);
						if (d2 < best) {
							best = d2;
							best_idx = i;
						}
					}
					else if ((s.type == Geom2DShape::S_RECT || s.type == Geom2DShape::S_POLY) && s.points.GetCount() >= 2) {
						for (int k = 1; k < s.points.GetCount(); k++) {
							double d2 = dist2_seg(s.points[k - 1], s.points[k], local);
							if (d2 < best) {
								best = d2;
								best_idx = i;
							}
						}
					}
					else if (s.type == Geom2DShape::S_CIRCLE && s.points.GetCount() >= 1) {
						float r = s.radius;
						if (r <= 0 && s.points.GetCount() >= 2) {
							vec2 d2 = s.points[1] - s.points[0];
							r = sqrt(Dot(d2, d2));
						}
						if (r > 0) {
							vec2 diff = local - s.points[0];
							float d = sqrt(Dot(diff, diff));
							double d2 = (double)fabs(d - r);
							if (d2 < best) {
								best = d2;
								best_idx = i;
							}
						}
					}
				}
				if (best_idx >= 0) {
					layer->shapes.Remove(best_idx);
					RefrehRenderers();
				}
				return;
			}
			if (edit_tool == TOOL_2D_POLY) {
				if (!draw2d_active) {
					draw2d_poly.Clear();
					draw2d_poly.Add(local);
					draw2d_active = true;
					draw2d_obj = obj;
				}
				else {
					vec2 diff = draw2d_poly[0] - local;
					float d = sqrt(Dot(diff, diff));
					if (draw2d_poly.GetCount() >= 3 && d < 0.05f) {
						Geom2DShape shape;
						shape.type = Geom2DShape::S_POLY;
						shape.points.SetCount(draw2d_poly.GetCount());
						for (int i = 0; i < draw2d_poly.GetCount(); i++)
							shape.points[i] = draw2d_poly[i];
						shape.closed = true;
						layer->shapes.Add(pick(shape));
						draw2d_poly.Clear();
						draw2d_active = false;
						draw2d_obj = nullptr;
						RefrehRenderers();
					}
					else {
						draw2d_poly.Add(local);
					}
				}
				return;
			}
			draw2d_active = true;
			draw2d_start = local;
			draw2d_last = local;
			draw2d_obj = obj;
			draw2d_view = view_i;
			return;
		}
		if (type == "mouseMove" && draw2d_active && draw2d_obj == obj) {
			draw2d_last = local;
			return;
		}
		if (type == "mouseUp" && draw2d_active && draw2d_obj == obj) {
			Geom2DShape shape;
			switch (edit_tool) {
			case TOOL_2D_LINE:
				shape.type = Geom2DShape::S_LINE;
				shape.points.Add(draw2d_start);
				shape.points.Add(draw2d_last);
				break;
			case TOOL_2D_RECT:
				shape.type = Geom2DShape::S_RECT;
				shape.points.Add(draw2d_start);
				shape.points.Add(draw2d_last);
				break;
			case TOOL_2D_CIRCLE:
				shape.type = Geom2DShape::S_CIRCLE;
				shape.points.Add(draw2d_start);
				shape.points.Add(draw2d_last);
				{
					vec2 diff = draw2d_last - draw2d_start;
					shape.radius = sqrt(Dot(diff, diff));
				}
				break;
			default:
				break;
			}
			if (shape.points.GetCount() >= 2)
				layer->shapes.Add(pick(shape));
			draw2d_active = false;
			draw2d_obj = nullptr;
			RefrehRenderers();
			return;
		}
	}

	if (view == VIEW_GEOMPROJECT && type == "mouseDown" && edit_tool != TOOL_SELECT) {
		GeomObject* obj = v0.selected_obj;
		if (obj) {
			vec3 plane_origin = vec3(0);
			vec3 plane_normal = vec3(0, 0, 1);
			vec3 obj_pos = vec3(0);
			quat obj_ori = Identity<quat>();
			vec3 obj_scale = vec3(1);
			if (state) {
				if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
					obj_pos = os->position;
					obj_ori = os->orientation;
					obj_scale = os->scale;
				}
			}
			else if (GeomTransform* tr = obj->FindTransform()) {
				obj_pos = tr->position;
				obj_ori = tr->orientation;
				obj_scale = tr->scale;
			}
			plane_origin = obj_pos;
			auto set_plane_from_view = [&] {
				if (view_i < 0 || view_i >= 4)
					return;
				EditRendererBase* rend = v0.rends[view_i];
				if (!rend)
					return;
				switch (rend->view_mode) {
				case VIEWMODE_XY: plane_normal = vec3(0, 0, 1); break;
				case VIEWMODE_XZ: plane_normal = vec3(0, 1, 0); break;
				case VIEWMODE_YZ: plane_normal = vec3(1, 0, 0); break;
				case VIEWMODE_PERSPECTIVE:
					plane_normal = VectorTransform(VEC_FWD, obj_ori);
					break;
				default: break;
				}
			};
			switch (edit_plane) {
			case PLANE_XY: plane_normal = vec3(0, 0, 1); break;
			case PLANE_XZ: plane_normal = vec3(0, 1, 0); break;
			case PLANE_YZ: plane_normal = vec3(1, 0, 0); break;
			case PLANE_LOCAL: plane_normal = VectorTransform(VEC_FWD, obj_ori); break;
			default: set_plane_from_view(); break;
			}
			vec3 world;
			if (ScreenToPlaneWorldPoint(view_i, p, plane_origin, plane_normal, world)) {
				GeomEditableMesh& mesh = obj->GetEditableMesh();
				auto world_to_local = [&](const vec3& w) {
					quat inv = obj_ori.GetInverse();
					vec3 v = VectorTransform(w - obj_pos, inv);
					vec3 out = v;
					if (obj_scale[0] != 0) out[0] /= obj_scale[0];
					if (obj_scale[1] != 0) out[1] /= obj_scale[1];
					if (obj_scale[2] != 0) out[2] /= obj_scale[2];
					return out;
				};
				int picked = PickNearestPoint(mesh, view_i, p, edit_pick_radius_px);
				auto snap_local = [&](vec3 local) {
					if (!edit_snap_enable)
						return local;
					double s = edit_snap_step;
					if (s <= 0.0)
						return local;
					auto snap = [&](double v) { return (float)(floor(v / s + 0.5) * s); };
					if (edit_snap_local) {
						local[0] = snap(local[0]);
						local[1] = snap(local[1]);
						local[2] = snap(local[2]);
					}
					return local;
				};
				if (edit_tool == TOOL_POINT) {
					vec3 local = snap_local(world_to_local(world));
					mesh.points.Add(local);
					state->UpdateObjects();
					RefrehRenderers();
				}
				else if (edit_tool == TOOL_LINE) {
					int idx = picked;
					if (idx < 0) {
						vec3 local = snap_local(world_to_local(world));
						idx = mesh.points.GetCount();
						mesh.points.Add(local);
					}
					if (edit_line_start < 0) {
						edit_line_start = idx;
					}
					else {
						if (edit_line_start != idx) {
							GeomEdge edge;
							edge.a = edit_line_start;
							edge.b = idx;
							mesh.lines.Add(edge);
						}
						edit_line_start = idx;
					}
					state->UpdateObjects();
					RefrehRenderers();
				}
				else if (edit_tool == TOOL_FACE) {
					int idx = picked;
					if (idx < 0) {
						vec3 local = snap_local(world_to_local(world));
						idx = mesh.points.GetCount();
						mesh.points.Add(local);
					}
					edit_face_points.Add(idx);
					if (edit_face_points.GetCount() >= 3) {
						GeomFace face;
						face.a = edit_face_points[0];
						face.b = edit_face_points[1];
						face.c = edit_face_points[2];
						mesh.faces.Add(face);
						edit_face_points.Clear();
					}
					state->UpdateObjects();
					RefrehRenderers();
				}
				else if (edit_tool == TOOL_ERASE) {
					if (picked >= 0) {
						RemoveEditablePoint(mesh, picked);
						state->UpdateObjects();
						RefrehRenderers();
					}
				}
				else if (edit_tool == TOOL_JOIN) {
					if (picked >= 0) {
						if (edit_join_start < 0) {
							edit_join_start = picked;
						}
						else {
							if (edit_join_start != picked && !HasLine(mesh, edit_join_start, picked)) {
								GeomEdge edge;
								edge.a = edit_join_start;
								edge.b = picked;
								mesh.lines.Add(edge);
							}
							edit_join_start = picked;
						}
						state->UpdateObjects();
						RefrehRenderers();
					}
				}
				else if (edit_tool == TOOL_SPLIT) {
					int line_idx = PickNearestLine(mesh, view_i, p, edit_line_pick_radius_px);
					if (line_idx >= 0) {
						mesh.lines.Remove(line_idx);
						state->UpdateObjects();
						RefrehRenderers();
					}
				}
			}
		}
	}
	if (view == VIEW_GEOMPROJECT && type == "mouseDown" && edit_tool == TOOL_SELECT && !sculpt_mode) {
		GeomObject* obj = v0.selected_obj;
		if (obj) {
			GeomSkeleton* sk = obj->FindSkeleton();
			if (sk) {
				EditRendererBase* rend = v0.rends[view_i];
				if (rend) {
					Size sz = rend->GetSize();
					if (sz.cx > 0 && sz.cy > 0) {
						GeomCamera& gcam = rend->GetGeomCamera();
						Camera cam;
						gcam.LoadCamera(rend->view_mode, cam, sz);
						mat4 view = cam.GetWorldMatrix();
						mat4 proj = cam.GetProjectionMatrix();
						auto project_point = [&](const vec3& world, Point& out) -> bool {
							vec4 clip = proj * (view * world.Embed());
							if (clip[3] == 0)
								return false;
							vec3 ndc = clip.Splice() / clip[3];
							if (ndc[0] < -1 || ndc[0] > 1 || ndc[1] < -1 || ndc[1] > 1)
								return false;
							out = Point(
								(int)floor((ndc[0] + 1) * 0.5 * (float)sz.cx + 0.5f),
								(int)floor((-ndc[1] + 1) * 0.5 * (float)sz.cy + 0.5f));
							return true;
						};
						auto dist2_segment = [](const Point& a, const Point& b, const Point& p) -> double {
							double ax = a.x;
							double ay = a.y;
							double bx = b.x;
							double by = b.y;
							double px = p.x;
							double py = p.y;
							double dx = bx - ax;
							double dy = by - ay;
							double len2 = dx * dx + dy * dy;
							if (len2 <= 1e-6) {
								double sx = px - ax;
								double sy = py - ay;
								return sx * sx + sy * sy;
							}
							double t = ((px - ax) * dx + (py - ay) * dy) / len2;
							if (t < 0.0) t = 0.0;
							if (t > 1.0) t = 1.0;
							double cx = ax + t * dx;
							double cy = ay + t * dy;
							double sx = px - cx;
							double sy = py - cy;
							return sx * sx + sy * sy;
						};
						vec3 obj_pos = vec3(0);
						quat obj_ori = Identity<quat>();
						vec3 obj_scale = vec3(1);
						if (state) {
							if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
								obj_pos = os->position;
								obj_ori = os->orientation;
								obj_scale = os->scale;
							}
						}
						else if (GeomTransform* tr = obj->FindTransform()) {
							obj_pos = tr->position;
							obj_ori = tr->orientation;
							obj_scale = tr->scale;
						}
						double best = edit_line_pick_radius_px * edit_line_pick_radius_px;
						VfsValue* best_bone = nullptr;
						auto pick_bone = [&](auto&& pick_bone, VfsValue& bone_node,
						                     const vec3& parent_pos, const quat& parent_ori) -> void {
							if (!IsVfsType(bone_node, AsTypeHash<GeomBone>()))
								return;
							GeomBone& bone = bone_node.GetExt<GeomBone>();
							vec3 scaled(bone.position[0] * obj_scale[0],
							            bone.position[1] * obj_scale[1],
							            bone.position[2] * obj_scale[2]);
							vec3 pos = parent_pos + VectorTransform(scaled, parent_ori);
							Point a2, b2;
							if (project_point(parent_pos, a2) && project_point(pos, b2)) {
								double d2 = dist2_segment(a2, b2, p);
								if (d2 < best) {
									best = d2;
									best_bone = &bone_node;
								}
							}
							quat ori = parent_ori * bone.orientation;
							for (auto& sub : bone_node.sub) {
								if (IsVfsType(sub, AsTypeHash<GeomBone>()))
									pick_bone(pick_bone, sub, pos, ori);
							}
						};
						for (auto& sub : sk->val.sub) {
							if (IsVfsType(sub, AsTypeHash<GeomBone>()))
								pick_bone(pick_bone, sub, obj_pos, obj_ori);
						}
						if (best_bone) {
							selected_bone = best_bone;
							render_ctx.selected_bone = selected_bone;
							RefrehRenderers();
						}
					}
				}
			}
		}
	}
	if (view == VIEW_GEOMPROJECT && type == "mouseDown" && edit_tool == TOOL_SELECT && sculpt_mode) {
		GeomObject* obj = v0.selected_obj;
		if (obj) {
			GeomEditableMesh* mesh = obj->FindEditableMesh();
			if (mesh) {
				vec3 ray_o, ray_d;
				if (ScreenToRay(view_i, p, ray_o, ray_d)) {
					vec3 obj_pos = vec3(0);
					quat obj_ori = Identity<quat>();
					vec3 obj_scale = vec3(1);
					if (state) {
						if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
							obj_pos = os->position;
							obj_ori = os->orientation;
							obj_scale = os->scale;
						}
					}
					else if (GeomTransform* tr = obj->FindTransform()) {
						obj_pos = tr->position;
						obj_ori = tr->orientation;
						obj_scale = tr->scale;
					}
					quat inv = obj_ori.GetInverse();
					vec3 local_ray_o = VectorTransform(ray_o - obj_pos, inv);
					vec3 local_ray_d = VectorTransform(ray_d, inv);
					if (obj_scale[0] != 0) local_ray_o[0] /= obj_scale[0];
					if (obj_scale[1] != 0) local_ray_o[1] /= obj_scale[1];
					if (obj_scale[2] != 0) local_ray_o[2] /= obj_scale[2];
					if (obj_scale[0] != 0) local_ray_d[0] /= obj_scale[0];
					if (obj_scale[1] != 0) local_ray_d[1] /= obj_scale[1];
					if (obj_scale[2] != 0) local_ray_d[2] /= obj_scale[2];
					local_ray_d.Normalize();
					vec3 center = local_ray_o + local_ray_d * 2.0f;
					double r = sculpt_radius;
					bool hit = false;
					double best_t = 1e9;
					if (fabs(Dot(local_ray_d, local_ray_d)) > 1e-6) {
						double b = 2.0 * Dot(local_ray_o, local_ray_d);
						double c = Dot(local_ray_o, local_ray_o) - r * r;
						double disc = b * b - 4.0 * c;
						if (disc >= 0.0) {
							double t0 = (-b - sqrt(disc)) * 0.5;
							double t1 = (-b + sqrt(disc)) * 0.5;
							double t = t0;
							if (t < 0) t = t1;
							if (t > 0) {
								best_t = t;
								hit = true;
							}
						}
					}
					if (hit) {
						center = local_ray_o + local_ray_d * best_t;
					}
					for (vec3& pt : mesh->points) {
						vec3 diff = pt - center;
						float dist = diff.GetLength();
						if (dist > sculpt_radius || dist <= 1e-6f)
							continue;
						float falloff = 1.0f - dist / (float)sculpt_radius;
						float s = (float)sculpt_strength * falloff;
						if (!sculpt_add)
							s = -s;
						pt += diff.GetNormalized() * s;
					}
					state->UpdateObjects();
					RefrehRenderers();
				}
			}
		}
	}
	if (view == VIEW_GEOMPROJECT && type == "mouseDown" && edit_tool == TOOL_SELECT && weight_paint_mode) {
		GeomObject* obj = v0.selected_obj;
		if (obj && selected_bone) {
			if (!IsVfsType(*selected_bone, AsTypeHash<GeomBone>()))
				return;
			GeomBone& bone = selected_bone->GetExt<GeomBone>();
			String bone_name = bone.name.IsEmpty() ? selected_bone->id : bone.name;
			GeomEditableMesh* mesh = obj->FindEditableMesh();
			if (mesh) {
				vec3 ray_o, ray_d;
				if (ScreenToRay(view_i, p, ray_o, ray_d)) {
					vec3 obj_pos = vec3(0);
					quat obj_ori = Identity<quat>();
					vec3 obj_scale = vec3(1);
					if (state) {
						if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
							obj_pos = os->position;
							obj_ori = os->orientation;
							obj_scale = os->scale;
						}
					}
					else if (GeomTransform* tr = obj->FindTransform()) {
						obj_pos = tr->position;
						obj_ori = tr->orientation;
						obj_scale = tr->scale;
					}
					quat inv = obj_ori.GetInverse();
					vec3 local_ray_o = VectorTransform(ray_o - obj_pos, inv);
					vec3 local_ray_d = VectorTransform(ray_d, inv);
					if (obj_scale[0] != 0) local_ray_o[0] /= obj_scale[0];
					if (obj_scale[1] != 0) local_ray_o[1] /= obj_scale[1];
					if (obj_scale[2] != 0) local_ray_o[2] /= obj_scale[2];
					if (obj_scale[0] != 0) local_ray_d[0] /= obj_scale[0];
					if (obj_scale[1] != 0) local_ray_d[1] /= obj_scale[1];
					if (obj_scale[2] != 0) local_ray_d[2] /= obj_scale[2];
					local_ray_d.Normalize();
					int closest = -1;
					double best = 1e9;
					for (int i = 0; i < mesh->points.GetCount(); i++) {
						vec3 p0 = mesh->points[i];
						vec3 diff = p0 - local_ray_o;
						double t = Dot(diff, local_ray_d);
						vec3 proj = local_ray_o + local_ray_d * t;
						double d2 = (p0 - proj).GetLength();
						if (d2 < best) {
							best = d2;
							closest = i;
						}
					}
					if (closest >= 0) {
						vec3 center = mesh->points[closest];
						GeomSkinWeights& sw = obj->GetSkinWeights();
						Vector<float>& w = sw.weights.GetAdd(bone_name);
						w.SetCount(mesh->points.GetCount(), 0.0f);
						for (int i = 0; i < mesh->points.GetCount(); i++) {
							vec3 diff = mesh->points[i] - center;
							float dist = diff.GetLength();
							if (dist > weight_radius)
								continue;
							float falloff = 1.0f - dist / (float)weight_radius;
							float delta = (float)weight_strength * falloff;
							if (!weight_add)
								delta = -delta;
							float nv = w[i] + delta;
							w[i] = Clamp(nv, 0.0f, 1.0f);
						}
						state->UpdateObjects();
						RefrehRenderers();
					}
				}
			}
		}
	}
	PyValue payload = PyValue::Dict();
	payload.SetItem(PyValue("type"), PyValue(type));
	payload.SetItem(PyValue("x"), PyValue(p.x));
	payload.SetItem(PyValue("y"), PyValue(p.y));
	payload.SetItem(PyValue("flags"), PyValue((int64)flags));
	payload.SetItem(PyValue("key"), PyValue(key));
	payload.SetItem(PyValue("view"), PyValue(view_i));
	if (anim) {
		payload.SetItem(PyValue("time"), PyValue(anim->time));
		payload.SetItem(PyValue("frame"), PyValue(anim->position));
	}
	DispatchScriptEvent(type, nullptr, payload);
}

void Edit3D::DispatchFrameEvents(double dt) {
	double time = anim ? anim->time : 0.0;
	int frame = anim ? anim->position : 0;
	for (const ScriptEventHandler& h : script_event_handlers) {
		if (h.event != "enterframe")
			continue;
		if (!h.vm)
			continue;
		PyValue payload = PyValue::Dict();
		payload.SetItem(PyValue("type"), PyValue("enterFrame"));
		payload.SetItem(PyValue("dt"), PyValue(dt));
		payload.SetItem(PyValue("time"), PyValue(time));
		payload.SetItem(PyValue("frame"), PyValue(frame));
		if (h.node)
			payload.SetItem(PyValue("target"), MakeDisplayObject(this, h.node, h.vm));
		Vector<PyValue> args;
		args.Add(payload);
		String err;
		if (!RunPyCallback(*h.vm, h.func, args, err))
			LOG("Script enterFrame failed: " + err);
	}
}

void Edit3D::SetEditTool(EditTool tool) {
	edit_tool = tool;
	edit_line_start = -1;
	edit_join_start = -1;
	edit_face_points.Clear();
	draw2d_active = false;
	draw2d_poly.Clear();
	draw2d_obj = nullptr;
}

void Edit3D::CreateEditableMeshObject() {
	if (!state)
		return;
	GeomDirectory* dir = nullptr;
	if (v0.selected_ref && v0.selected_ref->kind == GeomProjectCtrl::TreeNodeRef::K_VFS && v0.selected_ref->vfs)
		dir = GetNodeDirectory(*v0.selected_ref->vfs);
	if (!dir)
		dir = &state->GetActiveScene();
	String base = "editable_mesh";
	String name = base;
	int idx = 1;
	while (dir->FindObject(name))
		name = base + "_" + IntStr(idx++);
	GeomObject& obj = dir->GetAddModel(name);
	obj.type = GeomObject::O_MODEL;
	obj.GetEditableMesh();
	state->UpdateObjects();
	RefreshData();
}

void Edit3D::Create2DLayerObject() {
	if (!state)
		return;
	GeomDirectory* dir = nullptr;
	if (v0.selected_ref && v0.selected_ref->kind == GeomProjectCtrl::TreeNodeRef::K_VFS && v0.selected_ref->vfs)
		dir = GetNodeDirectory(*v0.selected_ref->vfs);
	if (!dir)
		dir = &state->GetActiveScene();
	String base = "layer2d";
	String name = base;
	int idx = 1;
	while (dir->FindObject(name))
		name = base + "_" + IntStr(idx++);
	GeomObject& obj = dir->GetAddModel(name);
	obj.type = GeomObject::O_MODEL;
	obj.Get2DLayer();
	state->UpdateObjects();
	RefreshData();
}

GeomObject* Edit3D::GetFocusedMeshObject() {
	if (!state || state->focus_mode != 1 || !state->focus_object_key)
		return nullptr;
	GeomObject* obj = state->FindObjectByKey(state->focus_object_key);
	if (!obj || !obj->FindEditableMesh())
		return nullptr;
	return obj;
}

GeomObject* Edit3D::GetFocused2DObject() {
	if (!state || state->focus_mode != 1 || !state->focus_object_key)
		return nullptr;
	GeomObject* obj = state->FindObjectByKey(state->focus_object_key);
	if (!obj || !obj->Find2DLayer())
		return nullptr;
	return obj;
}

void Edit3D::AddMeshKeyframeAtCursor() {
	GeomObject* obj = GetFocusedMeshObject();
	if (!obj)
		return;
	GeomEditableMesh* mesh = obj->FindEditableMesh();
	if (!mesh)
		return;
	GeomMeshAnimation& anim = obj->GetMeshAnimation();
	int frame = this->anim ? this->anim->position : 0;
	GeomMeshKeyframe& kf = anim.GetAddKeyframe(frame);
	kf.frame_id = frame;
	kf.points.SetCount(mesh->points.GetCount());
	for (int i = 0; i < mesh->points.GetCount(); i++)
		kf.points[i] = mesh->points[i];
	RefrehRenderers();
}

void Edit3D::ClearMeshKeyframes() {
	GeomObject* obj = GetFocusedMeshObject();
	if (!obj)
		return;
	if (GeomMeshAnimation* anim = obj->FindMeshAnimation())
		anim->keyframes.Clear();
	RefrehRenderers();
}

void Edit3D::Add2DKeyframeAtCursor() {
	GeomObject* obj = GetFocused2DObject();
	if (!obj)
		return;
	Geom2DLayer* layer = obj->Find2DLayer();
	if (!layer)
		return;
	Geom2DAnimation& anim = obj->Get2DAnimation();
	int frame = this->anim ? this->anim->position : 0;
	Geom2DKeyframe& kf = anim.GetAddKeyframe(frame);
	kf.frame_id = frame;
	kf.shapes.SetCount(layer->shapes.GetCount());
	for (int i = 0; i < layer->shapes.GetCount(); i++) {
		const Geom2DShape& s = layer->shapes[i];
		Geom2DShape& d = kf.shapes[i];
		d.type = s.type;
		d.radius = s.radius;
		d.stroke = s.stroke;
		d.width = s.width;
		d.closed = s.closed;
		d.points.SetCount(s.points.GetCount());
		for (int k = 0; k < s.points.GetCount(); k++)
			d.points[k] = s.points[k];
	}
	RefrehRenderers();
}

void Edit3D::Clear2DKeyframes() {
	GeomObject* obj = GetFocused2DObject();
	if (!obj)
		return;
	if (Geom2DAnimation* anim = obj->Find2DAnimation())
		anim->keyframes.Clear();
	RefrehRenderers();
}

bool Edit3D::ScreenToPlaneWorldPoint(int view_i, const Point& p, const vec3& origin, const vec3& normal, vec3& out) const {
	if (view_i < 0 || view_i >= 4)
		return false;
	EditRendererBase* rend = v0.rends[view_i];
	if (!rend)
		return false;
	Size sz = rend->GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	GeomCamera& gcam = rend->GetGeomCamera();
	Camera cam;
	gcam.LoadCamera(rend->view_mode, cam, sz);
	mat4 view = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	vec2 viewport_origin(0, 0);
	vec2 viewport_size((float)sz.cx, (float)sz.cy);
	vec3 pnear = Unproject(vec3(p.x, p.y, 0.0f), viewport_origin, viewport_size, view, proj);
	vec3 pfar = Unproject(vec3(p.x, p.y, 1.0f), viewport_origin, viewport_size, view, proj);
	vec3 dir = pfar - pnear;
	float denom = Dot(dir, normal);
	if (fabs(denom) < 1e-6)
		return false;
	float t = Dot(origin - pnear, normal) / denom;
	out = pnear + dir * t;
	return true;
}

bool Edit3D::ScreenToRay(int view_i, const Point& p, vec3& out_origin, vec3& out_dir) const {
	if (view_i < 0 || view_i >= 4)
		return false;
	EditRendererBase* rend = v0.rends[view_i];
	if (!rend)
		return false;
	Size sz = rend->GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	GeomCamera& gcam = rend->GetGeomCamera();
	Camera cam;
	gcam.LoadCamera(rend->view_mode, cam, sz);
	mat4 view = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	vec2 viewport_origin(0, 0);
	vec2 viewport_size((float)sz.cx, (float)sz.cy);
	vec3 pnear = Unproject(vec3(p.x, p.y, 0.0f), viewport_origin, viewport_size, view, proj);
	vec3 pfar = Unproject(vec3(p.x, p.y, 1.0f), viewport_origin, viewport_size, view, proj);
	vec3 dir = pfar - pnear;
	if (dir.GetLength() <= 1e-6f)
		return false;
	out_origin = pnear;
	out_dir = dir;
	out_dir.Normalize();
	return true;
}

void Edit3D::CreateSkeletonForSelected() {
	if (!v0.selected_obj)
		return;
	GeomObject& obj = *v0.selected_obj;
	GeomSkeleton* sk = obj.FindSkeleton();
	if (!sk) {
		sk = &obj.GetSkeleton();
		sk->name = "Skeleton";
		sk->val.id = "skeleton";
	}
	if (sk->val.sub.IsEmpty()) {
		VfsValue& n = sk->val.Add("root", AsTypeHash<GeomBone>());
		GeomBone& b = n.GetExt<GeomBone>();
		b.name = "root";
		b.position = vec3(0, 0, 0.3f);
	}
	state->UpdateObjects();
	RefreshData();
}

void Edit3D::AddBoneToSelectedSkeleton() {
	if (!v0.selected_obj)
		return;
	GeomObject& obj = *v0.selected_obj;
	GeomSkeleton* sk = obj.FindSkeleton();
	if (!sk) {
		CreateSkeletonForSelected();
		sk = obj.FindSkeleton();
		if (!sk)
			return;
	}
	VfsValue* parent = &sk->val;
	if (selected_bone && selected_bone->owner)
		parent = selected_bone;
	auto exists = [&](const String& name) {
		for (auto& s : parent->sub) {
			if (IsVfsType(s, AsTypeHash<GeomBone>())) {
				GeomBone& b = s.GetExt<GeomBone>();
				String n = b.name.IsEmpty() ? s.id : b.name;
				if (n == name)
					return true;
			}
		}
		return false;
	};
	String base = "bone";
	String name = base;
	int idx = 1;
	while (exists(name))
		name = base + "_" + IntStr(idx++);
	VfsValue& n = parent->Add(name, AsTypeHash<GeomBone>());
	GeomBone& b = n.GetExt<GeomBone>();
	b.name = name;
	b.position = vec3(0, 0, 0.3f);
	state->UpdateObjects();
	RefreshData();
}

void Edit3D::RemoveSelectedBone() {
	if (!selected_bone || !selected_bone->owner)
		return;
	selected_bone->owner->Remove(selected_bone);
	selected_bone = nullptr;
	render_ctx.selected_bone = nullptr;
	state->UpdateObjects();
	RefreshData();
}

void Edit3D::SetWeightPaintMode(bool enable) {
	weight_paint_mode = enable;
	render_ctx.show_weights = weight_paint_mode;
	render_ctx.weight_bone.Clear();
	if (selected_bone && IsVfsType(*selected_bone, AsTypeHash<GeomBone>())) {
		GeomBone& bone = selected_bone->GetExt<GeomBone>();
		String name = bone.name.IsEmpty() ? selected_bone->id : bone.name;
		render_ctx.weight_bone = name;
	}
	RefrehRenderers();
}

int Edit3D::PickNearestPoint(const GeomEditableMesh& mesh, int view_i, const Point& p, double radius_px) const {
	if (!state)
		return -1;
	GeomObject* obj = v0.selected_obj;
	if (!obj)
		return -1;
	EditRendererBase* rend = v0.rends[view_i];
	if (!rend)
		return -1;
	Size sz = rend->GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return -1;
	GeomCamera& gcam = rend->GetGeomCamera();
	Camera cam;
	gcam.LoadCamera(rend->view_mode, cam, sz);
	mat4 view = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	vec3 obj_pos = vec3(0);
	quat obj_ori = Identity<quat>();
	vec3 obj_scale = vec3(1);
	if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
		obj_pos = os->position;
		obj_ori = os->orientation;
		obj_scale = os->scale;
	}
	double best = radius_px * radius_px;
	int best_idx = -1;
	for (int i = 0; i < mesh.points.GetCount(); i++) {
		vec3 local = mesh.points[i];
		vec3 scaled(local[0] * obj_scale[0], local[1] * obj_scale[1], local[2] * obj_scale[2]);
		vec3 world = VectorTransform(scaled, obj_ori) + obj_pos;
		vec4 clip = proj * (view * world.Embed());
		if (clip[3] == 0)
			continue;
		vec3 ndc = clip.Splice() / clip[3];
		if (ndc[0] < -1 || ndc[0] > 1 || ndc[1] < -1 || ndc[1] > 1)
			continue;
		Point pt(
			(int)floor((ndc[0] + 1) * 0.5 * (float)sz.cx + 0.5f),
			(int)floor((-ndc[1] + 1) * 0.5 * (float)sz.cy + 0.5f));
		double dx = pt.x - p.x;
		double dy = pt.y - p.y;
		double d2 = dx * dx + dy * dy;
		if (d2 < best) {
			best = d2;
			best_idx = i;
		}
	}
	return best_idx;
}

int Edit3D::PickNearestLine(const GeomEditableMesh& mesh, int view_i, const Point& p, double radius_px) const {
	if (!state)
		return -1;
	GeomObject* obj = v0.selected_obj;
	if (!obj)
		return -1;
	EditRendererBase* rend = v0.rends[view_i];
	if (!rend)
		return -1;
	Size sz = rend->GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return -1;
	GeomCamera& gcam = rend->GetGeomCamera();
	Camera cam;
	gcam.LoadCamera(rend->view_mode, cam, sz);
	mat4 view = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	vec3 obj_pos = vec3(0);
	quat obj_ori = Identity<quat>();
	vec3 obj_scale = vec3(1);
	if (const GeomObjectState* os = state->FindObjectStateByKey(obj->key)) {
		obj_pos = os->position;
		obj_ori = os->orientation;
		obj_scale = os->scale;
	}
	auto project_point = [&](const vec3& local, Point& out) -> bool {
		vec3 scaled(local[0] * obj_scale[0], local[1] * obj_scale[1], local[2] * obj_scale[2]);
		vec3 world = VectorTransform(scaled, obj_ori) + obj_pos;
		vec4 clip = proj * (view * world.Embed());
		if (clip[3] == 0)
			return false;
		vec3 ndc = clip.Splice() / clip[3];
		if (ndc[0] < -1 || ndc[0] > 1 || ndc[1] < -1 || ndc[1] > 1)
			return false;
		out = Point(
			(int)floor((ndc[0] + 1) * 0.5 * (float)sz.cx + 0.5f),
			(int)floor((-ndc[1] + 1) * 0.5 * (float)sz.cy + 0.5f));
		return true;
	};
	auto dist2_segment = [](const Point& a, const Point& b, const Point& p) -> double {
		double ax = a.x;
		double ay = a.y;
		double bx = b.x;
		double by = b.y;
		double px = p.x;
		double py = p.y;
		double dx = bx - ax;
		double dy = by - ay;
		double len2 = dx * dx + dy * dy;
		if (len2 <= 1e-6) {
			double sx = px - ax;
			double sy = py - ay;
			return sx * sx + sy * sy;
		}
		double t = ((px - ax) * dx + (py - ay) * dy) / len2;
		if (t < 0.0) t = 0.0;
		if (t > 1.0) t = 1.0;
		double cx = ax + t * dx;
		double cy = ay + t * dy;
		double sx = px - cx;
		double sy = py - cy;
		return sx * sx + sy * sy;
	};
	double best = radius_px * radius_px;
	int best_idx = -1;
	for (int i = 0; i < mesh.lines.GetCount(); i++) {
		const GeomEdge& e = mesh.lines[i];
		if (e.a < 0 || e.b < 0 || e.a >= mesh.points.GetCount() || e.b >= mesh.points.GetCount())
			continue;
		Point a2, b2;
		if (!project_point(mesh.points[e.a], a2))
			continue;
		if (!project_point(mesh.points[e.b], b2))
			continue;
		double d2 = dist2_segment(a2, b2, p);
		if (d2 < best) {
			best = d2;
			best_idx = i;
		}
	}
	return best_idx;
}

bool Edit3D::HasLine(const GeomEditableMesh& mesh, int a, int b) const {
	if (a < 0 || b < 0)
		return false;
	for (const GeomEdge& e : mesh.lines) {
		if ((e.a == a && e.b == b) || (e.a == b && e.b == a))
			return true;
	}
	return false;
}

void Edit3D::RemoveEditablePoint(GeomEditableMesh& mesh, int idx) {
	if (idx < 0 || idx >= mesh.points.GetCount())
		return;
	mesh.points.Remove(idx);
	for (int i = mesh.lines.GetCount() - 1; i >= 0; i--) {
		GeomEdge& e = mesh.lines[i];
		if (e.a == idx || e.b == idx)
			mesh.lines.Remove(i);
		else {
			if (e.a > idx) e.a--;
			if (e.b > idx) e.b--;
		}
	}
	for (int i = mesh.faces.GetCount() - 1; i >= 0; i--) {
		GeomFace& f = mesh.faces[i];
		if (f.a == idx || f.b == idx || f.c == idx)
			mesh.faces.Remove(i);
		else {
			if (f.a > idx) f.a--;
			if (f.b > idx) f.b--;
			if (f.c > idx) f.c--;
		}
	}
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
	UpdateCameraObjectRender(fake_cam, sim_fake_hmd_pose, false);
	UpdateCameraObjectRender(localized_cam, sim_localized_pose, false);
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
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, false);
	UpdateCameraObjectRender(scene.GetAddCamera("hmd_camera"), sim_localized_pose, false);
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
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, false);
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
		UpdateCameraObjectRender(scene.GetAddCamera("hmd_camera"), sim_localized_pose, false);
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
		vec3 center_cam = ApplyInversePoseSimple(sim_fake_hmd_pose, sim_state.controller_poses_world[i].position);
		bool visible = VisibleInFrustumSimple(center_cam, sim_cfg);
		if (!visible) {
			log << "controller[" << i << "] ok=0 pos=(0.000, 0.000, 0.000) not_visible=1\n";
			continue;
		}
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
	UpdateCameraObjectRender(scene.GetAddCamera("sim_fake_camera"), sim_fake_hmd_pose, false);
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
	if (dock_pool) {
		ActivateDockableChild(file_pool);
		file_pool.SetFocus();
	}
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
			f.size = GetFileLength(abs);
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
