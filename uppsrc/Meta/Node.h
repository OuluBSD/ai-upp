#ifndef _Meta_Node_h_
#define _Meta_Node_h_

struct VfsValue;
struct VfsValueExtCtrl;
struct VfsSrcPkg;
struct VfsValueExt;
struct Entity;
struct IdeShellHost;
struct IdeShell;
class ClangTypeResolver;
class ToolAppCtrl;

#define DATASET_ITEM(type, name, desc) struct type;
EXT_LIST
#undef DATASET_ITEM
struct SrcTextData;

struct EntityData : Pte<EntityData> {
	virtual ~EntityData() {}
	virtual int GetKind() const = 0;
	virtual void Visit(Vis& s) = 0;
};

// Deprecated
// DatasetPtrs is used only classes with "public SolverBase"
// The SolverBase is also being shifted out.
struct DatasetPtrs {
	#define DATASET_ITEM(type, name, desc) Ptr<type> name;
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
// declaration, which has been done when VfsValueExtFactory::Register is being called.
template <class T, int I>
struct DatasetAssigner {
	static void Set(DatasetPtrs& p, T* o);
};

#define DATASET_ITEM(type, name, desc) \
template <int I> struct DatasetAssigner<type,I> {\
	static void Set(DatasetPtrs& p, type* o) {p.name = o;}\
};
EXT_LIST
#undef DATASET_ITEM

#define DATASET_ITEM(type, name, desc) \
template <class _> struct DatasetPtrs::Getter<type,_> {static type& Get(DatasetPtrs& p) {return *p.name;}};
EXT_LIST
#undef DATASET_ITEM

void FillDataset(DatasetPtrs& p, VfsValue& n, Component* this_comp);

class DatasetProvider {
public:
	virtual DatasetPtrs GetDataset() const = 0;
	
};


#if 0
struct NodeDistance {
	virtual ~NodeDistance() {}
	virtual void Assign(const NodeDistance& d) {value = d.value;}
	double value = 0;
	NodeDistance() {}
	NodeDistance(double v) : value(v) {}
	NodeDistance(const NodeDistance& d) {Assign(d);}
	NodeDistance(NodeDistance&& d) {Assign(d);}
	void operator=(const NodeDistance& d) {Assign(d);}
};
#endif

struct NodeRoute {
	Vector<VfsValue*> route;
	bool from_owner_only = false;
};

struct VfsValueExt : Pte<VfsValueExt> {
	VfsValue& val;
	
	VfsValueExt(VfsValue& n) : val(n) {}
	virtual ~VfsValueExt() {}
	virtual void Visit(Vis& s) = 0;
	virtual TypeCls GetTypeCls() const = 0;
	virtual String GetTypeName() const = 0;
	virtual hash_t GetTypeHash() const = 0;
	virtual String GetName() const {return String();}
	virtual double GetUtility() {ASSERT_(0, "Not implemented"); return 0;}
	virtual double GetEstimate() {ASSERT_(0, "Not implemented"); return 0;}
	virtual double GetDistance(VfsValue& dest) {ASSERT_(0, "Not implemented"); return 0;}
	virtual bool TerminalTest(NodeRoute& prev) {ASSERT_(0, "Not implemented"); return true;}
	virtual String ToString() const {return String();}
	hash_t GetHashValue() const;
	int AstGetKind() const;
	
	static String GetCategory() {return String();}
	
