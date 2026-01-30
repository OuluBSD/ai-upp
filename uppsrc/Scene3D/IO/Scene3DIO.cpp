#include "IO.h"

NAMESPACE_UPP

void Scene3DDocument::Visit(Vis& v) {
	v VIS_(version)
	  VIS_(name)
	  VISN(project)
	  VIS_(active_scene)
	  VISN(focus)
	  VISN(program);
}

static void FixupDirectoryOwners(GeomDirectory& dir, GeomDirectory* owner) {
	dir.owner = owner;
	for (int i = 0; i < dir.subdir.GetCount(); i++) {
		GeomDirectory& sub = dir.subdir[i];
		sub.name = dir.subdir.GetKey(i);
		FixupDirectoryOwners(sub, &dir);
	}
	for (GeomObject& o : dir.objs)
		o.owner = &dir;
}

void FixupScene3DOwners(GeomProject& prj) {
	for (int i = 0; i < prj.scenes.GetCount(); i++) {
		GeomScene& scene = prj.scenes[i];
		scene.owner = &prj;
		FixupDirectoryOwners(scene, 0);
	}
	hash_t max_key = 0;
	for (GeomScene& scene : prj.scenes) {
		for (GeomObject& obj : GeomObjectCollection(scene)) {
			if (obj.key > max_key)
				max_key = obj.key;
		}
	}
	if (max_key >= prj.key_counter)
		prj.key_counter = max_key + 1;
}

static bool CheckScene3DVersion(int version) {
	return version > 0 && version <= SCENE3D_VERSION;
}

bool SaveScene3DJson(const String& path, Scene3DDocument& doc, bool pretty) {
	if (!CheckScene3DVersion(doc.version))
		doc.version = SCENE3D_VERSION;
	String json;
	if (!DoVisitToJson(doc, json, pretty))
		return false;
	return SaveFile(path, json);
}

bool LoadScene3DJson(const String& path, Scene3DDocument& doc) {
	String json = LoadFile(path);
	if (json.IsEmpty())
		return false;
	if (!VisitFromJson(doc, json.Begin()))
		return false;
	if (!CheckScene3DVersion(doc.version))
		return false;
	FixupScene3DOwners(doc.project);
	return true;
}

bool SaveScene3DBin(const String& path, Scene3DDocument& doc) {
	if (!CheckScene3DVersion(doc.version))
		doc.version = SCENE3D_VERSION;
	FileOut out(path);
	if (!out.IsOpen())
		return false;
	Vis vis(out);
	doc.Visit(vis);
	out.Close();
	return !vis.IsError();
}

bool LoadScene3DBin(const String& path, Scene3DDocument& doc) {
	FileIn in(path);
	if (!in.IsOpen())
		return false;
	Vis vis(in);
	doc.Visit(vis);
	if (vis.IsError())
		return false;
	if (!CheckScene3DVersion(doc.version))
		return false;
	FixupScene3DOwners(doc.project);
	return true;
}

END_UPP_NAMESPACE
