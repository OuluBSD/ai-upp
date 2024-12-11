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
	
	METAKIND_DATABASE_SOURCE,
	METAKIND_PKG_ENV,
	METAKIND_CONTEXT,
	METAKIND_DB_REF,
	METAKIND_COMMENT,
	METAKIND_ECS_SPACE,
	
	METAKIND_EXTENSION_BEGIN = 1100,
	
	METAKIND_ECS_ENTITY,
	
	METAKIND_ECS_COMPONENT_BEGIN,
	
	METAKIND_ECS_COMPONENT_PROFILE,
	METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE,
	METAKIND_ECS_COMPONENT_SCRIPT,
	METAKIND_ECS_COMPONENT_LYRICS,
	METAKIND_ECS_COMPONENT_SONG,
	METAKIND_ECS_COMPONENT_MARKETPLACE,
	METAKIND_ECS_COMPONENT_IMG_LAYER,
	METAKIND_ECS_COMPONENT_IMG_GEN_LAYER,
	METAKIND_ECS_COMPONENT_IMG_ASPECT_FIXER_LAYER,
	
	METAKIND_ECS_COMPONENT_END,
	
	
	METAKIND_EXTENSION_END = 1200,
	
	
	
};

inline bool IsEcsComponentKind(int kind) {
	return kind > METAKIND_ECS_COMPONENT_BEGIN && kind < METAKIND_ECS_COMPONENT_END;
}

struct MetaNode;

struct MetaNodeExt : Pte<MetaNodeExt> {
	MetaNode& node;
	
	MetaNodeExt(MetaNode& n) : node(n) {}
	virtual ~MetaNodeExt() {}
	virtual void Serialize(Stream& s) = 0;
	virtual void Jsonize(JsonIO& json) = 0;
	virtual hash_t GetHashValue() const = 0;
	virtual String GetName() const {return String();}
	
	void CopyFrom(const MetaNodeExt& e);
	bool operator==(const MetaNodeExt& e) const;
	
};

struct MetaExtCtrl : Ctrl {
	Ptr<MetaNodeExt> ext;
	
	virtual ~MetaExtCtrl() {}
	virtual void Data() = 0;
	virtual void ToolMenu(Bar& bar) = 0;
	MetaNode& GetNode();
	MetaNodeExt& GetExt();
	
	template <class T> T& GetExt() {return dynamic_cast<T&>(*ext);}
};

struct MetaExtFactory {
	typedef MetaNodeExt* (*NewFn)(MetaNode&);
	typedef MetaExtCtrl* (*NewCtrl)();
	typedef bool (*IsFn)(const MetaNodeExt& e);
	
	struct Factory {
		int kind;
		String name;
		String ctrl_name;
		NewFn new_fn = 0;
		NewCtrl new_ctrl_fn = 0;
		IsFn is_fn = 0;
	};
	
	
	template<class T> struct Functions {
		static MetaNodeExt* Create(MetaNode& owner) {MetaNodeExt* c = new T(owner); return c;}
		static bool IsNodeExt(const MetaNodeExt& e) {return dynamic_cast<const T*>(&e);}
	};
	template<class T> struct CtrlFunctions {
		static MetaExtCtrl* CreateCtrl() {return new T;}
	};
	static Array<Factory>& List() {static Array<Factory> f; return f;}
	
	template <class T> inline static void Register(String name) {
		Factory& f = List().Add();
		f.kind = T::GetKind();
		f.name = name;
		f.new_fn = &Functions<T>::Create;
		f.is_fn = &Functions<T>::IsNodeExt;
	}
	
	template <class Comp, class Ctrl> static void RegisterCtrl(String ctrl_name) {
		for (Factory& f : List()) {
			if (f.is_fn == &Functions<Comp>::IsNodeExt) {
				ASSERT_(!f.new_ctrl_fn, "Only one Ctrl per Extension is supported currently, and one is already registered");
				f.new_ctrl_fn = &CtrlFunctions<Ctrl>::CreateCtrl;
				f.ctrl_name = ctrl_name;
				return;
			}
		}
		Panic("No component found");
	}
	static MetaNodeExt* CreateKind(int kind, MetaNode& owner);
	static MetaNodeExt* CloneKind(int kind, const MetaNodeExt& e, MetaNode& owner);
	static MetaNodeExt* Clone(const MetaNodeExt& e, MetaNode& owner);
	static int FindKindFactory(int kind);
};

#define INITIALIZER_COMPONENT(x) INITIALIZER(x) {MetaExtFactory::Register<x>(#x);}
#define INITIALIZER_COMPONENT_CTRL(comp,ctrl) INITIALIZER(ctrl) {MetaExtFactory::RegisterCtrl<comp,ctrl>(#ctrl);}