	void CopyFrom(const VfsValueExt& e);
	bool operator==(const VfsValueExt& e) const;
	void Serialize(Stream& s);
	void Jsonize(JsonIO& json);
};

#define CLASSTYPE(x) \
	typedef x CLASSNAME; \
	TypeCls GetTypeCls() const override {return typeid(x);} \
	String GetTypeName() const override {return #x;} \
	hash_t GetTypeHash() const override {return TypedStringHasher<x>(#x);} \

#define DEFAULT_EXT(x) CLASSTYPE(x) x(VfsValue& n) : VfsValueExt(n) {}

template <bool b, class T>
struct EntityDataCreator {static EntityData* CreateEntityDataFn();};
template <class T> struct EntityDataCreator<false,T> {static EntityData* New() {return 0;}};
template <class T> struct EntityDataCreator<true, T> {static EntityData* New() {return new T;}};

struct VfsValueExtFactory {
	typedef VfsValueExt* (*NewFn)(VfsValue&);
	typedef VfsValueExtCtrl* (*NewCtrl)();
	typedef bool (*IsFn)(const VfsValueExt& e);
	//typedef void (*SetDatasetEntityData)(DatasetPtrs&, EntityData&);
	typedef EntityData* (*CreateEntityData)();
	
	struct Factory {
		hash_t type_hash;
		int category;
		String name;
		String ctrl_name;
		NewFn new_fn = 0;
		NewCtrl new_ctrl_fn = 0;
		//SetDatasetEntityData set_data_ed_fn = 0;
		CreateEntityData create_ed_fn = 0;
		IsFn is_fn = 0;
		const std::type_info* type = 0;
	};
	
	
	template<class T> struct Functions {
		static VfsValueExt* Create(VfsValue& owner) {VfsValueExt* c = new T(owner); return c;}
		static bool IsNodeExt(const VfsValueExt& e) {return dynamic_cast<const T*>(&e);}
	};
	template<class T> struct CtrlFunctions {
		static VfsValueExtCtrl* CreateCtrl() {return new T;}
	};
	/*template <class T> static void DatasetEntityData(DatasetPtrs& p, EntityData& ed) {
		T* o = dynamic_cast<T*>(&ed);
		ASSERT(o);
		DatasetAssigner<T,0>::Set(p,o);
	}*/
	static int FindTypeHashFactory(hash_t h);
	static Array<Factory>& List() {static Array<Factory> f; return f;}
	/*static void Set(DatasetPtrs& p, int o_kind, EntityData& data) {
		for (const auto& f : List()) {
			if (o_kind == f.kind) {
				f.set_data_ed_fn(p, data);
				return;
			}
		}
		ASSERT_(0, "Kind not registered");
	}*/
	static Index<String>& Categories() {static Index<String> v; return v;}
	
	template <class T> inline static void Register(String name) {
		#ifdef flagGUI
		static_assert(!std::is_base_of<::UPP::Ctrl, T>::value);
		#endif
		Factory& f = List().Add();
		f.type_hash = TypedStringHasher<T>(name);
		f.category = Categories().FindAdd(T::GetCategory());
		f.name = name;
		f.new_fn = &Functions<T>::Create;
		f.is_fn = &Functions<T>::IsNodeExt;
		//f.set_data_ed_fn = &DatasetEntityData<T>;
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
	static VfsValueExt* Create(hash_t type_hash, VfsValue& owner);
	//static VfsValueExt* CloneKind(int kind, const VfsValueExt& e, VfsValue& owner);
	static VfsValueExt* Clone(const VfsValueExt& e, VfsValue& owner);
	//static int FindKindFactory(int kind);
	static int AstFindKindCategory(int kind);
	
	int FindComponent(hash_t type_hash) {
		// note: must be added under entity
		TODO
		return -1;
	}
};

#define INITIALIZER_COMPONENT(x) INITIALIZER(x) {VfsValueExtFactory::Register<x>(#x);}
#define INITIALIZER_COMPONENT_CTRL(comp,ctrl) INITIALIZER(ctrl) {VfsValueExtFactory::RegisterCtrl<comp,ctrl>(#ctrl);}

struct AstValue {
	int             kind = -1;
	String          type;
	Point           begin = Null;
	Point           end = Null;
	hash_t          filepos_hash = 0;
	bool            is_ref = false;
	bool            is_definition = false;
	bool            is_disabled = false;
	
	// Temp
	Ptr<VfsValue>   type_ptr;
	
	bool IsNullInstance() const;
	void Serialize(Stream& s);
	void Xmlize(XmlIO& xml);
	void Jsonize(JsonIO& io);
	hash_t GetHashValue() const;
	String ToString() const;
	bool operator==(const AstValue& v) const;
	int Compare(const AstValue& v) const;
	int PolyCompare(const Value& v) const;
};

const dword ASTVALUE_V   = 0x10001;
template<> inline dword ValueTypeNo(const AstValue*)     { return ASTVALUE_V; }

struct VfsValue : Pte<VfsValue> {
	String             id;
	hash_t             type_hash = 0;
	hash_t             serial = 0;
	int                file = -1;
	Array<VfsValue>    sub;
	Value              value;
	One<VfsValueExt>   ext;
	
	// Temp
	int                pkg = -1;
	Ptr<VfsValue>      owner;
	Ptr<VfsValue>      symbolic_link;
	
	#ifdef flagDEBUG
	bool               only_temporary = false;
	#endif
	
	#if DEBUG_METANODE_DTOR
	bool               trace_kill = false;
	#endif
	
	VfsValue() {}
	VfsValue(VfsValue* owner, const VfsValue& n) {Assign(owner, n);}
	~VfsValue();
	VfsValue& operator[](int i);
	void ClearExtDeep();
	void Destroy();
	void Assign(VfsValue* owner, const VfsValue& n) {this->owner = owner; CopySubFrom(n); CopyFieldsFrom(n);}
	void CopyFrom(const VfsValue& n);
	void CopyFieldsFrom(const VfsValue& n, bool forced_downgrade=false);
	void CopySubFrom(const VfsValue& n);
	bool FindDifferences(const VfsValue& n, Vector<String>& diffs, int max_diffs=30) const;
	
	
	// Functions to use when value is AstValue type.
	bool IsAstValue() const;
	int GetAstValueCount() const;
	String AstGetKindString() const;
	static String AstGetKindString(int i);
	VfsValue& AstGetAdd(String id, String type, int kind);
	VfsValue& AstAdd(int kind, String id=String());
	int AstFind(int kind, const String& id) const;
	Vector<VfsValue*> AstFindAllShallow(int kind);
	Vector<const VfsValue*> AstFindAllShallow(int kind) const;
	void AstFindAllDeep(int kind, Vector<VfsValue*>& out);
	void AstFindAllDeep(int kind, Vector<const VfsValue*>& out) const;
	void AstRemoveAllShallow(int kind);
	void AstRemoveAllDeep(int kind);
	hash_t AstGetSourceHash(bool* total_hash_diffs=0) const;
	
	
	VfsValue& Add(const VfsValue& n);
	VfsValue& Add(VfsValue* n);
	VfsValue& Add(String id=String());
	VfsValue* Detach(VfsValue* n);
	VfsValue* Detach(int i);
	void Remove(VfsValue* n);
	void Remove(int i);
	String GetTreeString(int depth=0) const;
	String GetTypeString() const;
	int Find(const String& id) const;
	VfsValue* FindPath(const VfsPath& path);
	hash_t GetTotalHash() const;
	void Visit(Vis& v);
	void FixParent() {for (auto& s : sub) s.owner = this;}
	void PointPkgTo(VfsValueSubset& other, int pkg_id);
	void PointPkgTo(VfsValueSubset& other, int pkg_id, int file_id);
	void CopyPkgTo(VfsValue& other, int pkg_id) const;
	void CopyPkgTo(VfsValue& other, int pkg_id, int file_id) const;
	bool HasPkgDeep(int pkg_id) const;
	bool HasPkgFileDeep(int pkg_id, int file_id) const;
	void SetPkgDeep(int pkg_id);
	void SetFileDeep(int file_id);
	void SetPkgFileDeep(int pkg_id, int file_id);
	void SetTempDeep();
	Vector<VfsValue*> FindAll(TypeCls type);
	Vector<VfsValue*> FindTypeAllShallow(hash_t type_hash);
	VfsValue* FindDeep(TypeCls type);
	bool IsFieldsSame(const VfsValue& n) const;
	bool IsStructKind() const;
	//bool IsClassTemplateDefinition() const;
	String GetBasesString() const;
	String GetNestString() const;
	bool OwnerRecursive(const VfsValue& n) const;
	bool ContainsDeep(const VfsValue& n) const;
	void GetTypeHashes(Index<hash_t>& type_hashes) const;
	void RealizeSerial();
	void FixSerialDeep();
	Vector<Ptr<VfsValueExt>> GetAllExtensions();
	VfsPath GetPath() const;
	void DeepChk();
	void Chk();
	bool IsOwnerDeep(VfsValueExt& n) const;
	int GetCount() const;
	int GetDepth() const;
	
	operator AstValue&();
	operator AstValue*();
	operator const AstValue*() const;
	
	template <class T>
	bool IsTypeHash() const {
		return type_hash == AsTypeHash<T>();
	}
	
	template <class T>
	T& CreateExt() {
		ext.Clear();
		T* o = new T(*this);
		ext = o;
		type_hash = AsTypeHash<T>();
		return *o;
	}
	
	template <class T>
	T& GetAdd(String id="") {
		hash_t type_hash = AsTypeHash<T>();
		ASSERT(type_hash);
		for (auto& s : sub) {
			if (s.type_hash == type_hash && s.id == id) {
				ASSERT(s.ext);
				throw Exc("internal error: no ext");
				T* o = CastPtr<T>(&*s.ext);
				ASSERT(o);
				throw Exc("internal error: empty pointer");
				return *o;
			}
		}
		VfsValue& s = Add();
		s.id = id;
		T* o = new T(s);
		s.ext = o;
		s.owner = this;
		s.pkg = pkg;
		s.file = file;
		s.type_hash = type_hash;
		return *o;
	}
	
	template <class T>
	T& Add(String id="") {
		VfsValue& s = Add();
		s.id = id;
		T* o = new T(s);
		s.ext = o;
		s.owner = this;
		s.pkg = pkg;
		s.file = file;
		s.type_hash = AsTypeHash<T>();
		ASSERT(s.type_hash);
		return *o;
	}
	
	template <class T>
	Ptr<T> FindPath(const VfsPath& vfs) {
		VfsValue* n = this;
		for(const Value& key : vfs.Parts()) {
			String key_str = key.ToString();
			int i = n->Find(key_str);
			if (i >= 0)
				n = &n->sub[i];
			else
				break;
		}
		if (n && n->ext)
			return &*CastPtr<T>(&*n->ext);
		return 0;
	}
	
	template <class T>
	T* Find() {
		for (auto& s : sub) {
			if (s.ext) {
				T* o = dynamic_cast<T*>(&*s.ext);
				if (o) return o;
			}
		}
		return 0;
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
	
	template <class T> T& GetExt() {
		return dynamic_cast<T&>(*ext);
	}
	
	template <class T> T* GetOwnerExt() const {
		return owner ? owner->ext ? CastPtr<T>(&*owner->ext) : 0 : 0;
	}
	
	template <class T> T* FindOwner(int max_depth=-1) const {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			if (n->ext && n->ext->GetTypeCls() == type) {
				T* o = CastPtr<T>(&*n->ext);
				ASSERT(o);
				return o;
			}
			n = n->owner;
		}
		return 0;
	}
	
	template <class T> T* FindRoot(int max_depth=-1) {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = this;
		T* root = 0;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			if (n->ext && n->ext->GetTypeCls() == type) {
				T* o = CastPtr<T>(&*n->ext);
				ASSERT(o);
				root = o;
			}
			n = n->owner;
		}
		return root;
	}
	
	template <class T> T* FindOwnerRoot(int max_depth=-1) const {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = owner;
		T* root = 0;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			if (n->ext && n->ext->GetTypeCls() == type) {
				T* o = CastPtr<T>(&*n->ext);
				ASSERT(o);
				root = o;
			}
			n = n->owner;
		}
		return root;
	}
	
	template <class T> T* FindOwnerWith(int max_depth=-1) const {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			for (auto& s : n->sub) {
				if (s.ext && s.ext->GetTypeCls() == type) {
					T* o = CastPtr<T>(&*s.ext);
					ASSERT(o);
					return o;
				}
			}
			n = n->owner;
		}
		return 0;
	}
	
	template <class T> T* FindOwnerWithCast(int max_depth=-1) const {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			for (auto& s : n->sub) {
				if (s.ext) {
					T* o = CastPtr<T>(&*s.ext);
					if (o)
						return o;
				}
			}
			n = n->owner;
		}
		return 0;
	}
	
	template <class T> T* FindOwnerRootWith(int max_depth=-1) const {
		TypeCls type = AsTypeCls<T>();
		VfsValue* n = owner;
		T* root = 0;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			for (auto& s : n->sub) {
				if (s.ext && s.ext->GetTypeCls() == type) {
					T* o = CastPtr<T>(&*s.ext);
					ASSERT(o);
					root = o;
				}
			}
			n = n->owner;
		}
		return root;
	}
	
