#ifndef _Eon_Component_h_
#define _Eon_Component_h_


#if 0
class ComponentBase;
class Entity;
class ComponentStore;


//template <class T> inline T* ComponentBase_Static_As(ComponentBase*) {return 0;}

class ComponentBase :
	public VfsValueExt,
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
	ComponentBase(VfsValue& n);
	virtual ~ComponentBase();
	
	Entity* GetEntity();
	
	String GetDynamicName() const;
	//virtual void Visit(Vis& v) = 0;
	
	virtual bool Arg(String key, Value value) {return true;}
	
	//template <class T> RefT_Entity<T> As() {return ComponentBase_Static_As<T>(this);}
	
	template <class ValDevSpec, class T> bool LinkManually(T& o, String* err_msg=0);
	
	//template <class T> void EtherizeRef(Stream& e, Ref<T>& ref);
	//template <class T> void EtherizeRefContainer(Stream& e, T& cont);
	
	void GetComponentPath(Vector<String>& path);
	
};

using ComponentBasePtr = Ptr<ComponentBase>;


template<typename T>
struct Component :
	public ComponentBase
{
public:
	using ComponentT = Component<T>;
	using ComponentBase::ComponentBase;

	void Visit(Vis& v) override {}
	
	void CopyTo(ComponentBase* target) const override {
		ASSERT(target->GetTypeCls() == GetTypeCls());
	    if (target->GetTypeCls() == GetTypeCls())
	        VisitCopy<ComponentBase>(*this, *target);
	}
	
};

#endif

#define ECS_COMPONENT_CTOR_(x) \
	CLASSTYPE(x) \
	x(VfsValue& e) : Component(e)
#define ECS_COMPONENT_CTOR(x) ECS_COMPONENT_CTOR_(x) {}

#define VISIT_COMPONENT v.VisitT<Component>("Component", *this);



#endif
