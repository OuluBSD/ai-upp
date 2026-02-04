#include "ModelerApp.h"

NAMESPACE_UPP


GeomProjectCtrl::GeomProjectCtrl(Edit3D* e) {
	this->e = e;
	
	time.WhenCursor << THISBACK(OnCursor);
	tree.WhenAction << THISBACK(TreeSelect);
	
	hsplit.Horz().SetPos(2000) << metasplit << vsplit,
	metasplit.Vert() << tree << props;
	vsplit.Vert().SetPos(7500) << grid << time;
	
	grid.SetGridSize(2,2);
	for(int i = 0; i < 4; i++) {
		rends[i].ctx = &e->render_ctx;
		rends[i].WhenChanged = THISBACK1(RefreshRenderer, i);
		rends[i].WhenMenu = THISBACK1(BuildViewMenu, i);
	}
	rends[0].SetViewMode(VIEWMODE_YZ);
	rends[1].SetViewMode(VIEWMODE_XZ);
	rends[2].SetViewMode(VIEWMODE_XY);
	rends[3].SetViewMode(VIEWMODE_PERSPECTIVE);
	rends[0].SetCameraSource(CAMSRC_FOCUS);
	rends[1].SetCameraSource(CAMSRC_FOCUS);
	rends[2].SetCameraSource(CAMSRC_FOCUS);
	rends[3].SetCameraSource(CAMSRC_PROGRAM);

	grid.Add(rends[0]);
	grid.Add(rends[1]);
	grid.Add(rends[2]);
	grid.Add(rends[3]);
	
	
}

void GeomProjectCtrl::RefreshRenderer(int i) {
	if (i >= 0 && i < 4)
		rends[i].Refresh();
}

void GeomProjectCtrl::RefreshAll() {
	for (int i = 0; i < 4; i++)
		rends[i].Refresh();
}

void GeomProjectCtrl::Update(double dt) {
	GeomAnim& anim = *e->anim;
	GeomVideo& video = e->video;
	bool was_playing = anim.is_playing || video.is_importing;
	
	if (video.is_importing) {
		video.Update(dt);
		TimelineData();
	}
	else {
		anim.Update(dt);
	}
	
	time.SetSelectedColumn(anim.position);
	time.Refresh();
	
	if (anim.is_playing || was_playing) {
		for(int i = 0; i < 4; i++) {
			rends[i].Refresh();
		}
	}
}

void GeomProjectCtrl::Data() {
	GeomProject& prj = *e->prj;
	
	tree.Clear();
	tree.SetRoot(ImagesImg::Root(), "Project");
	
	tree_scenes = tree.Add(0, ImagesImg::Scenes(), "Scenes");
	
	int scene_idx = 0;
	for (auto& s : prj.val.sub) {
		if (!IsVfsType(s, AsTypeHash<GeomScene>()))
			continue;
		GeomScene& scene = s.GetExt<GeomScene>();
		String name = scene.name.IsEmpty() ? "Scene #" + IntStr(scene_idx) : scene.name;
		int j = tree.Add(tree_scenes, ImagesImg::Scene(), RawToValue(&scene.val), name);
		
		TreeValue(j, scene.val);
		
		if (scene_idx == 0 && !tree.HasFocus())
			tree.SetCursor(j);
		scene_idx++;
	}
	
	/*for(int i = 0; i < prj.octrees.GetCount(); i++) {
		OctreePointModel& o = prj.octrees[i];
		String name = prj.dictionary[o.id];
		tree.Add(tree_octrees, ImagesImg::Octree(), o.id, name);
	}*/
	
	tree.Open(0);
	
	TreeSelect();
}

void GeomProjectCtrl::TreeSelect() {
	int cursor = tree.GetCursor();
	if (cursor < 0)
		return;
	Value v = tree.Get(cursor);
	if (!v.Is<VfsValue*>())
		return;
	VfsValue* node = ValueTo<VfsValue*>(v);
	if (!node)
		return;
	if (IsVfsType(*node, AsTypeHash<GeomScene>())) {
		int idx = 0;
		for (auto& s : e->prj->val.sub) {
			if (!IsVfsType(s, AsTypeHash<GeomScene>()))
				continue;
			if (&s == node) {
				e->state->active_scene = idx;
				e->state->UpdateObjects();
				RefreshAll();
				break;
			}
			idx++;
		}
	}

}