	template <class T> Vector<Ptr<T>> FindAllDeep() {
		Vector<Ptr<T>> v;
		FindAllDeep0<T>(v);
		return v;
	}
	template <class T> void FindAllDeep0(Vector<Ptr<T>>& v) {
		for (auto& s : sub) {
			T* o = s.ext ? CastPtr<T>(&*s.ext) : 0;
			if (o)
				v.Add(o);
		}
		for (auto& s : sub) {
			s.FindAllDeep0<T>(v);
		}
	}
	template <class T> void RemoveAllDeep() {
		Vector<int> rmlist;
		int i = 0;
		for (auto& s : sub) {
			T* o = s.ext ? CastPtr<T>(&*s.ext) : 0;
			if (o)
				rmlist << i;
			else
				s.RemoveAllDeep<T>();
			i++;
		}
		if (!rmlist.IsEmpty())
			sub.Remove(rmlist);
	}
	template <class T> void RemoveAllShallow() {
		Vector<int> rmlist;
		int i = 0;
		for (auto& s : sub) {
			T* o = s.ext ? CastPtr<T>(&*s.ext) : 0;
			if (o)
				rmlist << i;
			i++;
		}
		if (!rmlist.IsEmpty())
			sub.Remove(rmlist);
	}
	
	
public:
	using Val = VfsValue;
	
