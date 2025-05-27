#ifndef _Core2_Engine_h_
#define _Core2_Engine_h_


class Engine;
class Component;


class System :
	public VfsValueExt,
	public Destroyable,
	public Enableable
{
public:
    System();
    System(VfsValue& m);
    virtual ~System();

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
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
        return val.Find<SystemT>();
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> Add(Args&&... args)
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
        VfsValue& sub = val.Add();
        SystemT* syst = new SystemT(sub, args...);
		sub.ext = syst;
		sub.type_hash = syst->GetTypeHash();
        return syst;
    }

    template<typename SystemT, typename... Args>
    Ptr<SystemT> GetAdd(Args&&... args) {
        auto sys = val.Find<SystemT>();
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
	
	void AddToUpdateList(ComponentPtr c);
	void RemoveFromUpdateList(ComponentPtr c);
	
	Ptr<System> Add(TypeCls type, bool startup=true);
	Ptr<System> GetAdd(String id, bool startup=true);
    
    Val& GetRootPool();
	Val& GetMachine();
    
    
public:
	Event<> WhenEnterUpdate;
	Event<System&> WhenEnterSystemUpdate;
	
	Event<> WhenLeaveUpdate;
	Event<> WhenLeaveSystemUpdate;
	
	Event<Engine&> WhenGuiProgram;
	Event<Engine&> WhenInitialize;
	Event<Engine&> WhenPreFirstUpdate;
	Event<Engine&> WhenPostInitialize;
	Event<Engine&> WhenBoot;
	Event<Engine&> WhenUserProgram;
	
	//EntitySystem& GetEntitySystem() {ASSERT(sys); return *sys;}
	
	
private:
    //using SystemCollection = ArrayMap<TypeCls, System>;
    //SystemCollection systems;
	
	bool is_started = false;
    bool is_initialized = false;
    bool is_suspended = false;
    bool is_running = false;
    bool is_looping_systems = false;
    
    int FindSystem(TypeCls type_id);// {return systems.Find(type_id);}
    void Add(TypeCls type_id, System* system, bool startup=true);
    void Remove(TypeCls typeId);
    
    Vector<ComponentPtr> update_list;
    
    
public:
	
	// Moved AtomSystem here
    void AddUpdated(AtomBase& p);
    void RemoveUpdated(AtomBase& p);
	
private:
	Vector<AtomBasePtr> updated;
	
    #if 0
private:
	typedef System* (*NewSystemFn)(VfsValue&);
    static VectorMap<String, TypeCls>& EonToType() {static VectorMap<String, TypeCls> m; return m;}
    static VectorMap<TypeCls, NewSystemFn>& TypeNewFn() {static VectorMap<TypeCls, NewSystemFn> m; return m;}
	
	template <class T>
	static System* NewSystem(VfsValue& n) {
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
	#endif
	
	
    void SystemStartup(TypeCls type_id, System* system);
    
};


template <class S>
inline void Component::AddToSystem() {
	S* sys = GetEngine().Get<S>();
	ASSERT(sys);
	if (sys)
		sys->Add(*this);
}

template <class S>
inline void Component::RemoveFromSystem() {
	S* sys = GetEngine().Get<S>();
	ASSERT(sys);
	if (sys)
		sys->Remove(*this);
}

#if 0

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

#endif

void MachineEcsInit(Engine&); // todo rename


template <class S>
inline void AtomBase::AddToSystem() {
	auto eng = val.FindOwner<Engine>();
	ASSERT(eng);
	if (!eng) return;
	Ptr<S> sys = eng->Get<S>();
	ASSERT(sys);
	if (sys)
		sys->Add(*this);
}

template <class S>
inline void AtomBase::RemoveFromSystem() {
	auto eng = val.FindOwner<Engine>();
	ASSERT(eng);
	if (!eng) return;
	Ptr<S> sys = eng->Get<S>();
	ASSERT(sys);
	if (sys)
		sys->Remove(*this);
}


#endif
