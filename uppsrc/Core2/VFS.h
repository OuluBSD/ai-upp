#ifndef _Core2_VFS_h_
#define _Core2_VFS_h_

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

struct Visitor;

struct VfsPath : Moveable<VfsPath> {
	VfsPath();
	//VfsPath(const Value& s);
	VfsPath(const VfsPath& path);
	VfsPath(const Vector<Value>& path);
	VfsPath(VfsPath&& path);
	VfsPath& operator=(const VfsPath& path);
	VfsPath& Add(Value part);
	void	Set(String path);
	void	Set(Value s, const VfsPath& vfs);
	void	Set(const Vector<Value>& parts);
	void	Set(const VfsPath& path, int begin, int end);
	void	SetDotPath(String path);
	void	SetPosixPath(String path);
	void    Remove(int i);
	bool	IsLeft(const VfsPath& path) const;
	bool	IsSame(const VfsPath& path, int this_begin, int other_begin, int len) const;
	String	AsSysPath() const;
	int		GetPartCount() const;
	bool	IsEmpty() const;
	Value	TopPart() const;
	String  GetFilename() const;
	String	ToString() const;
	VfsPath GetCanonical() const;
	VfsPath Left(int i) const;
	bool	Normalize();
	bool	IsValid() const;
	bool	IsValidFullPath() const;
	void	Append(const VfsPath& p);
	const String& Get() const;
	const Vector<Value>& Parts() const;
	operator String() const;
	void    RemoveLast();
	hash_t  GetHashValue() const;
	bool operator==(const VfsPath& p) const;
	bool operator==(const String& p) const;
	void    Visit(Visitor& v);
	bool    IsSysDirectory() const;
	bool    IsSysFile() const;
private:
	String			str;
	Vector<Value>	parts;
	void	StrFromParts();
};

VfsPath ValVfs(Value v);
VfsPath StrVfs(String s);

VfsPath operator+(const char* s, const VfsPath& vfs);
VfsPath operator+(Value s, const VfsPath& vfs);
VfsPath operator+(const VfsPath& vfs, const char* s);
VfsPath operator+(const VfsPath& vfs, Value s);
VfsPath operator/(const VfsPath& a, const VfsPath& b);
bool IsFullInternalDirectory(const String& path);
String AppendInternalFileName(const String& a, const String& b);
String NormalizeInternalPath(const String& path);
Value FindValuePath(Value v, const VfsPath& p);

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
	void operator=(Value v) {Set(v);}
	virtual Value Get();
	virtual Value Get(Value key);
	virtual Value GetKey(int i);
	virtual void Set(Value v) {Panic("Not implemented");}
	virtual void MapSet(Value v) {Panic("Not implemented");}
	virtual Shared<VfsItem> GetMap();
	virtual Shared<VfsItem> RealizeMap();
	virtual Shared<VfsItem> At(int i);
	virtual int GetCount() const;
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

struct ValueFS : VFS {
	ValueFS() {}
	ValueFS(Value& v) : v(&v) {}
	
	Shared<VfsItem> At(const VfsPath& p);
	Shared<VfsItem> operator()(String);
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
private:
	Value* v = 0;
};

#endif
