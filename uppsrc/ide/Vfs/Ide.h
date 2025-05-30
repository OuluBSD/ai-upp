#ifndef _ide_Vfs_Ide_h_
#define _ide_Vfs_Ide_h_

struct VfsSrcFile : Moveable<VfsSrcFile> {
	int id = -1;
	String saved_hash;
	hash_t highest_seen_serial = 0;
	VectorMap<hash_t,String> seen_types;
	One<VfsValue> temp;
	int temp_id = -1;
	
	VfsSrcPkg* pkg = 0;
	String full_path, rel_path;
	mutable Mutex lock;
	bool managed_file = false; // if file only exists as VfsValue json
	bool keep_file_ids = false;
	bool env_file = false;
	
	VfsSrcFile() {}
	void Visit(Vis& v);
	bool IsDirTree() const {return GetFileExt(full_path) == ".d";}
	bool IsBinary() const {return GetFileExt(full_path) == ".bin";}
	bool IsECS() const {return GetFileExt(full_path) == ".ecs";}
	bool IsEnv() const {return GetFileExt(full_path) == ".env";}
	bool IsExt(String s) const {return GetFileExt(full_path) == s;}
	bool Store(bool forced=false);
	String StoreJson();
	bool Load();
	bool LoadJson(String json);
	void RefreshSeenTypes();
	VfsValue& GetTemp();
	VfsValue& CreateTemp(int dbg_src);
	void ClearTemp();
	void OnSeenTypes();
	void OnSerialCounter();
	String GetTitle() const {return GetFileTitle(full_path);}
	void KeepFileIndices(bool b=true) {keep_file_ids = b;}
	void ManageFile(bool b=true) {managed_file = b;}
	void MakeTempFromEnv(bool all_files=false);
	String UpdateStoring();
	void UpdateLoading();
	
	#if 0
	VfsSrcFile(const VfsSrcFile& f) {*this = f;}
	VfsSrcFile(VfsSrcFile&& f) /*: ai_items(pick(f.ai_items))*/ {}
	void operator=(const VfsSrcFile& s);
	void UpdateLinks(FileAnnotation& ann);
	#endif
};

struct VfsSrcPkg {
	typedef VfsSrcPkg CLASSNAME;
	
	Array<VfsSrcFile> files;
	
	String dir;
	int id = -1;
	
	VfsSrcPkg() {}
	VfsSrcPkg(VfsSrcPkg&& f) {*this = f;}
	VfsSrcPkg(const VfsSrcPkg& f) {*this = f;}
	void operator=(const VfsSrcPkg& f);
	void Init();
	bool Load();
	bool Store(bool forced);
	VfsSrcFile& GetAddFile(const String& full_path);
	VfsSrcFile& GetMetaFile();
	//void SetPath(String full_path, String upp_dir);
	String GetTitle() const;
	String GetRelativePath(const String& path) const;
	String GetFullPath(const String& rel_path) const;
	String GetFullPath(int file_i) const;
	void Visit(Vis& v);
	int FindFile(String path) const;
	String GetDirectory() const {return dir;}
private:
	bool post_saving = false;
	Mutex lock;
	mutable String title;
};

struct IdeMetaEnvironment {
	MetaEnvironment& env;
	Array<VfsSrcPkg> pkgs;
	
	IdeMetaEnvironment();
	VfsSrcPkg& GetAddPkg(String path);
	String ResolveVfsSrcPkgPath(const String& includes, String path, String& ret_upp_dir);
	VfsSrcFile& ResolveFile(const String& includes, const String& path);
	//VfsSrcFile& ResolveFileInfo(const String& includes, String path);
	VfsSrcFile& Load(const String& includes, const String& path);
	void OnLoadFile(VfsSrcFile& file);
	VfsValue* LoadDatabaseSourceVisit(VfsSrcFile& file, String path, Vis& v);
	VfsValue& RealizeFileNode(int pkg, int file, hash_t type_hash);
	void SplitValue(VfsValue& root, VfsValueSubset& other, int pkg_id);
	void SplitValue(VfsValue& root, VfsValueSubset& other, int pkg_id, int file_id);
	void SplitValue(const VfsValue& root, VfsValue& other, int pkg_id);
	void SplitValue(const VfsValue& root, VfsValue& other, int pkg_id, int file_id);
	int FindPkg(String dir) const;
	Vector<VfsValue*> FindAllEnvs();
	VfsValue* FindNodeEnv(Entity& n);
	bool LoadFileRoot(const String& includes, const String& path, bool manage_file);
	bool LoadFileRootVisit(const String& includes, const String& path, Vis& v, bool manage_file, VfsValue*& file_node);
	String GetFilepath(int pkg_id, int file_id) const;
	VfsValue* FindDeclaration(const VfsValue& n);
	Vector<VfsValue*> FindDeclarationsDeep(const VfsValue& n);
	static VfsValue* FindDeclarationStatic(const VfsValue& n);
	static Vector<VfsValue*> FindDeclarationsDeepStatic(const VfsValue& n);
	static bool IsMergeable(int kind);
	static bool IsMergeable(CXCursorKind kind);
};

void Assign(VfsValue& mn, VfsValue* owner, const ClangNode& n);
void Store(IdeMetaEnvironment& env, String& includes, const String& path, ClangNode& n);
void UpdateWorkspace(IdeMetaEnvironment& env, Workspace& wspc);
IdeMetaEnvironment& IdeMetaEnv();
bool IsStructKind(const VfsValue& n);

#endif
