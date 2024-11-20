#ifndef _AI_MetaSrcPkg_h_
#define _AI_MetaSrcPkg_h_

struct FileAnnotation;

NAMESPACE_UPP

bool MakeRelativePath(const String& includes, const String& dir, String& best_ai_dir, String& best_rel_dir);
Vector<String> FindParentUppDirectories(const String& dir);

struct MetaNode {
	/*struct Item : Moveable<Item> {
		typedef enum : int {
			FILE_NODE,
			TEMP_NODE,
			META_COMMENT,
		} Type;
		AstNode* ptr = 0;
		Type type;
		int kind = -1;
	};*/
	Array<MetaNode> sub;
	int kind = -1;
	String id;
	Point begin = Null;
	Point end = Null;
	hash_t common_hash = 0;
	
	
	MetaNode() {}
	MetaNode(const MetaNode& n) {*this = n;}
	void operator=(const MetaNode& n) {sub <<= n.sub; kind = n.kind; id = n.id; begin = n.begin; end = n.end; common_hash = n.common_hash;}
	void operator=(const ClangNode& n);
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	hash_t GetCommonHash() const;
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
	
	MetaSrcPkg() {}
	MetaSrcPkg(MetaSrcPkg&& f) {*this = f;}
	MetaSrcPkg(const MetaSrcPkg& f) {*this = f;}
	void operator=(const MetaSrcPkg& f);
	bool Load(const String& aion_path, MetaNode& file_nodes);
	bool Store(const String& aion_path, MetaNode& file_nodes);
	/*void SetPath(String path);
	void Load();
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
private:
	
	bool post_saving = false;
	String path, dir;
	Mutex lock;
	
};




struct MetaEnvironment {
	MetaNode root;
	ArrayMap<String, MetaSrcPkg> pkgs;
	RWMutex lock;
	
	MetaEnvironment();
	String ResolveMetaSrcPkgPath(const String& includes, String path);
	//MetaSrcPkg& ResolveFile(const String& includes, String path);
	//MetaSrcFile& ResolveFileInfo(const String& includes, String path);
	void Load(const String& includes, const String& path, FileAnnotation& fa);
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	void Store(String& includes, const String& path, ClangNode& n);
	bool MergeVisit(Vector<MetaNode*>& scope, ClangNode& n);
	void MergeNode(MetaNode& root, const MetaNode& other, int pkg_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id);
	
	static bool IsMergeable(CXCursorKind kind);
};

MetaEnvironment& MetaEnv();

END_UPP_NAMESPACE

#endif
