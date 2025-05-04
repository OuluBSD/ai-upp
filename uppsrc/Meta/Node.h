#ifndef _Meta_Node_h_
#define _Meta_Node_h_

struct MetaNode;
struct MetaExtCtrl;
struct MetaSrcPkg;
struct MetaNodeExt;
struct Entity;
struct IdeShellHost;
struct IdeShell;
class ClangTypeResolver;
class ToolAppCtrl;

#define DATASET_ITEM(type, name, kind, group, desc) struct type;
EXT_LIST
#undef DATASET_ITEM
struct SrcTextData;

struct EntityData : Pte<EntityData> {
	virtual ~EntityData() {}
	virtual int GetKind() const = 0;
	virtual void Visit(Vis& s) = 0;
};

struct DatasetPtrs {
	#define DATASET_ITEM(type, name, kind, group, desc) Ptr<type> name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	bool editable_biography = false;
	
	DatasetPtrs() {}
	DatasetPtrs(const DatasetPtrs& p) {*this = p;}
	void operator=(const DatasetPtrs& p);
	static DatasetPtrs& Single() {static DatasetPtrs p; return p;}
	void Clear();
	
	// Prevent forward declaration errors with "class _"
	template <class T, class _> struct Getter {static T& Get(DatasetPtrs& d);};
	template <class T> T& Get() {return Getter<T,int>::Get(*this);}
};

// "int I" is required because the operation "p.name = o;" must be delayed after full
// declaration, which has been done when MetaExtFactory::Register is being called.
template <class T, int I>
struct DatasetAssigner {
	static void Set(DatasetPtrs& p, T* o);
};

#define DATASET_ITEM(type, name, kind, group, desc) \
template <int I> struct DatasetAssigner<type,I> {\
	static void Set(DatasetPtrs& p, type* o) {p.name = o;}\
};
EXT_LIST
#undef DATASET_ITEM

#define DATASET_ITEM(type, name, kind, group, desc) \
template <class _> struct DatasetPtrs::Getter<type,_> {static type& Get(DatasetPtrs& p) {return *p.name;}};
EXT_LIST
#undef DATASET_ITEM

void FillDataset(DatasetPtrs& p, MetaNode& n, Component* this_comp);

class DatasetProvider {
public:
	virtual DatasetPtrs GetDataset() const = 0;
	
};

struct MetaNodeExt : Pte<MetaNodeExt> {
	MetaNode& node;
	
	MetaNodeExt(MetaNode& n) : node(n) {}
	virtual ~MetaNodeExt() {}
	virtual void Visit(Vis& s) = 0;
	virtual String GetName() const {return String();}
	hash_t GetHashValue() const;
	int GetKind() const;
	
	void CopyFrom(const MetaNodeExt& e);
	bool operator==(const MetaNodeExt& e) const;
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
};

template <bool b, class T>
struct EntityDataCreator {static EntityData* CreateEntityDataFn();};
template <class T> struct EntityDataCreator<false,T> {static EntityData* New() {return 0;}};
template <class T> struct EntityDataCreator<true, T> {static EntityData* New() {return new T;}};

struct MetaExtFactory {
	typedef MetaNodeExt* (*NewFn)(MetaNode&);
	typedef MetaExtCtrl* (*NewCtrl)();
	typedef bool (*IsFn)(const MetaNodeExt& e);
	typedef void (*SetDatasetEntityData)(DatasetPtrs&, EntityData&);
	typedef EntityData* (*CreateEntityData)();
	
	struct Factory {
		int kind;
		int category;
		String name;
		String ctrl_name;
		NewFn new_fn = 0;
		NewCtrl new_ctrl_fn = 0;
		SetDatasetEntityData set_data_ed_fn = 0;
		CreateEntityData create_ed_fn = 0;
		IsFn is_fn = 0;
		const std::type_info* type = 0;
	};
	
	
	template<class T> struct Functions {
		static MetaNodeExt* Create(MetaNode& owner) {MetaNodeExt* c = new T(owner); return c;}
		static bool IsNodeExt(const MetaNodeExt& e) {return dynamic_cast<const T*>(&e);}
	};
	template<class T> struct CtrlFunctions {
		static MetaExtCtrl* CreateCtrl() {return new T;}
	};
	template <class T> static void DatasetEntityData(DatasetPtrs& p, EntityData& ed) {
		T* o = dynamic_cast<T*>(&ed);
		ASSERT(o);
		DatasetAssigner<T,0>::Set(p,o);
	}
	static Array<Factory>& List() {static Array<Factory> f; return f;}
	static void Set(DatasetPtrs& p, int o_kind, EntityData& data) {
		for (const auto& f : List()) {
			if (o_kind == f.kind) {
				f.set_data_ed_fn(p, data);
				return;
			}
		}
		ASSERT_(0, "Kind not registered");
	}
	template <class T> inline static void Register(String name) {
		#ifdef flagGUI
		static_assert(!std::is_base_of<::UPP::Ctrl, T>::value);
		#endif
		Factory& f = List().Add();
		f.kind = T::GetKind();
		f.category = FindKindCategory(f.kind);
		f.name = name;
		f.new_fn = &Functions<T>::Create;
		f.is_fn = &Functions<T>::IsNodeExt;
		f.set_data_ed_fn = &DatasetEntityData<T>;
		f.create_ed_fn = &EntityDataCreator<std::is_base_of<::UPP::EntityData,T>::value,T>::New;
		f.type = &typeid(T);
	}
	
