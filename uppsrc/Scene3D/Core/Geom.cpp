#include "Core.h"

NAMESPACE_UPP


GeomObjectCollection::GeomObjectCollection(GeomDirectory& d) {
	iter.addr[0] = &d;
}

bool GeomObjectIterator::Next() {
	if (!*this)
		return false;
	
	while (1) {
		ASSERT(level >= 0);
		
		{
			GeomDirectory* a = addr[level];
			int oc = a->objs.GetCount();
			int sc = a->subdir.GetCount();
			int& p = pos[level];
			p++;
			
			if (p < oc)
				; // pass
			else if (p == oc + sc) {
				if (!level)
					break;
				level--;
				continue;
			}
			else {
				ASSERT(p >= oc && p < oc + sc);
				int s = p - oc;
				addr[level+1] = &a->subdir[s];
				pos[level+1] = -1;
				level++;
				continue;
			}
		}
		
		return true;
	}
	
	return false;
}

GeomObjectIterator::operator bool() const {
	if (!level) {
		GeomDirectory* a = addr[level];
		if (!a)
			return false;
		int oc = a->objs.GetCount();
		int sc = a->subdir.GetCount();
		if (pos[0] >= oc + sc)
			return false;
	}
	return true;
}

GeomObject& GeomObjectIterator::operator*() {
	GeomDirectory* a = addr[level];
	int& p = pos[level];
	return a->objs[p];
}

GeomObject* GeomObjectIterator::operator->() {
	GeomDirectory* a = addr[level];
	if (!a)
		return 0;
	int& p = pos[level];
	if (p < 0 || p >= a->objs.GetCount())
		return 0;
	return &a->objs[p];
}

void GeomKeypoint::Visit(Vis& v) {
	v VIS_(frame_id)
	  VISN(position)
	  VISN(orientation);
}

void GeomTimeline::Visit(Vis& v) {
	v VISM(keypoints);
}









GeomKeypoint& GeomTimeline::GetAddKeypoint(int kp_i) {
	int i = keypoints.Find(kp_i);
	if (i >= 0)
		return keypoints[i];
	int pos = keypoints.GetCount();
	for(int i = 0; i < keypoints.GetCount(); i++) {
		int j = keypoints.GetKey(i);
		if (j > kp_i) {
			pos = i;
			break;
		}
	}
	GeomKeypoint& kp = keypoints.Insert(pos, kp_i);
	kp.frame_id = kp_i;
	kp.position = Identity<vec3>();
	kp.orientation = Identity<quat>();
	return kp;
}

int GeomTimeline::FindPre(int kp_i) const {
	for(int i = 0; i < keypoints.GetCount(); i++) {
		int j = keypoints.GetKey(i);
		if (j > kp_i)
			return i-1;
		if (j == kp_i)
			return i;
	}
	return -1;
}

int GeomTimeline::FindPost(int kp_i) const {
	for(int i = keypoints.GetCount()-1; i >= 0; i--) {
		int j = keypoints.GetKey(i);
		if (j <= kp_i)
			return i+1 < keypoints.GetCount() ? i+1 : -1;
	}
	return -1;
}






	
GeomScene& GeomProject::AddScene() {
	GeomScene& s = scenes.Add();
	s.owner = this;
	return s;
}

/*GeomModel& GeomProject::AddModel() {
	GeomModel& m = models.Add();
	m.owner = this;
	return m;
}*/


String GeomObject::GetPath() const {
	String path = name;
	GeomDirectory* dir = owner;
	while (dir) {
		path = dir->name + "/" + path;
		dir = dir->owner;
	}
	return path;
}

void GeomObject::Visit(Vis& v) {
	int type_i = (int)type;
	v VIS_(name)
	  VIS_(type_i)
	  VIS_(asset_ref)
	  VIS_(pointcloud_ref)
	  VISN(timeline);
	if (v.IsLoading())
		type = (Type)type_i;
}




GeomProject& GeomDirectory::GetProject() const {
	const GeomDirectory* root = this;
	while (root->owner)
		root = root->owner;
	const GeomScene* scene = CastConstPtr<GeomScene>(root);
	ASSERT(scene && scene->GeomScene::owner);
	return *scene->GeomScene::owner;
}

