#ifndef _AI_MetaSrcPkg_h_
#define _AI_MetaSrcPkg_h_

#define DEBUG_METANODE_DTOR 0

struct FileAnnotation;

NAMESPACE_UPP

bool MakeRelativePath(const String& includes, const String& dir, String& best_ai_dir, String& best_rel_dir);
Vector<String> FindParentUppDirectories(const String& dir);

struct MetaNodeSubset;

enum {
	METAKIND_BEGIN = 1000,
	
	METAKIND_ECS_SPACE = 1000,
	METAKIND_ECS_NODE,
	
	
	METAKIND_COMMENT = 10000,
};

struct MetaNode : Pte<MetaNode> {
	Array<MetaNode> sub;
	int kind = -1;
	String id, type;
	unsigned type_hash = 0;
	Point begin = Null;
	Point end = Null;
	hash_t filepos_hash = 0;
	hash_t common_hash = 0;
	int file = -1;
	bool is_ref = false;
	bool is_definition = false;
	
	// Temp
	int pkg = -1;
	bool only_temporary = false;
	Ptr<MetaNode> owner;
	Ptr<MetaNode> type_ptr;
	#if DEBUG_METANODE_DTOR
	bool trace_kill = false;
	#endif
	
	MetaNode() {}
	MetaNode(MetaNode* owner, const MetaNode& n) {Assign(owner, n);}
	~MetaNode();
	void Destroy();
	void Assign(MetaNode* owner, const MetaNode& n) {this->owner = owner; CopySubFrom(n); CopyFieldsFrom(n);}
	void Assign(MetaNode* owner, const ClangNode& n);
	void CopyFrom(const MetaNode& n);
	void CopyFieldsFrom(const MetaNode& n);
	void CopySubFrom(const MetaNode& n);
	MetaNode& GetAdd(String id, String type, int kind);
	MetaNode& Add(const MetaNode& n);
	MetaNode& Add(MetaNode* n);
	MetaNode& Add();
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	hash_t GetCommonHash(bool* total_hash_diffs=0) const;
	void Serialize(Stream& s) {s % sub % kind % id % type % type_hash % begin % end % filepos_hash % common_hash % file % is_ref % is_definition; if (s.IsLoading()) FixParent();}
	void FixParent() {for (auto& s : sub) s.owner = this;}
	void PointPkgTo(MetaNodeSubset& other, int pkg_id);
	void PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id);
	void CopyPkgTo(MetaNode& other, int pkg_id) const;
	void CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const;
	bool HasPkgDeep(int pkg_id) const;
	bool HasPkgFileDeep(int pkg_id, int file_id) const;
	void SetPkgDeep(int pkg_id);
	void SetFileDeep(int pkg_id);
	void SetTempDeep();
	Vector<MetaNode*> FindAllShallow(int kind);
	Vector<const MetaNode*> FindAllShallow(int kind) const;
	void FindAllDeep(int kind, Vector<MetaNode*>& out);
	void FindAllDeep(int kind, Vector<const MetaNode*>& out) const;
	bool IsStructKind() const;
	int GetRegularCount() const;
	//bool IsClassTemplateDefinition() const;
	String GetBasesString() const;
	String GetNestString() const;
	bool OwnerRecursive(const MetaNode& n) const;
	bool ContainsDeep(const MetaNode& n) const;
	void RemoveAllShallow(int kind);
	void RemoveAllDeep(int kind);
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
	bool Store(MetaNode& file_nodes, bool forced);
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
	VectorMap<unsigned,Vector<Ptr<MetaNode>>> type_hash_nodes;
	ArrayMap<String, MetaSrcPkg> pkgs;
	MetaNode root;
	RWMutex lock;
	
	MetaEnvironment();
	String ResolveMetaSrcPkgPath(const String& includes, String path, String& ret_upp_dir);
	MetaSrcPkg& ResolveFile(const String& includes, String path);
	//MetaSrcFile& ResolveFileInfo(const String& includes, String path);
	MetaSrcPkg& Load(const String& includes, const String& path);
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	void Store(MetaSrcPkg& af, bool forced=false);
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
	void RefreshNodePtrs(MetaNode& n);
	void MergeVisitPost(MetaNode& n);
	MetaNode* FindDeclaration(const MetaNode& n);
	MetaNode* FindTypeDeclaration(unsigned type_hash);
	Vector<MetaNode*> FindDeclarationsDeep(const MetaNode& n);
};

MetaEnvironment& MetaEnv();

END_UPP_NAMESPACE

#endif