struct MetaNode : Pte<MetaNode> {
	Array<MetaNode> sub;
	int kind = -1;
	String id, type;
	hash_t type_hash = 0;
	Point begin = Null;
	Point end = Null;
	hash_t filepos_hash = 0;
	int file = -1;
	bool is_ref = false;
	bool is_definition = false;
	hash_t serial = 0;
	bool is_disabled = false;
	One<MetaNodeExt> ext;
	
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
	void CopyFieldsFrom(const MetaNode& n, bool forced_downgrade=false);
	void CopySubFrom(const MetaNode& n);
	void FindDifferences(const MetaNode& n, Vector<String>& diffs, int max_diffs=30) const;
	String GetKindString() const;
	static String GetKindString(int i);
	MetaNode& GetAdd(String id, String type, int kind);
	MetaNode& Add(const MetaNode& n);
	MetaNode& Add(MetaNode* n);
	MetaNode& Add();
	MetaNode& Add(int kind, String id=String());
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	hash_t GetTotalHash() const;
	hash_t GetSourceHash(bool* total_hash_diffs=0) const;
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
	void FixParent() {for (auto& s : sub) s.owner = this;}
	void PointPkgTo(MetaNodeSubset& other, int pkg_id);
	void PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id);
	void CopyPkgTo(MetaNode& other, int pkg_id) const;
	void CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const;
	bool HasPkgDeep(int pkg_id) const;
	bool HasPkgFileDeep(int pkg_id, int file_id) const;
	void SetPkgDeep(int pkg_id);
	void SetFileDeep(int file_id);
	void SetPkgFileDeep(int pkg_id, int file_id);
	void SetTempDeep();
	Vector<MetaNode*> FindAllShallow(int kind);
	Vector<const MetaNode*> FindAllShallow(int kind) const;
	void FindAllDeep(int kind, Vector<MetaNode*>& out);
	void FindAllDeep(int kind, Vector<const MetaNode*>& out) const;
	bool IsFieldsSame(const MetaNode& n) const;
	bool IsSourceKind() const;
	bool IsStructKind() const;
	int GetRegularCount() const;
	//bool IsClassTemplateDefinition() const;
	String GetBasesString() const;
	String GetNestString() const;
	bool OwnerRecursive(const MetaNode& n) const;
	bool ContainsDeep(const MetaNode& n) const;
	void RemoveAllShallow(int kind);
	void RemoveAllDeep(int kind);
	void GetTypeHashes(Index<hash_t>& type_hashes) const;
	void RealizeSerial();
	void FixSerialDeep();
	Vector<Ptr<MetaNodeExt>> GetAllExtensions();
	String GetPath() const;
	void DeepChk();
	void Chk();
	
	template <class T>
	T& Add() {
		MetaNode& s = Add();
		T* o = new T(s);
		s.ext = o;
		s.owner = this;
		s.pkg = pkg;
		s.file = file;
		s.kind = T::GetKind();
		return *o;
	}
	
	template <class T>
	Vector<Ptr<T>> FindAll() {
		Vector<Ptr<T>> v;
		for (auto& s : sub) {
			if (s.ext) {
				T* o = dynamic_cast<T*>(&*s.ext);
				if (o) v.Add(o);
			}
		}
		return v;
	}
	
	template <class T>
	T* Find(int kind=-1) {
		bool chk_kind = kind >= 0;
		for (auto& s : sub) {
			if (chk_kind && s.kind != kind) continue;
			if (s.ext) {
				T* o = dynamic_cast<T*>(&*s.ext);
				ASSERT(!chk_kind || o);
				if (o) return o;
			}
		}
		return 0;
	}
};

struct MetaNodeSubset {
	Array<MetaNodeSubset> sub;
	Ptr<MetaNode> n;
	
	MetaNodeSubset() {}
	//MetaNodeSubset(MetaNode& n) : n(&n) {}
	void Clear() {sub.Clear(); n = 0;}
};

struct MetaSrcPkg;

struct MetaSrcFile : Moveable<MetaSrcFile> {
	int id = -1;
	String saved_hash;
	hash_t highest_seen_serial = 0;
	VectorMap<hash_t,String> seen_types;
	One<MetaNode> temp;
	
	MetaSrcPkg* pkg = 0;
	String full_path, rel_path;
	mutable Mutex lock;
	bool managed_file = false; // if file only exists as MetaNode json
	bool keep_file_ids = false;
	bool env_file = false;
	
