#ifndef _Eon_Component_h_
#define _Eon_Component_h_

namespace Ecs {

class ComponentBase;
class WorldState;
class Entity;
class ComponentStore;


//template <class T> inline T* ComponentBase_Static_As(ComponentBase*) {return 0;}

class ComponentBase :
	public MetaNodeExt,
	public Destroyable,
	public Enableable
{
protected:
	friend class Entity;
	
public:
	virtual void CopyTo(ComponentBase* component) const = 0;
	virtual void Visit(Vis& vis) = 0; // linking errors here means invalid derived visit
	virtual TypeCls GetTypeCls() const = 0;
	virtual void Initialize() {};
	virtual void Uninitialize() {};
	virtual void Update(double dt) {Panic("unimplemented");}
	virtual String ToString() const;
	
	
	Engine& GetEngine();
	
	void AddToUpdateList();
	void RemoveFromUpdateList();
	
public:
	ComponentBase(MetaNode& n);
	virtual ~ComponentBase();
	
	Entity* GetEntity();
	
	String GetDynamicName() const;
	virtual void Serialize(Stream& e) = 0;
	
	virtual bool Arg(String key, Value value) {return true;}
	
	//template <class T> RefT_Entity<T> As() {return ComponentBase_Static_As<T>(this);}
	
	template <class S, class R> void AddToSystem(R ref);
	template <class S, class R> void RemoveFromSystem(R ref);
	
	template <class ValDevSpec, class T> bool LinkManually(T& o, String* err_msg=0);
	
	//template <class T> void EtherizeRef(Ether& e, Ref<T>& ref);
	//template <class T> void EtherizeRefContainer(Ether& e, T& cont);
	
	void GetComponentPath(Vector<String>& path);
	
};

using ComponentBasePtr = Ptr<ComponentBase>;


template<typename T>
struct Component :
	public ComponentBase
{
public:
	using ComponentT = Component<T>;

	void Visit(Vis& v) override {} // don't visit ComponentBase (for error detection reasons)
	
	void CopyTo(ComponentBase* target) const override {
		ASSERT(target->GetTypeCls() == GetTypeCls());
	    
		*static_cast<T*>(target) = *static_cast<const T*>(this);
	}
	
};


#if 0
using ComponentMapBase	= ArrayMap<TypeCls,ComponentBase>;
//using ComponentRefMap	= ArrayMap<TypeCls,Ref<ComponentBase>>;

class ComponentMap : public ComponentMapBase {
	
	void ReturnComponent(ComponentStore& s, ComponentBase* c);
	
public:
	
	ComponentMap() {}
	
	#define IS_EMPTY_SHAREDPTR(x) (x.IsEmpty())
	
	void Dump();
	
	template<typename ComponentT>
	Ptr<ComponentT> Get() {
		CXX2A_STATIC_ASSERT(IsComponent<ComponentT>::value, "T should derive from Component");
		
		int i = ComponentMapBase::Find(AsTypeCls<ComponentT>());
		ASSERT(i >= 0);
		if (i < 0)
			throw (Exc("Could not find component " + AsTypeName<ComponentT>()));
		
		return &ComponentMapBase::operator[](i);
	}
	
	template<typename ComponentT>
	Ptr<ComponentT> Find() {
		CXX2A_STATIC_ASSERT(IsComponent<ComponentT>::value, "T should derive from Component");
		
		int i = ComponentMapBase::Find(AsTypeCls<ComponentT>());
		if (i < 0)
			return Null;
		else
			return &ComponentMapBase::operator[](i);
	}
	
	template<typename ComponentT>
	void Add(ComponentT* component) {
		CXX2A_STATIC_ASSERT(IsComponent<ComponentT>::value, "T should derive from Component");
		
		TypeCls type = ComponentT::TypeIdClass();
		ASSERT(type);
		int i = ComponentMapBase::Find(type);
		ASSERT_(i < 0, "Cannot have duplicate componnets");
		ComponentMapBase::Add(type, component);
	}
	
	template<typename ComponentT>
	void Remove(ComponentStore* s) {
		CXX2A_STATIC_ASSERT(IsComponent<ComponentT>::value, "T should derive from Component");
		
		int i = ComponentMapBase::Find(AsTypeCls<ComponentT>());
		ASSERT_(i < 0, "Tried to remove non-existent component");
		
		auto& comp = ComponentMapBase::operator[](i);
		comp.Uninitialize();
		comp.Destroy();
		
		ReturnComponent(*s, Detach(i));
	}
	
	void AddBase(ComponentBase* component) {
		TypeCls type = component->GetTypeCls();
		int i = ComponentMapBase::Find(type);
		ASSERT(i < 0);
		ComponentMapBase::Add(type, component);
	}
	
	#undef IS_EMPTY_SHAREDPTR
	
};
#endif

}

#endif
