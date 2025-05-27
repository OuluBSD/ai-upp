#ifndef _Core2_Entity_h_
#define _Core2_Entity_h_

#define COMPONENT_CONSTRUCTOR_(x) CLASSTYPE(x) x(VfsValue& n) : Component(n)
#define COMPONENT_CONSTRUCTOR(x) COMPONENT_CONSTRUCTOR_(x) {}
#define COMPONENT_OVERRIDE_TODO void Visit(Vis& s) override {TODO}
#define METANODE_EXT_CONSTRUCTOR_(x) CLASSTYPE(x) x(VfsValue& n) : VfsValueExt(n)
#define METANODE_EXT_CONSTRUCTOR(x) METANODE_EXT_CONSTRUCTOR_(x) {}

#define COMPONENT_STUB_HEADER(type) \
struct type : Component \
{ \
	COMPONENT_CONSTRUCTOR(type) \
	void Visit(Vis& v) override {v.Ver(1)(1);} \
}; \
INITIALIZE(type)

#define COMPONENT_STUB_IMPL(type) \
	INITIALIZER_COMPONENT(type);

using EntityId				= int64;

class Entity :
	public VfsValueExt,
	public Destroyable,
	public Enableable
{
	
	METANODE_EXT_CONSTRUCTOR(Entity)
	void Clear() {data.Clear();}
	void Visit(Vis& v) override;
	int GetGender() const;
	EntityData* FindData(const VfsPath& path);
	
	bool operator()(const Entity& a, const Entity& b) const {
		return a.data.Get("order", Value()) < b.data.Get("order", Value());
	}
	
	void SetPrefab(String s) {prefab = s;}
	String GetPrefab() const {return prefab;}
	String GetName() const override {return val.id;}
	String ToString() const override {return IntStr64(idx) + " " + prefab + (val.id.GetCount() ? ": " + val.id : String());}
	
	template<typename... ComponentTs>
	RTuple<Ptr<ComponentTs>...> CreateComponents() {
		static_assert(AllComponents<ComponentTs...>::value, "Ts should all be a component");
		
		auto tuple = RTuple<Ptr<ComponentTs>...> {{
				Add0<ComponentTs>(false)
			}
			...
		};
		tuple.ForEach([this](auto& comp) {InitializeComponent(*comp);});
		return tuple;
	}
	
protected:
	friend class EntityInfoCtrl;
	friend class VNodeComponentCtrl;
	friend struct MetaEnvironment;
	friend struct VirtualNode;
	friend struct IdeMetaEnvironment;
	
	VectorMap<String, Value> data;
	ArrayMap<VfsPath, EntityData> objs;
	EntityId idx = -1;
	String prefab;
	
	Value& Data(const String& key) {return data.GetAdd(key);}
};

INITIALIZE(Entity);


#define INITIALIZE_VALUECOMPONENT(x) \
struct x : VfsValueExt { \
	CLASSTYPE(x) \
	x(VfsValue& n) : VfsValueExt(n) {} \
	void Visit(Vis& v) override {} \
}; \
INITIALIZE(x)






template<typename... ComponentTs>
struct EntityPrefab {
	static_assert(AllComponents<ComponentTs...>::value, "All components should derive from Component");
	
	using Components = RTuple<Ptr<ComponentTs>...>;
	
	static String GetComponentNames() {
		return RTuple<Ptr<ComponentTs>...>::GetTypeNames();
	}
	
	static Components Make(Entity& e) {
		return e.CreateComponents<ComponentTs...>();
	}
};




COMPONENT_STUB_HEADER(Context)
COMPONENT_STUB_HEADER(PkgEnv)
COMPONENT_STUB_HEADER(DbRef)
COMPONENT_STUB_HEADER(VirtualIOScript)
COMPONENT_STUB_HEADER(VirtualIOScriptProofread)
COMPONENT_STUB_HEADER(VirtualIOScriptLine)
COMPONENT_STUB_HEADER(VirtualIOScriptSub)


#endif