void GeomProjectCtrl::BuildViewMenu(Bar& bar, int i) {
	if (i < 0 || i >= 4)
		return;
	bar.Add(t_("View: YZ"), [=] { rends[i].SetViewMode(VIEWMODE_YZ); RefreshRenderer(i); });
	bar.Add(t_("View: XZ"), [=] { rends[i].SetViewMode(VIEWMODE_XZ); RefreshRenderer(i); });
	bar.Add(t_("View: XY"), [=] { rends[i].SetViewMode(VIEWMODE_XY); RefreshRenderer(i); });
	bar.Add(t_("View: Perspective"), [=] { rends[i].SetViewMode(VIEWMODE_PERSPECTIVE); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Camera: Focus"), [=] { rends[i].SetCameraSource(CAMSRC_FOCUS); RefreshRenderer(i); });
	bar.Add(t_("Camera: Program"), [=] { rends[i].SetCameraSource(CAMSRC_PROGRAM); RefreshRenderer(i); });
	bar.Separator();
	bar.Add(t_("Reset Camera"), [=] {
		GeomCamera& cam = rends[i].GetGeomCamera();
		cam.position = vec3(0, 0, 0);
		cam.orientation = Identity<quat>();
		cam.scale = 1.0f;
		RefreshRenderer(i);
	});
}

void GeomProjectCtrl::OnCursor(int i) {
	e->anim->position = i;
}

void GeomProjectCtrl::TreeValue(int id, VfsValue& node) {
	auto warn_unknown = [&](const VfsValue& n) {
		hash_t type_hash = n.ext ? n.ext->GetTypeHash() : n.type_hash;
		if (warned_tree_types.Find(type_hash) >= 0)
			return;
		warned_tree_types.Add(type_hash);
		LOG("GeomProjectCtrl: unexpected VfsValue type in tree: " + n.GetTypeString());
	};
	Vector<VfsValue*> dirs;
	Vector<VfsValue*> objs;
	for (auto& s : node.sub) {
		if (IsVfsType(s, AsTypeHash<GeomDirectory>()))
			dirs.Add(&s);
		else if (IsVfsType(s, AsTypeHash<GeomObject>()))
			objs.Add(&s);
		else
			warn_unknown(s);
	}
	for (VfsValue* s : dirs) {
		GeomDirectory& dir = s->GetExt<GeomDirectory>();
		String name = dir.name.IsEmpty() ? dir.val.id : dir.name;
		int j = tree.Add(id, ImagesImg::Directory(), RawToValue(s), name);
		TreeValue(j, *s);
	}
	for (VfsValue* s : objs) {
		GeomObject& o = s->GetExt<GeomObject>();
		Image img;
		switch (o.type) {
			case GeomObject::O_CAMERA: img = ImagesImg::Camera(); break;
			case GeomObject::O_MODEL:  img = ImagesImg::Model(); break;
			case GeomObject::O_OCTREE: img = ImagesImg::Octree(); break;
			default: img = ImagesImg::Object();
		}
		String name = o.name.IsEmpty() ? s->id : o.name;
		tree.Add(id, img, RawToValue(s), name);
	}
}

void GeomProjectCtrl::TimelineData() {
	GeomProject& prj = *e->prj;
	GeomScene& scene = e->state->GetActiveScene();
	Vector<GeomObject*> objects;
	GeomObjectCollection collection(scene);
	for (GeomObject& o : collection)
		objects.Add(&o);
	
	time.SetCount(objects.GetCount());
	time.SetKeypointRate(prj.kps);
	time.SetLength(scene.length);
	time.SetKeypointColumnWidth(13);
	
	for(int i = 0; i < objects.GetCount(); i++) {
		GeomObject& o = *objects[i];
		/*int j = prj.list[i];
		int id = j / GeomProject::O_COUNT;
		int type = j % GeomProject::O_COUNT;*/
		
		String name = o.name.IsEmpty() ? IntStr(i) : o.name;
		
		TimelineRowCtrl& row = time.GetRowIndex(i);
		row.SetTitle(name);
		
		GeomTimeline* tl = o.FindTimeline();
		if (tl)
			row.SetKeypoints(tl->keypoints.GetKeys());
		else
			row.SetKeypoints(Vector<int>());
		
		row.Refresh();
	}
	
	time.Refresh();
}

END_UPP_NAMESPACE
