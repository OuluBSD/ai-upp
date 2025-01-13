#ifndef _ide_Shell_VFS_h_
#define _ide_Shell_VFS_h_

NAMESPACE_UPP

struct VfsPath : Moveable<VfsPath> {
	String str;
	Vector<String> parts;
	
	VfsPath();
	VfsPath(const String& s);
	void Set(String path);
	void Set(const VfsPath& path, int begin, int end);
	bool IsLeft(const VfsPath& path) const;
	bool IsSame(const VfsPath& path, int this_begin, int other_begin, int len) const;
	String AsSysPath() const;
};

typedef enum : byte {
	VFS_UNKNOWN,
	VFS_DIRECTORY,
	VFS_FILE,
	VFS_SYMLINK
} VfsItemType;

struct VfsItem : Moveable<VfsItem> {
	String name;
	String type_str;
	VfsItemType type = VFS_UNKNOWN;
};

struct VFS : Pte<VFS> {
	VFS() {}
	virtual ~VFS() {}
	virtual bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) = 0;
	String last_error;
};

struct SystemFS : VFS {
	SystemFS() {}
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
};

END_UPP_NAMESPACE

#endif
