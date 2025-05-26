#ifndef _Eon_EcsEngine_h_
#define _Eon_EcsEngine_h_


namespace Ecs {

class Engine;
class ComponentBase;

// TODO remove Ecs duplicate classes like SystemBase
class SystemBase :
	public MetaSystemBase,
	public Destroyable,
	public Enableable
{
public:
    SystemBase();
    SystemBase(VfsValue& m);
    virtual ~SystemBase();

    virtual TypeCls GetTypeCls() const = 0;
	virtual void Visit(Vis& vis) {}
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
	
	System(VfsValue& m) : SystemBase(m) {};
    TypeCls GetTypeCls() const override {return AsTypeCls<T>();}
    void Visit(Vis& vis) override {vis.VisitT<SystemBase>("SystemBase",*this);}
    
};


class Engine : public VfsValueExt
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
        CXX2A_STATIC_ASSERT(Ecs::IsSystem<SystemT>::value, "T should derive from System");
        return node.Find<SystemT>();
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> Add(Args&&... args)
    {
        CXX2A_STATIC_ASSERT(Ecs::IsSystem<SystemT>::value, "T should derive from System");
        VfsValue& sub = val.Add();
        SystemT* syst = new SystemT(sub, args...);
		sub.ext = syst;
		sub.type_hash = syst->GetTypeHash();
        return syst;
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> GetAdd(Args&&... args) {
        auto sys = node.Find<SystemT>();
        if (sys)
            return sys;
        return Add<SystemT>(args...);
    }
    
    
    
    template<typename SystemT>
    void Remove()
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");

        ASSERT(is_initialized && is_started);
        for(int i = 0; i < val.sub.GetCount(); i++) {
			auto& n = val.sub[i];
			if (n.ext) {
				SystemT* t = CastPtr<SystemT>(&*n.ext);
				if (t) {
					val.sub.Remove(i--);
				}
			}
        }
    }

	CLASSTYPE(Engine)
    Engine(VfsValue& n);
    virtual ~Engine();

    bool HasStarted() const;

    bool Start();
    void Update(double dt);
    void Stop();
    void Suspend();
    void Resume();
    void DieFast() {Start(); Update(0); Stop();}
	void Clear() {ticks=0; is_started=0; is_initialized=0; is_suspended=0; is_running=0; /*systems.Clear();*/}
	
    bool IsRunning() const {return is_running;}
	void SetNotRunning() {is_running = false;}
	void Visit(Vis& vis) override;
	
	void AddToUpdateList(ComponentBasePtr c);
	void RemoveFromUpdateList(ComponentBasePtr c);
	
	Ptr<SystemBase> Add(TypeCls type, bool startup=true);
	Ptr<SystemBase> GetAdd(String id, bool startup=true);
    Pool& GetRootPool();
    
	Event<> WhenEnterUpdate;
	Event<SystemBase&> WhenEnterSystemUpdate;
	
	Event<> WhenLeaveUpdate;
	Event<> WhenLeaveSystemUpdate;
	
	static Event<Engine&> WhenGuiProgram;
	static Event<Engine&> WhenInitialize;
	static Event<Engine&> WhenPreFirstUpdate;
	
	//EntitySystem& GetEntitySystem() {ASSERT(sys); return *sys;}
	Machine& GetMachine();
	
	
private:
    //using SystemCollection = ArrayMap<TypeCls, SystemBase>;
    //SystemCollection systems;
	
	bool is_started = false;
    bool is_initialized = false;
    bool is_suspended = false;
    bool is_running = false;
    bool is_looping_systems = false;
    
    int FindSystem(TypeCls type_id);// {return systems.Find(type_id);}
    void Add(TypeCls type_id, SystemBase* system, bool startup=true);
    void Remove(TypeCls typeId);
    
    Vector<ComponentBasePtr> update_list;
    
    
private:
	typedef SystemBase* (*NewSystemFn)(VfsValue&);
    static VectorMap<String, TypeCls>& EonToType() {static VectorMap<String, TypeCls> m; return m;}
    static VectorMap<TypeCls, NewSystemFn>& TypeNewFn() {static VectorMap<TypeCls, NewSystemFn> m; return m;}
	
	template <class T>
	static SystemBase* NewSystem(VfsValue& n) {
		T* o = new T(n);
		n.ext = o;
		n.type_hash = AsTypeHash<T>();
		return o;
	}
	
public:
	
	template <class T>
	static void Register(String id) {
		//String id = T::GetEonId();
		ASSERT(id.GetCount() > 0);
		TypedStringHasher<T>(id);
		TypeCls type = AsTypeCls<T>();
		ASSERT(EonToType().Find(id) < 0);
		EonToType().Add(id, type);
		ASSERT(TypeNewFn().Find(type) < 0);
		TypeNewFn().Add(type, &NewSystem<T>);
	}
	
    void SystemStartup(TypeCls type_id, SystemBase* system);
    
};


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
	for(int i = 0; i < val.sub.GetCount(); i++) {
		auto& n = val.sub[i];
		if (n.ext) {
			T* t = CastPtr<T>(&*n.ext);
			if (t) {
				val.sub.Remove(i--);
			}
		}
    }
}

template<typename T>
Ptr<T> Entity::Add0(bool initialize) {
	VfsValue& sub = val.Add();
    T* comp = new T(sub);
	sub.ext = comp;
	sub.type_hash = comp->GetTypeHash();
    if (initialize) {
		InitializeComponent(*comp);
	}
	ASSERT(comp->GetEntity());
	return comp;
}


}




void MachineEcsInit(Machine& mach);




#endif
