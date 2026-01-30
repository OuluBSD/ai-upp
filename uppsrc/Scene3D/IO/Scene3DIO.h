#ifndef _Scene3D_IO_Scene3DIO_h_
#define _Scene3D_IO_Scene3DIO_h_

struct Scene3DDocument {
	int version = 1;
	String name;
	GeomProject project;
	int active_scene = 0;
	GeomCamera focus;
	GeomCamera program;

	void Visit(Vis& v);
};

static const int SCENE3D_VERSION = 1;

void FixupScene3DOwners(GeomProject& prj);

bool SaveScene3DJson(const String& path, Scene3DDocument& doc, bool pretty = true);
bool LoadScene3DJson(const String& path, Scene3DDocument& doc);
bool SaveScene3DBin(const String& path, Scene3DDocument& doc);
bool LoadScene3DBin(const String& path, Scene3DDocument& doc);

#endif