GeomDirectory& GeomDirectory::GetAddDirectory(String name) {
	int i = subdir.Find(name);
	if (i >= 0)
		return subdir[i];
	GeomDirectory& dir = subdir.Add(name);
	dir.name = name;
	dir.owner = this;
	return dir;
}

void GeomDirectory::Visit(Vis& v) {
	v VIS_(name);
	if (v.mode == Vis::MODE_JSON) {
		if (v.IsLoading()) {
			subdir.Clear();
			const Value& va = v.json->Get()["subdir"];
			subdir.Reserve(va.GetCount());
			for (int i = 0; i < va.GetCount(); i++) {
				String key;
				LoadFromJsonValue(key, va[i]["key"]);
				GeomDirectory& dir = subdir.Add(key);
				JsonIO jio(va[i]["value"]);
				Vis vis(jio);
				dir.Visit(vis);
			}
		}
		else {
			Vector<Value> va;
			va.SetCount(subdir.GetCount());
			for (int i = 0; i < subdir.GetCount(); i++) {
				ValueMap item;
				item.Add("key", StoreAsJsonValue(subdir.GetKey(i)));
				item.Add("value", v.VisitAsJsonValue(subdir[i]));
				va[i] = item;
			}
			v.json->Set("subdir", ValueArray(pick(va)));
		}
	}
	else {
		if (v.mode == Vis::MODE_STREAM)
			v.VisitMapSerialize(subdir);
		else if (v.mode == Vis::MODE_HASH)
			v.VisitMapHash(subdir);
		else if (v.mode == Vis::MODE_VCS)
			v.VisitMapVcs("subdir", subdir);
	}
	v VISV(objs);
}

GeomObject* GeomDirectory::FindObject(String name) {
	for(GeomObject& o : objs)
		if (o.name == name)
			return &o;
	return 0;
}

GeomObject* GeomDirectory::FindObject(String name, GeomObject::Type type) {
	for(GeomObject& o : objs)
		if (o.name == name && o.type == type)
			return &o;
	return 0;
}

GeomObject* GeomDirectory::FindCamera(String name) {
	return FindObject(name, GeomObject::O_CAMERA);
}

GeomObject& GeomDirectory::GetAddModel(String name) {
	GeomObject* o = FindObject(name);
	if (o)
		return *o;
	
	o = &objs.Add();
	o->key = GetProject().NewKey();
	o->name = name;
	o->type = GeomObject::O_MODEL;
	return *o;
}

void GeomScene::Visit(Vis& v) {
	v.VisitT<GeomDirectory>("GeomDirectory", *this);
	v VIS_(length);
}

void GeomProject::Visit(Vis& v) {
	v VISV(scenes)
	  VIS_(kps)
	  VIS_(fps)
	  VIS_(key_counter);
}

GeomObject& GeomDirectory::GetAddCamera(String name) {
	GeomObject* o = FindObject(name);
	if (o)
		return *o;
	
	o = &objs.Add();
	o->key = GetProject().NewKey();
	o->name = name;
	o->type = GeomObject::O_CAMERA;
	return *o;
}

GeomObject& GeomDirectory::GetAddOctree(String name) {
	GeomObject* o = FindObject(name);
	if (o)
		return *o;
	
	o = &objs.Add();
	o->key = GetProject().NewKey();
	o->name = name;
	o->type = GeomObject::O_OCTREE;
	return *o;
}













GeomWorldState::GeomWorldState() {
	//focus.scale = 100;
	//program.scale = 100;
	//program.orientation = AxesQuat(M_PI/4, -M_PI/4, 0);
	
}

void GeomWorldState::UpdateObjects() {
	GeomScene& scene = GetActiveScene();
	GeomObjectCollection collection(scene);
	this->objs.SetCount(0);
	int i = 0;
	active_camera_obj_i = -1;
	for (GeomObject& o : collection) {
		GeomObjectState& s = objs.Add();
		s.obj = &o;
		if (active_camera_obj_i < 0 && o.IsCamera())
			active_camera_obj_i = i;
		i++;
	}
}

