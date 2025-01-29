#ifndef _ide_Shell_VFS_h_
#define _ide_Shell_VFS_h_

NAMESPACE_UPP

/*
	Internal path notation (default is INTERNAL_COMB):
		INTERNAL_POSIX:		e.g. /usr/local/bin/bash
		INTERNAL_CPM:		e.g. C:\GAMES\ACROSS
		INTERNAL_COMB:		e.g. C:/Apps/Upp
*/
#if defined flagINTERNAL_POSIX
	#define INTERNAL_POSIX 1
	#define INTERNAL_CPM   0
	#define INTERNAL_COMB  0
	#define INTERNAL_ROOT_PATH "/"
	#define INTERNAL_SEPS "/"
	#define INTERNAL_SEP '/'
	#define INTERNAL_ROOT_FILE(x) "/" x
#elif defined flagINTERNAL_CPM
	#define INTERNAL_POSIX 0
	#define INTERNAL_CPM   1
	#define INTERNAL_COMB  0
	#define INTERNAL_ROOT_PATH ""
	#define INTERNAL_SEPS "\\"
	#define INTERNAL_SEP '\\'
	#define INTERNAL_ROOT_FILE(x) x ":"
#else
	#define INTERNAL_POSIX 0
	#define INTERNAL_CPM   0
	#define INTERNAL_COMB  1
	#define INTERNAL_ROOT_PATH ""
	#define INTERNAL_SEPS "/" // the default is "/" and not "\" because it's easier to type for the original maintainer (who uses the Finnish/Swedish keyboard layout)
	#define INTERNAL_SEP '/'
	#define INTERNAL_ROOT_FILE(x) x ":"
#endif


struct VfsPath : Moveable<VfsPath> {
	VfsPath();
	VfsPath(const String& s);
	VfsPath(const VfsPath& path);
	VfsPath(const Vector<String>& path);
	VfsPath(VfsPath&& path);
	VfsPath& operator=(const VfsPath& path);
	void	Set(String path);
	void	Set(const Vector<String>& parts);
	void	Set(const VfsPath& path, int begin, int end);
	bool	IsLeft(const VfsPath& path) const;
	bool	IsSame(const VfsPath& path, int this_begin, int other_begin, int len) const;
	String	AsSysPath() const;
	int		GetPartCount() const;
	bool	IsEmpty() const;
	String	TopPart() const;
	bool	Normalize();
	bool	IsValidFullPath() const;
	void	Append(const VfsPath& p);
	const String& Get() const;
	const Vector<String>& Parts() const;
	operator String() const;
	
private:
	String			str;
	Vector<String>	parts;
	void	StrFromParts();
};

String operator+(const char* s, const VfsPath& vfs);
String operator+(const VfsPath& vfs, const char* s);
bool IsFullInternalDirectory(const String& path);
String AppendInternalFileName(const String& a, const String& b);
String NormalizeInternalPath(const String& path);

typedef enum : byte {
	VFS_NULL,
	VFS_DIRECTORY,
	VFS_FILE,
	VFS_SYMLINK
} VfsItemType;

struct VfsItem : Moveable<VfsItem> {
	String name;
	String type_str;
	VfsItemType type = VFS_NULL;
};

struct VFS : Pte<VFS> {
	VFS() {}
	virtual ~VFS() {}
	virtual bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) = 0;
	virtual VfsItemType CheckItem(const VfsPath& rel_path) = 0;
	
	String last_error;
};

struct SystemFS : VFS {
	SystemFS() {}
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
};

END_UPP_NAMESPACE

#endif
