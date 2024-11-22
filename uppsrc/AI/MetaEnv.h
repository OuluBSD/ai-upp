#ifndef _AI_MetaSrcPkg_h_
#define _AI_MetaSrcPkg_h_

struct FileAnnotation;

NAMESPACE_UPP

bool MakeRelativePath(const String& includes, const String& dir, String& best_ai_dir, String& best_rel_dir);
Vector<String> FindParentUppDirectories(const String& dir);

struct MetaNodeSubset;

struct MetaNode : Pte<MetaNode> {
	Array<MetaNode> sub;
	int kind = -1;
	String id, type;
	Point begin = Null;
	Point end = Null;
	hash_t filepos_hash = 0;
	hash_t common_hash = 0;
	int file = -1;
	bool is_ref = false;
	
	// Temp
	int pkg = -1;
	bool only_temporary = false;
	
	MetaNode() {}
	MetaNode(const MetaNode& n) {*this = n;}
	~MetaNode() {}
	void operator=(const MetaNode& n) {sub <<= n.sub; CopyFieldsFrom(n);}
	void operator=(const ClangNode& n);
	void CopyFieldsFrom(const MetaNode& n);
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	hash_t GetCommonHash() const;
	void Serialize(Stream& s) {s % sub % kind % id % type % begin % end % filepos_hash % common_hash % file % is_ref;}
	void PointPkgTo(MetaNodeSubset& other, int pkg_id);
	void PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id);
	void CopyPkgTo(MetaNode& other, int pkg_id) const;
	void CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const;
	bool HasPkgDeep(int pkg_id) const;
	bool HasPkgFileDeep(int pkg_id, int file_id) const;
	void SetPkgDeep(int pkg_id);
	void SetFileDeep(int pkg_id);
	void SetTempDeep();
};

struct MetaNodeSubset {
	Array<MetaNodeSubset> sub;
	Ptr<MetaNode> n;
	
	MetaNodeSubset() {}
	//MetaNodeSubset(MetaNode& n) : n(&n) {}
	void Clear() {sub.Clear(); n = 0;}
};

#if 0
struct AstNode {
	
};

struct MetaSrcFile : Moveable<MetaSrcFile> {
	//Array<AiAnnotationItem> ai_items;
	Index<String> idlist;
	Vector<AstNode*> idptrs;
	AstNode root;
	
	mutable Mutex lock;
	
	MetaSrcFile() {}
	MetaSrcFile(const MetaSrcFile& f) {*this = f;}
	MetaSrcFile(MetaSrcFile&& f) /*: ai_items(pick(f.ai_items))*/ {}
	void operator=(const MetaSrcFile& s);
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	void UpdateLinks(FileAnnotation& ann);
};

#endif

struct MetaSrcPkg {
	typedef MetaSrcPkg CLASSNAME;
	//ArrayMap<String, MetaSrcFile> files;
	String saved_hash;
	Index<String> filenames;
	String bin_path, upp_dir;
	int id = -1;
	
	MetaSrcPkg() {}
	MetaSrcPkg(MetaSrcPkg&& f) {*this = f;}
	MetaSrcPkg(const MetaSrcPkg& f) {*this = f;}
	void operator=(const MetaSrcPkg& f);
	bool Load(MetaNode& file_nodes);
	bool Store(MetaNode& file_nodes);
	void SetPath(String bin_path, String upp_dir);
	String GetRelativePath(const String& path) const;
	String GetFullPath(const String& rel_path) const;
	String GetFullPath(int file_i) const;
	/*void Load();
	void Save(bool forced=false);
	void Clear();
	void PostSave();
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	//MetaSrcFile& RealizePath(const String& includes, const String& path);
	bool IsEmpty() const { return files.IsEmpty(); }
	String GetHashSha1();
	void Load(const String& includes, const String& path, FileAnnotation& fa);
	void Store(const String& includes, const String& path, FileAnnotation& fa);*/
	void Serialize(Stream& s) {s % saved_hash % filenames;}
	
private:
	
	bool post_saving = false;
	Mutex lock;
	
};




struct MetaEnvironment {
	VectorMap<hash_t,Vector<Ptr<MetaNode>>> filepos_nodes;
	ArrayMap<String, MetaSrcPkg> pkgs;
	MetaNode root;
	RWMutex lock;
	
	MetaEnvironment();
	String ResolveMetaSrcPkgPath(const String& includes, String path, String& ret_upp_dir);
	MetaSrcPkg& ResolveFile(const String& includes, String path);
	//MetaSrcFile& ResolveFileInfo(const String& includes, String path);
	void Load(const String& includes, const String& path);
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	void Store(String& includes, const String& path, ClangNode& n);
	bool MergeNode(MetaNode& root, const MetaNode& other);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id);
	String GetFilepath(int pkg_id, int file_id) const;
	static bool IsMergeable(CXCursorKind kind);
	static bool IsMergeable(int kind);
	bool MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1);
	void RefreshFilePos(MetaNode& n);
	void MergeVisitPost(MetaNode& n);
	MetaNode* FindDeclaration(const MetaNode& n);
};

MetaEnvironment& MetaEnv();

END_UPP_NAMESPACE

#endif