	typedef typename Array<VfsValue>::Iterator Iterator;
	typedef typename Array<VfsValue>::ConstIterator ConstIterator;
	
	class IteratorDeep {
		Iterator begin;
		Vector<Val*> cur;
		Vector<int> pos;
	protected:
		friend struct VfsValue;
		IteratorDeep(Val* cur) {
			this->cur.Add(cur);
			pos.Add(0);
		}
		
		IteratorDeep(const Val* cur) {
			this->cur.Add((Val*)cur);
			pos.Add(0);
		}
		
	public:
		IteratorDeep(const IteratorDeep& it) {
			*this = it;
		}
		IteratorDeep& operator = (const IteratorDeep& it) {
			begin = it.begin;
			cur <<= it.cur;
			pos <<= it.pos;
			return *this;
		}
		int GetDepth() {return pos.GetCount();}
		int GetPos() {return pos[pos.GetCount()-1];}
		int64 GetCurrentAddr() {return (int64)cur[cur.GetCount() - 1];}
		
		bool IsEnd() const {return pos.GetCount() == 1 && pos[0] == 1;}
		bool operator == (IteratorDeep& it) {return GetCurrentAddr() == it.GetCurrentAddr();}
		bool operator != (IteratorDeep& it) {return !(*this == it);}
		void operator ++(int i) {
			#define LASTPOS pos[pos.GetCount()-1]
			#define LASTCUR cur[cur.GetCount()-1]
			#define SECLASTCUR cur[cur.GetCount()-2]
			#define ADDPOS pos[pos.GetCount()-1]++
			#define ADDCUR LASTCUR = &SECLASTCUR->sub[LASTPOS]
			#define REMLASTPOS pos.Remove(pos.GetCount()-1)
			#define REMLASTCUR cur.Remove(cur.GetCount()-1)
			//String s; for(int i = 0; i < pos.GetCount(); i++) s << " " << pos[i]; LOG("+++ " << s);
			
			if (pos.GetCount() == 1 && pos[0] < 0) {pos[0]++; return;}
			if (pos.GetCount() == 1 && pos[0] == 1) return; // at end
			
			if (LASTCUR->GetCount()) {
				pos.Add(0);
				cur.Add(&LASTCUR->sub[0]);
			}
			else if (pos.GetCount() == 1) {
				pos[0] = 1; // at end
			}
			else {
				while (1) {
					if (LASTPOS + 1 < SECLASTCUR->GetCount()) {
						ADDPOS;
						ADDCUR;
						break;
					} else {
						REMLASTPOS;
						REMLASTCUR;
						if (pos.GetCount() <= 1) {
							pos.SetCount(1);
							pos[0] = 1;
							break;
						}
					}
				}
			}
		}
		void IncNotDeep() {
			if (LASTCUR->GetCount()) {
				while (1) {
					if (cur.GetCount() >= 2 && LASTPOS + 1 < SECLASTCUR->GetCount()) {
						ADDPOS;
						ADDCUR;
						break;
					} else {
						REMLASTPOS;
						REMLASTCUR;
						if (pos.GetCount() <= 1) {
							pos.SetCount(1);
							pos[0] = 1;
							break;
						}
					}
				}
			}
			else operator++(1);
		}
		void DecToParent() {
			pos.Remove(pos.GetCount()-1);
			cur.Remove(cur.GetCount()-1);
		}
		
