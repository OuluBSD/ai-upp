#ifndef _Eon_EcsEngine_h_
#define _Eon_EcsEngine_h_


namespace Ecs {

class Engine;
struct ComponentBaseUpdater;

class SystemBase : public Pte<SystemBase>
{
public:
    SystemBase();
    SystemBase(MetaNode& m);
    virtual ~SystemBase();

    virtual TypeCls GetType() const = 0;
	virtual void Visit(Vis& vis) = 0;
    virtual bool Arg(String key, Value value) {return true;}
	
	Engine& GetEngine() const;
protected:
    friend Engine;

    virtual bool Initialize() {return true;}
    virtual void Start() {}
    virtual void Update(double /*dt*/) {}
    virtual void Stop() {}
    virtual void Uninitialize() {}

	
};


template<typename T>
class System :
	public SystemBase
{
public:
	using SystemT = System<T>;
    using SystemBase::SystemBase;
	
	System(MetaNode& m) : SystemBase(m) {};
    TypeCls GetType() const override {return AsTypeCls<T>();}
    void Visit(Vis& vis) override {vis.VisitT<SystemBase>(this);}
    
};


class Engine : public Pte<Engine>
{
	int64 ticks = 0;
	
public:
	int64 GetTicks() const {return ticks;}
	
	
    template<typename SystemT>
    Ptr<SystemT> Get() {
        auto system = TryGet<SystemT>();
        ASSERT(system);
        return system;
    }

    template<typename SystemT>
    Ptr<SystemT> TryGet()
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
        
        SystemCollection::Iterator it = FindSystem(AsTypeCls<SystemT>());
        if (it)
            return &*it;
        
        return Ptr<SystemT>();
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> Add(Args&&... args)
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
		
		SystemT* syst = new SystemT(*this, args...);
        Add(AsTypeCls<SystemT>(), syst);
        return syst->template AsRef<SystemT>();
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> GetAdd(Args&&... args) {
        SystemCollection::Iterator it = FindSystem(AsTypeCls<SystemT>());
        if (it)
            return &*it;
        return Add<SystemT>(args...);
    }
    
    
    
    template<typename SystemT>
    void Remove()
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");

        ASSERT(is_initialized && is_started);
        Remove(AsTypeCls<SystemT>());
    }

    Engine();
    virtual ~Engine();

    bool HasStarted() const;

    bool Start();
    void Update(double dt);
    void Stop();
    void Suspend();
    void Resume();
    void DieFast() {Start(); Update(0); Stop();}
	void Clear() {ticks=0; is_started=0; is_initialized=0; is_suspended=0; is_running=0; systems.Clear();}
	
    bool IsRunning() const {return is_running;}
	void SetNotRunning() {is_running = false;}
	void Visit(Vis& vis);
	
	void AddToUpdateList(ComponentBaseUpdater* c);
	void RemoveFromUpdateList(ComponentBaseUpdater* c);
	
	Ptr<SystemBase> Add(TypeCls type, bool startup=true);
	Ptr<SystemBase> GetAdd(String id, bool startup=true);
    
	Callback WhenEnterUpdate;
	Callback1<SystemBase&> WhenEnterSystemUpdate;
	
	Callback WhenLeaveUpdate;
	Callback WhenLeaveSystemUpdate;
	
	static Callback WhenGuiProgram;
	static Callback WhenInitialize;
	static Callback WhenPreFirstUpdate;
	
	EntitySystem& GetEntitySystem() {ASSERT(sys); return *sys;}
	Machine& GetMachine() {return sys->GetMachine();}
	
protected:
	friend class EntitySystem;
	
	EntitySystem* sys = 0;
	
private:
    using SystemCollection = ArrayMap<TypeCls, SystemBase>;
    SystemCollection systems;
	
	bool is_started = false;
    bool is_initialized = false;
    bool is_suspended = false;
    bool is_running = false;
    bool is_looping_systems = false;
    
    int FindSystem(TypeCls type_id) {return systems.Find(type_id);}
    void Add(TypeCls type_id, SystemBase* system, bool startup=true);
    void Remove(TypeCls typeId);
    
    Vector<ComponentBaseUpdater*> update_list;
    
    
private:
	typedef SystemBase* (*NewSystemFn)(Ecs::Engine&);
    static VectorMap<String, TypeCls>& EonToType() {static VectorMap<String, TypeCls> m; return m;}
    static VectorMap<TypeCls, NewSystemFn>& TypeNewFn() {static VectorMap<TypeCls, NewSystemFn> m; return m;}
	
	template <class T>
	static SystemBase* NewSystem(Ecs::Engine& e) {return new T(e);}
	
public:
	
	template <class T>
	static void Register(String id) {
		//String id = T::GetEonId();
		ASSERT(id.GetCount() > 0);
		TypeCls type = T::TypeIdClass();
		ASSERT(EonToType().Find(id) < 0);
		EonToType().Add(id, type);
		ASSERT(TypeNewFn().Find(type) < 0);
		TypeNewFn().Add(type, &NewSystem<T>);
	}
	
    void SystemStartup(TypeCls type_id, SystemBase* system);
    
};


Engine& GetActiveEngine();
void SetActiveEngine(Engine& e);
void ClearActiveEngine();

template <class S, class R>
inline void ComponentBase::AddToSystem(R ref) {
	S* sys = GetEngine().Get<S>();
	if (sys)
		sys->Add(ref);
}

template <class S, class R>
inline void ComponentBase::RemoveFromSystem(R ref) {
	S* sys = GetEngine().Get<S>();
	if (sys)
		sys->Remove(ref);
}


template<typename T>
void Entity::Remove0() {
	comps.Remove<T>(GetEngine().Get<ComponentStore>());
}

template<typename T>
T* Entity::Add0(bool initialize) {
	T* comp = GetEngine().Get<ComponentStore>()->CreateComponent<T>();
	ASSERT(comp);
	comps.Add(comp);
	if (initialize) {
		InitializeComponent(*comp);
	}
	ASSERT(comp->GetEntity());
	return comp;
}


}




void MachineEcsInit();




#endif
