#ifndef _Meta_VfsValue_h_
#define _Meta_VfsValue_h_

#include <Vfs/Core/Core.h>
#include <Vfs/Factory/VfsFactory.h>
#include <Vfs/Overlay/VfsOverlay.h>
#include <Vfs/Overlay/Precedence.h>

struct VfsValue;
struct VfsValueExtCtrl;
struct VfsSrcPkg;
struct VfsValueExt;
struct Entity;
struct IdeShellHost;
struct IdeShell;
class ClangTypeResolver;
class ToolAppCtrl;
struct VfsValueSubset;
struct DatasetPtrs;


template <> void JsonizeArray<Array<VfsValue>,CallbackN<JsonIO&,VfsValue&>>(JsonIO& io, Array<VfsValue>& array, CallbackN<JsonIO&,VfsValue&> item_jsonize, void* arg);


struct EntityData : Pte<EntityData> {
	virtual ~EntityData() {}
	
	// Use CLASSTYPE(x) macro to override these
	virtual hash_t GetTypeHash() const = 0;
	virtual TypeCls GetTypeCls() const = 0;
	virtual String GetTypeName() const = 0;
	virtual void* GetTypePtr() const = 0;
	virtual void Visit(Vis& s) = 0;
};


struct NodeRoute {
	Vector<VfsValue*> route;
	bool from_owner_only = false;
};

struct VfsValueExt : Pte<VfsValueExt> {
	using VfsValueExtPtr = Ptr<VfsValueExt>;
	VfsValue&				val;
	bool					is_initialized = false;
	
	VfsValueExt(VfsValue& n);
	virtual ~VfsValueExt();
	virtual void			Visit(Vis& s) = 0;
	virtual TypeCls			GetTypeCls() const = 0;
	virtual String			GetTypeName() const = 0;
	virtual hash_t			GetTypeHash() const = 0;
	virtual void*			GetTypePtr() const = 0;
	virtual String			GetName() const;
	virtual double			GetUtility();
	virtual double			GetEstimate();
	virtual double			GetDistance(VfsValue& dest);
	virtual bool			GenerateSubValues(const Value& params, NodeRoute& prev);
	virtual bool			TerminalTest();
	virtual String			ToString() const;
	virtual bool			Start();
	virtual void			Stop();
	virtual bool			Initialize(const WorldState& ws);
	virtual bool			PostInitialize();
	virtual void			Uninitialize();
	virtual void			UninitializeDeep();
	virtual void			Update(double dt);
	virtual bool			Arg(String key, Value value);
	virtual bool			AddDependency(VfsValueExt& val);
	virtual VfsValueExtPtr	GetDependency(int i) const;
	virtual int				GetDependencyCount() const;
	virtual void			RemoveDependency(int i);
	virtual void			ClearDependencies();
	virtual void			RemoveDependency(const VfsValueExt* e);
	virtual String			GetTreeString(int indent=0) const;
	virtual bool			GetAll(Vector<VirtualNode>&);
	hash_t					GetHashValue() const;
	int						AstGetKind() const;
	bool					IsInitialized() const;
	void					SetInitialized(bool b=true);
	void					CopyFrom(const VfsValueExt& e);
	bool					operator==(const VfsValueExt& e) const;
	void					Serialize(Stream& s);
	void					Jsonize(JsonIO& json);
	
	static String			GetCategory();
	
protected:
	Vector<VfsValueExtPtr>	deps;
};

using VfsValueExtPtr = Ptr<VfsValueExt>;