	template <class Comp, class Ctrl> static void RegisterCtrl(String ctrl_name) {
		#ifdef flagGUI
		static_assert(std::is_base_of<::UPP::Ctrl, Ctrl>::value);
		static_assert(!std::is_base_of<::UPP::Ctrl, Comp>::value);
		#endif
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
	static int FindKindCategory(int kind);
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
	MetaNode* Detach(MetaNode* n);
	MetaNode* Detach(int i);
	void Remove(MetaNode* n);
	void Remove(int i);
	String GetTreeString(int depth=0) const;
	int Find(int kind, const String& id) const;
	int Find(const String& id) const;
	MetaNode* FindPath(const VfsPath& path);
	hash_t GetTotalHash() const;
	hash_t GetSourceHash(bool* total_hash_diffs=0) const;
	void Visit(Vis& v);
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
	VfsPath GetPath() const;
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
	
	template <class T> T& GetExt() {
		return dynamic_cast<T&>(*ext);
	}
	
	template <class T> T* GetOwnerExt() const {
		return owner ? owner->ext ? CastPtr<T>(&*owner->ext) : 0 : 0;
	}
};

using Nod = MetaNode;

struct MetaNodeSubset {
	Array<MetaNodeSubset> sub;
	Ptr<MetaNode> n;
	
	MetaNodeSubset() {}
	//MetaNodeSubset(MetaNode& n) : n(&n) {}
	void Clear() {sub.Clear(); n = 0;}
};


typedef enum : byte {
	MERGEMODE_OVERWRITE_OLD,
	MERGEMODE_KEEP_OLD,
	MERGEMODE_UPDATE_SUBSET,
} MergeMode;

struct MetaEnvironment : VFS {
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
	
	MetaNode root;
	RWMutex lock;
	
	typedef MetaEnvironment CLASSNAME;
	MetaEnvironment();
	hash_t NewSerial();
	hash_t CurrentSerial() const {return serial_counter;}
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	static bool IsMergeable(CXCursorKind kind);
	static bool IsMergeable(int kind);
	bool MergeNode(MetaNode& root, const MetaNode& other, MergeMode mode);
	bool MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	bool MergeVisitPartMatching(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode);
	void RefreshNodePtrs(MetaNode& n);
	void MergeVisitPost(MetaNode& n);
	MetaNode* FindDeclaration(const MetaNode& n);
	MetaNode* FindTypeDeclaration(hash_t type_hash);
	Vector<MetaNode*> FindDeclarationsDeep(const MetaNode& n);
	bool MergeResolver(ClangTypeResolver& ctr);
	hash_t RealizeTypePath(const String& path);
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
};

MetaEnvironment& MetaEnv();





template <class T>
bool VisitFromJson(T& var, const char *json)
{
	try {
		Value jv = ParseJSON(json);
		if(jv.IsError())
			return false;
		JsonIO io(jv);
		Vis vis(io);
		var.Visit(vis);
	}
	catch(ValueTypeError) {
		return false;
	}
	catch(JsonizeError) {
		return false;
	}
	return true;
}

template <class T>
bool VisitFromJsonFile(T& var, const char *file = NULL)
{
	return VisitFromJson(var, LoadFile(sJsonFile(file)));
}

template <class T>
String VisitToJson(T& var)
{
	try {
		JsonIO io;
		Vis vis(io);
		var.Visit(vis);
		Value val = io.GetResult();
		return AsJSON(val);
	}
	catch (...) {
		return String();
	}
}

template <class T> inline hash_t GetVisitJsonHash(const T& o) {return VisitToJson<T>(const_cast<T&>(o)).GetHashValue();}

template <class T>
bool DoVisitToJson(T& var, String& res, bool pretty=false)
{
	try {
		JsonIO io;
		Vis vis(io);
		var.Visit(vis);
		if (vis.IsError())
			return false;
		Value val = io.GetResult();
		res = AsJSON(val, pretty);
	}
	catch (...) {
		return false;
	}
	return true;
}

template <class T>
bool VisitToJsonFile(T& var, const char *file = NULL)
{
	try {
		String json = VisitToJson(var);
		FileOut s(file);
		s << json;
		s.Close();
	}
	catch (...) {
		return false;
	}
	return true;
}

template <class T>
void VisitCopy(const T& src, T& dst) {
	StringStream ss;
	{
		Vis vis(ss);
		ASSERT(vis.IsStoring());
		const_cast<T&>(src).Visit(vis);
	}
	ss.Seek(0);
	ss.SetLoading();
	{
		Vis vis(ss);
		ASSERT(vis.IsLoading());
		dst.Visit(vis);
	}
}


#endif
