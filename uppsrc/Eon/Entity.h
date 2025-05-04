#ifndef _Eon_Entity_h_
#define _Eon_Entity_h_


namespace Ecs {

class Pool;

class Entity :
	public MetaNodeExt,
	public Destroyable,
	public Enableable
{
	EntityId id = -1;
	int64 created = 0;
	int64 changed = 0;

	String prefab;
	String name;
	
	//ComponentMap comps;
	EntityId m_id;
	
protected:
	friend class Pool;
	
	void SetId(EntityId i) {id = i;}
	void SetCreated(int64 i) {created = i;}
	void SetChanged(int64 i) {changed = i;}
	
	void CopyHeader(const Entity& e) {
		prefab = e.prefab;
		name = e.name;
	}
	
public:
	typedef Entity CLASSNAME;
	Entity(MetaNode& n);
	virtual ~Entity();
	
	static EntityId GetNextId();
	
	void Serialize(Stream& e);
	
	void SetPrefab(String s) {prefab = s;}
	String GetPrefab() const {return prefab;}
	String GetName() const override {return name;}
	EntityId GetId() const {return id;}
	int64 GetCreatedTick() const {return created;}
	int64 GetChangedTick() const {return changed;}
	
	String ToString() const {return IntStr64(id) + " " + prefab + (name.GetCount() ? ": " + name : String());}
	String GetTreeString(int indent=0);
	void SetName(String s) {name = s;}
	void OnChange();
	void UnrefDeep();
	ComponentBasePtr GetTypeCls(TypeCls comp_type);
	ComponentBasePtr GetAddTypeCls(TypeCls cls);
	ComponentBasePtr FindTypeCls(TypeCls comp_type);
	
	template<typename T>
	T& Get() {
		auto comps = node.FindAll<T>();
		ASSERT(comps.GetCount());
		if (comps.IsEmpty()) throw Exc("Can't find component " + AsTypeName<T>());
		return comps[0];
	}
	
	template<typename T>
	T* Find() {
		auto comps = node.FindAll<T>();
		return comps.IsEmpty() ? 0 : comps[0];
	}
	
	template<typename T>
	T* FindNearestEntityWith();
	
	template<typename T>
	Pool* FindNearestPoolWith();
	
	template<typename T>
	T* FindCast() {
		auto comps = node.FindAll<T>();
		T* o = 0;
		for(auto& comp : comps) {
			o = CastPtr<T>(&*comp);
			if (o)
				break;
		}
		return o;
	}
	
	template<typename T>
	Entity* FindInterface() {
		auto comps = node.FindAll<T>();
		Entity* o = 0;
		for(auto& comp : comps)
			if ((o = CastPtr<T>(&*comp)))
				break;
		return o;
	}
	
	template<typename T>
	Vector<Ptr<T>> FindInterfaces() {
		auto comps = node.FindAll<T>();
		Vector<Ptr<T>> v;
		Entity* o = 0;
		for(auto& comp : comps)
			if ((o = CastPtr<T>(&*comp)))
				v.Add(o);
		return v;
	}
	
	template<typename T> T* FindConnector();
	template<typename T> T* FindCommonConnector(Entity* sink);
	int GetPoolDepth() const;
	bool HasPoolParent(Pool* pool) const;
	
	template<typename T> void Remove() {
		OnChange();
		Remove0<T>();
	}
	template<typename T> T& Add() {
		OnChange();
		auto comp = Add0<T>(true);
		return comp;
	}
	template<typename T> T& GetAdd() {
		T* o = Find<T>();
		if (o)
			return o;
		OnChange();
		auto comp = Add0<T>(true);
		return comp;
	}
	
	ComponentBase&	GetAdd(String comp_name);
	
	template<typename... ComponentTs>
	Tuple<ComponentTs*...> TryGetComponents() {
		return MakeTuple(node.Find<ComponentTs>()...);
	}
	
	
	Entity*				Clone() const;
	void				InitializeComponents();
	void				InitializeComponent(ComponentBase& comp);
	void				InitializeComponentPtr(ComponentBasePtr comp) {return InitializeComponent(*comp);}
	void				UninitializeComponents();
	void				ClearComponents();
	
	EntityId			Id() const {return m_id;}
	
	void				Destroy() override;
	
	Engine&				GetEngine();
	const Engine&		GetEngine() const;
	Pool&				GetPool() const;
	Pool&				GetRoot();
	void				GetEntityPath(Vector<String>& path);
	
	//ComponentMap&		GetComponents() {return comps;}
	//const ComponentMap&	GetComponents() const {return comps;}
	
	template<typename... ComponentTs>
	Tuple<ComponentTs*...> CreateComponents() {
		static_assert(AllComponents<ComponentTs...>::value, "Ts should all be a component");
		
		auto tuple =  Tuple<ComponentTs*...> {{
				Add0<ComponentTs>(false)
			}
			...
		};
		tuple.ForEach([this](auto& comp) {InitializeComponent(comp.GetMutable());});
		return tuple;
	}
	
	ComponentBasePtr CreateEon(String id);
	ComponentBasePtr CreateComponent(TypeCls type);
	
	
	//void Visit(Vis& vis) {vis || comps;}
	
private:
	
	template<typename T> void Remove0();
	template<typename T> T* Add0(bool initialize);
	
	
	ComponentBasePtr AddPtr(ComponentBase* comp);
	
	
};

using EntityPtr = Ptr<Entity>;
//using EntityVec = Array<Entity>;






template<typename... ComponentTs>
struct EntityPrefab {
	static_assert(AllComponents<ComponentTs...>::value, "All components should derive from Component");
	
	using Components = Tuple<ComponentTs*...>;
	
	static String GetComponentNames() {
		return Tuple<ComponentTs*...>::AsTypeNames();
	}
	
    static String AsTypeName() {
        static auto s = "EntityPrefab<" + GetComponentNames() + ">" ; return s;
    }
    
	static Components Make(Entity& e) {
		return e.CreateComponents<ComponentTs...>();
	}
};


class EntityHashVisitor : public Vis {
	CombineHash ch;
	
	//bool OnEntry(const RTTI& type, TypeCls derived, const char* derived_name, void* mem, LockedScopeRefCounter* ref) override;
	
public:
	
	operator hash_t() const {return ch;}
	
};


}


#endif