		operator Val*() {
			if (pos.GetCount() && pos[0] == 1) return 0;
			return LASTCUR;
		}
		
		Val* operator->() {
			if (pos.GetCount() && pos[0] == 1) return 0;
			return LASTCUR;
		}
		
		Val& operator*() {
			return *LASTCUR;
		}
		
		const Val& operator*() const {return *LASTCUR;}
		
		Val* Higher() {
			if (cur.GetCount() <= 1) return 0;
			return cur[cur.GetCount()-2];
		}
	};
	
	
	IteratorDeep		BeginDeep() { return IteratorDeep(this);}
	const IteratorDeep	BeginDeep() const { return IteratorDeep(this);}
	Iterator			Begin() { return sub.Begin(); }
	Iterator			End() { return sub.End(); }
	
};

using Val = VfsValue;

struct VfsValueSubset {
	Array<VfsValueSubset> sub;
	Ptr<VfsValue> n;
	
	VfsValueSubset() {}
	//VfsValueSubset(VfsValue& n) : n(&n) {}
	void Clear() {sub.Clear(); n = 0;}
};


typedef enum : byte {
	MERGEMODE_OVERWRITE_OLD,
	MERGEMODE_KEEP_OLD,
	MERGEMODE_UPDATE_SUBSET,
} MergeMode;