#define CLASSTYPE(x) \
	typedef x CLASSNAME; \
	static TypeCls AsTypeCls() {return typeid(x);} \
	TypeCls GetTypeCls() const override {return typeid(x);} \
	String GetTypeName() const override {return #x;} \
	hash_t GetTypeHash() const override {return TypedStringHasher<x>(#x);} \
	void* GetTypePtr() const override {return (void*)this;} \

#define DEFAULT_EXT(x) CLASSTYPE(x) x(VfsValue& n) : VfsValueExt(n) {}

template <bool b, class T>
struct EntityDataCreator {static EntityData* CreateEntityDataFn();};
template <class T> struct EntityDataCreator<false,T> {static EntityData* New() {return 0;}};
template <class T> struct EntityDataCreator<true, T> {static EntityData* New() {return new T;}};

typedef enum {
	VFSEXT_DEFAULT,
	VFSEXT_SYSTEM_ECS,
	VFSEXT_SYSTEM_ATOM,
	VFSEXT_COMPONENT,
	VFSEXT_ATOM,
	VFSEXT_LINK,
	VFSEXT_EXCHANGE,
} VfsExtType;

struct VfsValueExtFactory {
	typedef VfsValueExt* (*NewFn)(VfsValue&);
	typedef VfsValueExtCtrl* (*NewCtrl)();
	typedef bool (*IsFn)(const VfsValueExt& e);
	typedef void (*SetDatasetPtr)(DatasetPtrs&, VfsValueExt&);
	typedef EntityData* (*CreateEntityData)();
	typedef LinkTypeCls (*GetLinkTypeFn)();
	
	struct Factory {
		VfsExtType			type;
		hash_t				type_hash = 0; // NOT same as TypeCls::GetHashValue()
		TypeCls				type_cls;
		String				category;
		String				name;
		String				eon_name;
		String				ctrl_name;
		NewFn				new_fn = 0;
		NewCtrl				new_ctrl_fn = 0;
		CreateEntityData	create_ed_fn = 0;
		SetDatasetPtr		set_ptr_fn = 0;
		IsFn				is_fn = 0;
	};
	
	struct AtomData : Moveable<AtomData> {
		NewFn				new_fn;
		String				name;
		AtomTypeCls			cls;
		TypeCls				rtti_cls;
		LinkTypeCls			link_type;
		Vector<String>		actions;
	};
	
	struct LinkData : Moveable<LinkData> {
		NewFn				new_fn;
		String				name;
		LinkTypeCls			cls;
		TypeCls				rtti_cls;
	};
	
	struct IfaceData : Moveable<IfaceData> {
		hash_t				type_hash = 0;
		TypeCls				cls;
		ValDevCls			vd;
		String				name;
	};
	
	/*struct ExptData : Moveable<ExptData> {
		NewExpt new_fn;
	};*/
	
	static VectorMap<String, TypeCls>& EonToType() {
		static VectorMap<String, TypeCls> m;
		return m;
	}
    
	template<class T> struct Functions {
		static VfsValueExt* Create(VfsValue& owner) {VfsValueExt* c = new T(owner); return c;}
		static bool IsNodeExt(const VfsValueExt& e) {return dynamic_cast<const T*>(&e);}
	};
	template<class T> struct CtrlFunctions {
		static VfsValueExtCtrl* CreateCtrl() {return new T;}
	};
	
	template<class T> static void SetDatasetData(DatasetPtrs& p, VfsValueExt& ext) {}
	static void SetDatasetPtrs(DatasetPtrs& p, VfsValueExt& ext);
	/*template <class T> static void DatasetEntityData(DatasetPtrs& p, EntityData& ed) {
		T* o = dynamic_cast<T*>(&ed);
		ASSERT(o);
		DatasetAssigner<T,0>::Set(p,o);
	}*/
	static int FindTypeHashFactory(hash_t h);
	static int FindTypeClsFactory(TypeCls t);
	static Array<Factory>& List() {static Array<Factory> f; return f;}
	/*static void SetEntityData(DatasetPtrs& p, hash_t type_hash, EntityData& data) {
		for (const auto& f : List()) {
			if (type_hash == f.type_hash) {
				f.set_data_ed_fn(p, data);
				return;
			}
		}
		ASSERT_(0, "Type not registered with given hash_type");
	}*/
	static Index<String>& Categories() {static Index<String> v; return v;}
	static VectorMap<AtomTypeCls,AtomData>& AtomDataMap() {static VectorMap<AtomTypeCls,AtomData> m; return m;}
	static VectorMap<LinkTypeCls,LinkData>& LinkDataMap() {static VectorMap<LinkTypeCls,LinkData> m; return m;}
	static VectorMap<ValDevTuple,IfaceData>& IfaceLinkDataMap() {static VectorMap<ValDevTuple,IfaceData> m; return m;}
	//static VectorMap<TypeCls,ExptData>& ExptDataMap() {static VectorMap<TypeCls,ExptData> m; return m;}
	static bool IsType(hash_t type_hash, VfsExtType t);
	template <class T> inline static void RegisterExchange(String name, DevCls dev, ValCls val) {
		if (!GetFactory<T>().type_hash)
			Register<T>(name, VFSEXT_EXCHANGE, "exchange." + ToVarName(name,'.'), "Sys|Exchange");
		ValDevCls vd(dev,val);
		IfaceData& d = IfaceLinkDataMap().GetAdd(vd);
		d.type_hash  = AsTypeHash<T>();
		d.cls        = AsTypeCls<T>();
		d.name       = AsTypeName<T>();
		d.vd.dev     = dev;
		d.vd.val     = val;
		//ExptData& d = ExptDataMap().GetAdd(AsTypeCls<T>()); // todo remove?
		//d.new_fn = &New<T>;
	}
	
	template <class T> inline static void RegisterAtom(String name) {
		Register<T>(name, VFSEXT_ATOM, "", "Atom|Uncategorised");
		AtomTypeCls cls = T::GetAtomType();
		AtomData& d = AtomDataMap().GetAdd(cls);
		d.rtti_cls = AsTypeCls<T>();
		d.cls = cls;
		d.name = AsTypeName<T>();
		d.new_fn = &Functions<T>::Create;
		d.link_type = T::GetLinkType();
		d.actions.Add(T::GetAction());
	}
	
	template <class T> static void RegisterLink(String name) {
		Register<T>(name, VFSEXT_LINK, "", "Link|Uncategorised");
		LinkTypeCls cls = T::GetLinkTypeStatic();
		ASSERT(cls.IsValid());
		LinkData& d = LinkDataMap().GetAdd(cls);
		d.rtti_cls = AsTypeCls<T>();
		d.cls = cls;
		d.name = AsTypeName<T>();
		d.new_fn = &Functions<T>::Create;
	}
	
	template <class T> inline static Factory& GetFactory() {
		TypeCls type_cls = AsTypeCls<T>();
		for (auto& f : List())
			if (f.type_cls == type_cls)
				return f;
		auto& f = List().Add();
		f.type_cls = type_cls;
		ASSERT(f.name.Find("|") < 0);
		return f;
	}
	inline static Factory* FindFactory(String name) {
		for (auto& f : List())
			if (f.name == name)
				return &f;
		return 0;
	}
	template <class T> inline static void Register(String name, VfsExtType type, String eon_name, String category) {
		Factory& f = GetFactory<T>();
		ASSERT(!f.type_hash);
		f.type_hash = TypedStringHasher<T>(name);
		f.type = type;
		f.category = category;
		f.name = name;
		f.eon_name = eon_name;
		f.new_fn = &Functions<T>::Create;
		f.is_fn = &Functions<T>::IsNodeExt;
		//f.set_data_ed_fn = &DatasetEntityData<T>;
		f.create_ed_fn = &EntityDataCreator<std::is_base_of<::UPP::EntityData,T>::value,T>::New;
		if (type == VFSEXT_SYSTEM_ECS)
			EonToType().Add(name, f.type_cls);
		if (type == VFSEXT_SYSTEM_ECS)
			f.set_ptr_fn = &SetDatasetData<T>;
		ASSERT(f.name.Find("|") < 0);
	}
	
	template <class Comp, class Ctrl> static void RegisterCtrl(String ctrl_name) {
		TypedStringHasher<Ctrl>(ctrl_name); // realize ctrl hash
		Factory& f = GetFactory<Comp>();
		ASSERT_(!f.new_ctrl_fn, "Only one Ctrl per Extension is supported currently, and one is already registered");
		f.new_ctrl_fn = &CtrlFunctions<Ctrl>::CreateCtrl;
		f.ctrl_name = ctrl_name;
		ASSERT(f.name.Find("|") < 0);
	}
	static VfsValueExt* Create(hash_t type_hash, VfsValue& owner);
	//static VfsValueExt* CloneKind(int kind, const VfsValueExt& e, VfsValue& owner);
	static VfsValueExt* Clone(const VfsValueExt& e, VfsValue& owner);
	//static int FindKindFactory(int kind);
	static int AstFindKindCategory(int kind);
	
	static int FindComponent(hash_t type_hash) {
		int i = 0;
		for (auto& l : List()) {
			if (l.type_hash == type_hash && l.type == VFSEXT_COMPONENT)
				return i;
			i++;
		}
		return -1;
	}
	
	static const Vector<Vector<String>>& GetCategories();
};

#define INITIALIZER_VFSEXT(x, eon, cat) INITIALIZER(x) {VfsValueExtFactory::Register<x>(#x, VFSEXT_DEFAULT, eon, cat);}
#define INITIALIZER_COMPONENT(x, eon, cat) INITIALIZER(x) {VfsValueExtFactory::Register<x>(#x, VFSEXT_COMPONENT, eon, cat);}
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
	hash_t             file_hash = 0;
	Array<VfsValue>    sub;
	Value              value;
	One<VfsValueExt>   ext;
	
	// Temp
	hash_t             pkg_hash = 0;
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
	void Assign(VfsValue* owner, const VfsValue& n);
	void CopyFrom(const VfsValue& n);
	void CopyFieldsFrom(const VfsValue& n, bool forced_downgrade=false);
	void CopySubFrom(const VfsValue& n);
	bool FindDifferences(const VfsValue& n, Vector<String>& diffs, int max_diffs=30) const;
	String ToString() const;
	
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
	String GetSourceHashDump(int indent=0, bool* total_hash_diffs=0) const;
	void AstFix();
	
	VfsValue* Resolve();
	VfsValue& Add(const VfsValue& n);
	VfsValue& Add(VfsValue* n);
	VfsValue& Add(String id=String(), hash_t h=0);
	VfsValue& Insert(int idx, String id=String(), hash_t h=0);
	VfsValue& Init(VfsValue& s, const String& id, hash_t h);
	VfsValue* Detach(VfsValue* n);
	VfsValue* Detach(int i);
	VfsValue& GetRoot();
	void SetCount(int i, hash_t type_hash=0);
	void Remove(VfsValue* n);
	void Remove(int i);
	String GetTreeString(int depth=0) const;
	String GetTypeString() const;
	int Find(const String& id) const;
	int Find(const String& id, hash_t type_hash) const;
	VfsValue* FindPath(const VfsPath& path);
	hash_t GetTotalHash() const;
	void Visit(Vis& v);
	void FixParent() {for (auto& s : sub) s.owner = this;}
	void PointPkgHashTo(VfsValueSubset& other, hash_t pkg);
	void PointPkgHashTo(VfsValueSubset& other, hash_t pkg, hash_t file);
	void CopyPkgHashTo(VfsValue& other, hash_t pkg) const;
	void CopyPkgHashTo(VfsValue& other, hash_t pkg, hash_t file) const;
	bool HasPkgHashDeep(hash_t pkg) const;
	bool HasPkgFileHashDeep(hash_t pkg, hash_t file) const;
	void SetPkgHashDeep(hash_t pkg);
	void SetFileHashDeep(hash_t pkg);
	void SetPkgFileHashDeep(hash_t pkg, hash_t file);
	void SetTempDeep();
	Vector<VfsValue*> FindAll(TypeCls type);
	Vector<VfsValue*> FindTypeAllShallow(hash_t type_hash);
	Vector<Ptr<VfsValue>> FindTypeAllShallowPtr(hash_t type_hash);
	VfsValue* FindDeep(TypeCls type);
	bool IsFieldsSame(const VfsValue& n) const;
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
	int GetDepth(const VfsValue* limit) const;
	Vector<VfsValue*> FindAllShallow(hash_t type_hash);
	int GetCount(String id) const;
	VfsValue& GetAdd(String id);
	VfsValue& GetAdd(String id, hash_t type_hash);
	void StopDeep();
	void ClearDependenciesDeep();
	void UninitializeDeep();
	void FindFiles(VectorMap<hash_t,VectorMap<hash_t,int>>& pkgfiles) const;
	VirtualNode				RootPolyValue();
	VirtualNode				RootVfsValue(const VfsPath& path);
	
	operator AstValue&();
	operator AstValue*();
	operator const AstValue*() const;
	
	template <class T>
	bool IsTypeHash() const {
		return type_hash == AsTypeHash<T>();
	}
	
	template <class T>
	bool Is() const {
		hash_t h = AsTypeHash<T>();
		return type_hash == h && ext && ext->GetTypeHash() == h;
	}
	
	template <class T>
	T& CreateExt() {
		ext.Clear();
		T* o = new T(*this);
		ext = o;
		type_hash = AsTypeHash<T>();
		return *o;
	}
	
	VfsValueExt& CreateExt(hash_t type_hash) {
		ASSERT(type_hash != 0);
		ext.Clear();
		VfsValueExt* o = VfsValueExtFactory::Create(type_hash, *this);
		ASSERT(o);
		ext = o;
		type_hash = o ? type_hash : 0;
		return *o;
	}
	
	template <class T>
	T& GetAdd(String id="") {
		hash_t type_hash = AsTypeHash<T>();
		ASSERT(type_hash);
		for (auto& s : sub) {
			if (s.type_hash == type_hash && s.id == id) {
				ASSERT(s.ext);
				if (!s.ext)
					throw Exc("internal error: no ext");
				T* o = CastPtr<T>(&*s.ext);
				ASSERT(o);
				if (!o)
					throw Exc("internal error: empty pointer");
				ASSERT(s.owner == this); // additional check
				return *o;
			}
		}
		VfsValue& s = Add();
		s.owner = this;
		s.id = id;
		T* o = new T(s);
		s.ext = o;
		s.pkg_hash = pkg_hash;
		s.file_hash = file_hash;
		s.type_hash = type_hash;
		return *o;
	}
	
	template <class T>
	T& Add(String id="") {
		VfsValue& s = Add();
		s.owner = this;
		s.id = id;
		T* o = new T(s);
		s.ext = o;
		s.pkg_hash = pkg_hash;
		s.file_hash = file_hash;
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
	T* Find(String id) {
		hash_t type_hash = AsTypeHash<T>();
		for (auto& s : sub) {
			if (s.type_hash == type_hash && s.id == id) {
				T* o = dynamic_cast<T*>(&*s.ext);
				if (o) return o;
			}
		}
		return 0;
	}
	
	template <class T>
	int FindPos(String id) {
		hash_t type_hash = AsTypeHash<T>();
		int i = 0;
		for (auto& s : sub) {
			if (s.type_hash == type_hash && s.id == id)
				return i;
			i++;
		}
		return -1;
	}
	
	template <class T>
	T* Find() {
		hash_t type_hash = AsTypeHash<T>();
		for (auto& s : sub) {
			if (s.type_hash == type_hash) {
				T* o = dynamic_cast<T*>(&*s.ext);
				if (o) return o;
			}
		}
		return 0;
	}
	
	template <class T>
	T* FindCast() {
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
	
	template <class T>
	Vector<Ptr<VfsValue>> FindAllWith() {
		Vector<Ptr<VfsValue>> v;
		for (auto& s0 : sub) {
			for (auto& s1 : s0.sub) {
				if (s1.ext) {
					T* o = dynamic_cast<T*>(&*s1.ext);
					if (o) {
						v.Add(&s0);
						break;
					}
				}
			}
		}
		return v;
	}
	
	template <class T>
	Vector<Ptr<T>> FindAllWithT() {
		Vector<Ptr<T>> v;
		for (auto& s0 : sub) {
			for (auto& s1 : s0.sub) {
				if (s1.ext) {
					T* o = dynamic_cast<T*>(&*s1.ext);
					if (o) {
						v.Add(o);
						break;
					}
				}
			}
		}
		return v;
	}
	
	template <class T> T* FindExt() {
		if (!ext)
			return 0;
		return dynamic_cast<T*>(&*ext);
	}
	
	template <class T> T& GetExt() {
		return dynamic_cast<T&>(*ext);
	}
	
	template <class T> T* GetOwnerExt() const {
		return owner ? owner->ext ? CastPtr<T>(&*owner->ext) : 0 : 0;
	}
	
	VfsValue* FindOwnerNull(int max_depth=-1) const {
		VfsValue* n = owner;
		int d = 1;
		while (n && n->type_hash && (max_depth < 0 || d++ <= max_depth))
			n = n->owner;
		return n;
	}
	
	bool HasOwnerDeep(VfsValue& val, int max_depth=-1) const {
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			if (n == &val)
				return true;
			n = n->owner;
		}
		return n == &val;
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
	
	template <class T> T* FindOwnerWith(String id, int max_depth=-1) const {
		hash_t type_hash = AsTypeHash<T>();
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			for (auto& s : n->sub) {
				if (s.id == id && s.type_hash == type_hash) {
					T* o = CastPtr<T>(&*s.ext);
					ASSERT(o);
					return o;
				}
			}
			n = n->owner;
		}
		return 0;
	}
	
	template <class T> T* FindOwnerWith(int max_depth=-1) const {
		hash_t type_hash = AsTypeHash<T>();
		VfsValue* n = owner;
		int d = 1;
		while (n && (max_depth < 0 || d++ <= max_depth)) {
			for (auto& s : n->sub) {
				if (s.type_hash == type_hash) {
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
	
public:
	template <class T>
	struct SubIterT {
		VfsValue& val;
		
		SubIterT(VfsValue& v) : val(v) {}
		SubIterT(SubIterT&& v) : val(v.val) {}
		int GetCount() const {
			hash_t type_hash = AsTypeHash<T>();
			int count = 0, i = -1;
			while (++i < val.sub.GetCount())
				if (val.sub[i].type_hash == type_hash)
					count++;
			return count;
		}
		bool IsEmpty() const {return GetCount() == 0;}
		
		struct Iterator {
			VfsValue& val;
			int pos = 0;
			Iterator(VfsValue& v, int i) : val(v), pos(i) {
				if (i < 0) {Inc();}
				else if (i == INT_MAX) {pos = val.sub.GetCount(); Dec();}
			}
			Iterator(Iterator&& v) : val(v.val), pos(v.pos) {}
			operator bool() const {return pos >= 0 && pos < val.sub.GetCount();}
			bool IsEnd() const {return pos < 0 && pos >= val.sub.GetCount();}
			void operator++() {Inc();}
			void operator++(int) {Inc();}
			void operator--() {Dec();}
			void operator--(int) {Dec();}
			void Inc() {
				hash_t type_hash = AsTypeHash<T>();
				while (++pos < val.sub.GetCount())
					if (val.sub[pos].type_hash == type_hash)
						break;
			}
			void Dec() {
				hash_t type_hash = AsTypeHash<T>();
				while (--pos >= 0)
					if (val.sub[pos].type_hash == type_hash)
						break;
			}
			operator T&() {return val.sub[pos].GetExt<T>();}
			T& operator*() {return val.sub[pos].GetExt <T>();}
		};
		Iterator begin() {return Iterator(val, -1);}
		Iterator end() {return Iterator(val, val.sub.GetCount());}
		Iterator rbegin() {return Iterator(val, INT_MAX);}
	};
	
	template <class T> SubIterT<T> Sub() {return SubIterT<T>(*this);}
};

using Val = VfsValue;
using ValPtr = Ptr<VfsValue>;
using VfsValuePtr = Ptr<VfsValue>;

template <class T>
inline T& VirtualNode::GetAddExt(String name) {
	ASSERT(data && data->mode == VFS_VALUE);
	ASSERT(data->vfs_value);
	return data->vfs_value->CreateExt<T>();
}

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
	WorldState env_ws;
	mutable Mutex seen_lock;
	
	// Track all distinct path names observed during workspace scanning
	// Use hash_t bit width (32 on 32-bit, 64 on 64-bit) for index keys
	static constexpr int PATH_HASH_BITS = (int)(sizeof(hash_t) * 8);

	struct StrHashAccessor {
		using Hash = hash_t;
		Hash operator()(const String& s) const { return (Hash)s.GetHashValue(); }
	};

	CritBitIndex<String, StrHashAccessor, PATH_HASH_BITS> seen_path_names;

	
	VfsValue root;
	RWMutex lock;
	
	typedef MetaEnvironment CLASSNAME;
	MetaEnvironment();
	hash_t NewSerial();
	hash_t CurrentSerial() const {return serial_counter;}
	//void Store(const String& includes, const String& path, FileAnnotation& fa);
	
	//static bool IsMergeable(CXCursorKind kind);
	//static bool IsMergeable(int kind);
	
	void AddSeenPath(const String& path);
	void AddSeenPaths(const Vector<String>& paths);
	String GetSeenPath(hash_t str_hash) const;
	bool MergeValue(VfsValue& root, const VfsValue& other, MergeMode mode);
	bool MergeVisit(Vector<VfsValue*>& scope, const VfsValue& n1, MergeMode mode);
	bool MergeVisitPartMatching(Vector<VfsValue*>& scope, const VfsValue& n1, MergeMode mode);
	void RefreshNodePtrs(VfsValue& n);
	void MergeVisitPost(VfsValue& n);
	VfsValue* FindTypeDeclaration(hash_t type_hash);
	bool MergeResolver(ClangTypeResolver& ctr);
	hash_t RealizeTypePath(const String& path);
	bool GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) override;
	VfsItemType CheckItem(const VfsPath& rel_path) override;
};

MetaEnvironment& MetaEnv();




template<> inline void Visitor::VisitVectorSerialize<Array<VfsValue>>(Array<VfsValue>& o) {
	ChkSerializeMagic();
	Ver(1)(1);
	int count = max(0,o.GetCount());
	(*stream) / count;
	if (!storing) {
		o.SetCount(count);
		if (scope) {
			VfsValue* owner = reinterpret_cast<VfsValue*>(scope);
			for (auto& i : o)
				i.owner = owner;
		}
	}
	int dbg_i = 0;
	for (auto& v : o) {
		ChkSerializeMagic();
		v.Visit(*this);
		dbg_i++;
	}
}




template <class T>
inline T* VirtualNode::As() {
	if (!data) return 0;
	Data& d = *data;
	if (d.mode == VFS_VALUE) {
		if (d.root_poly_value) {
			Value val = Get(*d.root_poly_value, d.path);
			if (val.Is<T>())
				return &const_cast<T&>(val.To<T>());
		}
	}
	else if (d.mode == VFS_ENTITY) {
		if (d.vfs_value)
			return d.vfs_value->FindExt<T>();
	}
	return 0;
}



// MCP Env API stubs (to be implemented). These are non-breaking additions.
// Contract (draft):
// - Node identity (EnvNodeInfo.id): stable across sessions for the same symbol/config.
//   Suggested format: hash(namespace + qualified_name + kind + file + primary_range), or an internal USR.
// - Ranges are 1-based line/column, inclusive start, exclusive end.
// - EnvStatus.initialized = true when AST/xrefs for current config are populated.
// - References may be large: EnvReferences must support paging via page_token/limit.
// - Code extraction should prefer AST pretty-print; fallback to file slice by range.
struct EnvNodeInfo : Moveable<EnvNodeInfo> {
    String id;
    String kind;
    String name;
    String file;
    int    start_line = 0;
    int    start_col = 0;
    int    end_line = 0;
    int    end_col = 0;
};

struct EnvRefPage {
    Vector<EnvNodeInfo> items; // or locations if lighter desired
    String next_page_token;
};

struct EnvStatusInfo {
    bool initialized = false;
    int64 last_update_ts = 0; // unix seconds
    int   stale_files = 0;
};

// TODO: implement these in Core2/VfsValue*.cpp and/or adapters in ide/Vfs
EnvNodeInfo EnvLocate(const String& file, int line, int column);
EnvNodeInfo EnvGet(const String& id);
Vector<EnvNodeInfo> EnvDefinition(const String& id);
EnvRefPage EnvReferences(const String& id, const String& page_token = String(), int limit = 200);
String EnvCodeById(const String& id);
String EnvCodeByRange(const String& file, int sline, int scol, int eline, int ecol);
EnvStatusInfo EnvStatus();

#endif