GeomScene& GeomWorldState::GetActiveScene() {
	ASSERT(active_scene >= 0 && active_scene < prj->scenes.GetCount());
	return prj->scenes[active_scene];
}







void GeomCamera::LoadCamera(ViewMode m, Camera& cam, Size sz, float far) const {
	float ratio = (float)sz.cy / (float)sz.cx;
	float aspect = (float)sz.cx / (float)sz.cy;
	
	vec3 position = this->position;
	quat orientation = this->orientation;
	float scale = this->scale;
	float len = 2;
	bool move_camera = true;
	switch (m) {
		
	case VIEWMODE_YZ:
		orientation = MatQuat(YRotation(M_PI/2));
		cam.SetOrthographic(len * aspect, len, 0.1, far);
		break;
		
	case VIEWMODE_XZ:
		orientation = MatQuat(XRotation(-M_PI/2));
		cam.SetOrthographic(len * aspect, len, 0.1, far);
		break;
		
	case VIEWMODE_XY:
		orientation = MatQuat(YRotation(0));
		cam.SetOrthographic(len * aspect, len, 0.1, far);
		break;
		
	case VIEWMODE_PERSPECTIVE:
		cam.SetPerspective(fov, aspect, 0.1, far);
		move_camera = false;
		break;
		
	default:
		return;
		
	}
	
	if (move_camera)
		position = position - VecMul(QuatMat(orientation), VEC_FWD) * scale * 0.01;
	
	cam.SetWorld(position, orientation, scale);
}

mat4 GeomCamera::GetViewMatrix(ViewMode m, Size sz) const {
	Camera cam;
	LoadCamera(m, cam, sz);
	return cam.GetViewMatrix();
}

Frustum GeomCamera::GetFrustum(ViewMode m, Size sz) const {
	Camera cam;
	LoadCamera(m, cam, sz);
	return cam.GetFrustum();
}






void GeomAnim::Update(double dt) {
	if (!is_playing) {
		return;
	}
	
	time += dt;
	
	GeomProject& prj = *state->prj;
	GeomScene& scene = state->GetActiveScene();
	
	double frame_time = 1.0 / state->prj->kps;
	position = time / frame_time;
	
	if (position < 0 || position >= scene.length) {
		is_playing = false;
		WhenSceneEnd();
		return;
	}
	
	for (GeomObjectState& os : state->objs) {
		GeomObject& o = *os.obj;
		if (!o.timeline.keypoints.IsEmpty()) {
			int pre_i = o.timeline.FindPre(position);
			int post_i = o.timeline.FindPost(position);
			if (pre_i >= 0 && post_i >= 0) {
				ASSERT(pre_i < post_i);
				GeomKeypoint& pre = o.timeline.keypoints[pre_i];
				GeomKeypoint& post = o.timeline.keypoints[post_i];
				float pre_time = pre.frame_id / (float)prj.kps;
				float post_time = post.frame_id / (float)prj.kps;
				float f = (time - pre_time) / (post_time - pre_time);
				ASSERT(f >= 0.0 && f <= 1.0);
				
				if (1) {
					os.position = Lerp(pre.position, post.position, f);
					os.orientation = Slerp(pre.orientation, post.orientation, f);
				}
			}
		}
	}
	
	
	if (state->active_camera_obj_i >= 0) {
		GeomObjectState& os = state->objs[state->active_camera_obj_i];
		ASSERT(os.obj->IsCamera());
		GeomCamera& cam = state->program;
		cam.position = os.position;
		cam.orientation = os.orientation;
	}
	
}

void GeomAnim::Reset() {
	is_playing = false;
	position = 0;
	time = 0;
}

void GeomAnim::Pause() {
	is_playing = false;
}

void GeomAnim::Play() {
	GeomScene& scene = state->GetActiveScene();
	if (position < 0 || position >= scene.length)
		Reset();
	is_playing = true;
	state->UpdateObjects();
}

void GeomCamera::Visit(Vis& v) {
	v VISN(position)
	  VISN(orientation)
	  VIS_(distance)
	  VIS_(fov)
	  VIS_(scale);
}


END_UPP_NAMESPACE
