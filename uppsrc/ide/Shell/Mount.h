#ifndef _ide_Shell_Mount_h_
#define _ide_Shell_Mount_h_

NAMESPACE_UPP

struct VfsPath : Moveable<VfsPath> {
	String str;
	Vector<String> parts;
	void Set(String path);
};

struct MountManager {
	struct MountPoint {
		VfsPath path;
		String type;
		Ptr<VFS> vfs;
		One<VFS> owned_vfs;
	};
	
	typedef MountManager CLASSNAME;
	MountManager();
	bool HasRoot() const;
	bool Mount(String path, VFS* vfs, String type="");
	MountPoint* Find(const String& path);
	
	Array<MountPoint> mounts;
	String last_error;
	
	static MountManager& System() {static MountManager m; return m;}
};

END_UPP_NAMESPACE

#endif