	MetaSrcFile() {}
	void Jsonize(JsonIO& json);
	void Serialize(Stream& s);
	bool IsBinary() const {return GetFileExt(full_path) == ".bin";}
	bool IsECS() const {return GetFileExt(full_path) == ".ecs";}
	bool IsEnv() const {return GetFileExt(full_path) == ".env";}
	bool IsExt(String s) const {return GetFileExt(full_path) == s;}
	bool Store(bool forced=false);
	String StoreJson();
	bool Load();
	bool LoadJson(String json);
	void RefreshSeenTypes();
	MetaNode& GetTemp();
	MetaNode& CreateTemp();
	void ClearTemp();
	void OnSeenTypes();
	void OnSerialCounter();
	String GetTitle() const {return GetFileTitle(full_path);}
	void KeepFileIndices(bool b=true) {keep_file_ids = b;}
	void ManageFile(bool b=true) {managed_file = b;}
	void MakeTempFromEnv(bool all_files=false);
	
	#if 0
	MetaSrcFile(const MetaSrcFile& f) {*this = f;}
	MetaSrcFile(MetaSrcFile&& f) /*: ai_items(pick(f.ai_items))*/ {}
	void operator=(const MetaSrcFile& s);
	void UpdateLinks(FileAnnotation& ann);
	#endif
};

struct MetaSrcPkg {
	typedef MetaSrcPkg CLASSNAME;
	
	Array<MetaSrcFile> files;
	
	String dir;
	int id = -1;
	
	MetaSrcPkg() {}
	MetaSrcPkg(MetaSrcPkg&& f) {*this = f;}
	MetaSrcPkg(const MetaSrcPkg& f) {*this = f;}
	void operator=(const MetaSrcPkg& f);
	void Init();
	bool Load();
	bool Store(bool forced);
	MetaSrcFile& GetAddFile(const String& full_path);
	MetaSrcFile& GetMetaFile();
	//void SetPath(String full_path, String upp_dir);
	String GetTitle() const;
	String GetRelativePath(const String& path) const;
	String GetFullPath(const String& rel_path) const;
	String GetFullPath(int file_i) const;
	void Serialize(Stream& s) {s % files;}
	int FindFile(String path) const;
	String GetDirectory() const {return dir;}
private:
	bool post_saving = false;
	Mutex lock;
	mutable String title;
};

class ClangTypeResolver;
struct MetaNode;
struct Entity;

typedef enum : byte {
	MERGEMODE_OVERWRITE_OLD,
	MERGEMODE_KEEP_OLD,
	MERGEMODE_UPDATE_SUBSET,
} MergeMode;

struct MetaEnvironment {
	struct FilePos : Moveable<FilePos> {
		Vector<Ptr<MetaNode>> hash_nodes;
	};
	struct Type : Moveable<Type> {
		Vector<Ptr<MetaNode>> hash_nodes;
		String seen_type;
	};
	VectorMap<hash_t,FilePos> filepos;
	VectorMap<hash_t,Type> types;
	hash_t serial_counter = 0;
	SpinLock serial_lock;
	
	Array<MetaSrcPkg> pkgs;
	MetaNode root;
	RWMutex lock;
	
	MetaEnvironment();
	hash_t NewSerial();
	hash_t CurrentSerial() const {return serial_counter;}
	int FindPkg(String dir) const;
	MetaSrcPkg& GetAddPkg(String path);
	String ResolveMetaSrcPkgPath(const String& includes, String path, String& ret_upp_dir);
	MetaSrcFile& ResolveFile(const String& includes, const String& path);
	//MetaSrcFile& ResolveFileInfo(const String& includes, String path);
	MetaSrcFile& Load(const String& includes, const String& path);
	bool LoadFileRoot(const String& includes, const String& path, bool manage_file);
	bool LoadFileRootJson(const String& includes, const String& path, const String& json, bool manage_file);
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	void Store(String& includes, const String& path, ClangNode& n);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id);
	void SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id);
	void SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id);
	String GetFilepath(int pkg_id, int file_id) const;
	static bool IsMergeable(CXCursorKind kind);
	static bool IsMergeable(int kind);
	bool MergeNode(MetaNode& root, const MetaNode& other, MergeMode mode);
	bool MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	bool MergeVisitPartMatching(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	void RefreshNodePtrs(MetaNode& n);
	void MergeVisitPost(MetaNode& n);
	MetaNode* FindDeclaration(const MetaNode& n);
	MetaNode* FindTypeDeclaration(unsigned type_hash);
	Vector<MetaNode*> FindDeclarationsDeep(const MetaNode& n);
	bool MergeResolver(ClangTypeResolver& ctr);
	hash_t RealizeTypePath(const String& path);
	MetaNode& RealizeFileNode(int pkg, int file, int kind);
	void OnLoadFile(MetaSrcFile& file);
	MetaNode* FindNodeEnv(Entity& n);
	void UpdateWorkspace(Workspace& wspc);
	Vector<MetaNode*> FindAllEnvs();
	bool LoadDatabaseSourceJson(MetaSrcFile& file, String path, String data);
};

MetaEnvironment& MetaEnv();






END_UPP_NAMESPACE

#endif