struct MetaEnvironment : VFS {
	struct FilePos : Moveable<FilePos> {
		Vector<Ptr<VfsValue>> hash_nodes;
	};
	struct Type : Moveable<Type> {
		Vector<Ptr<VfsValue>> hash_nodes;
		String seen_type;
	};
	VectorMap<hash_t,FilePos> filepos;
	VectorMap<hash_t,Type> types;
	hash_t serial_counter = 0;
	SpinLock serial_lock;
	
	VfsValue root;
	RWMutex lock;
	
	typedef MetaEnvironment CLASSNAME;
	MetaEnvironment();
	hash_t NewSerial();
	hash_t CurrentSerial() const {return serial_counter;}
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	static bool IsMergeable(CXCursorKind kind);
	static bool IsMergeable(int kind);
	bool MergeNode(VfsValue& root, const VfsValue& other, MergeMode mode);
	bool MergeVisit(Vector<VfsValue*>& scope, const VfsValue& n1, MergeMode mode);
	bool MergeVisitPartMatching(Vector<VfsValue*>& scope, const VfsValue& n1, MergeMode mode);
	void RefreshNodePtrs(VfsValue& n);
	void MergeVisitPost(VfsValue& n);
	VfsValue* FindDeclaration(const VfsValue& n);
	VfsValue* FindTypeDeclaration(hash_t type_hash);
	Vector<VfsValue*> FindDeclarationsDeep(const VfsValue& n);
	bool MergeResolver(ClangTypeResolver& ctr);
	hash_t RealizeTypePath(const String& path);
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
};

MetaEnvironment& MetaEnv();





#endif
