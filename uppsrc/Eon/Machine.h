#ifndef _Eon_Machine_h_
#define _Eon_Machine_h_

namespace Ecs {
class Engine;
}

class MachineVerifier;
class Space;
class Loop;


class SystemBase : public MetaSystemBase {
public:
    SystemBase(VfsValue&);
    virtual ~SystemBase();

    virtual TypeCls GetTypeCls() const = 0;
	virtual void Visit(Vis& vis) {}
	Machine& GetMachine() const;
	
protected:
    friend Machine;

    virtual bool Initialize() {return true;}
    virtual void Start() {}
    virtual void Update(double /*dt*/) {}
    virtual void Stop() {}
    virtual void Uninitialize() {}

	
};

template<typename T>
using IsSystem = std::is_base_of<SystemBase, T>;


template<typename T>
class System : public SystemBase
{
	using SystemT = System<T>;
public:
	System(VfsValue& n) : SystemBase(n) {}
	TypeCls GetTypeCls() const override {return AsTypeCls<T>();}
    void Visit(Vis& vis) override {vis.VisitT<SystemBase>("SystemBase",*this);}
    
};


#define SYS_CTOR(x) \
	CLASSTYPE(x) \
	x(VfsValue& m) : System<x>(m) {}
#define SYS_CTOR_(x) \
	CLASSTYPE(x) \
	x(VfsValue& m) : System<x>(m)
#define SYS_DEF_VISIT void Visit(Vis& vis) override {vis.VisitT<System<CLASSNAME>>("Base",*this);}
#define SYS_DEF_VISIT_(x) void Visit(Vis& vis) override {x; vis.VisitT<System<CLASSNAME>>("Base",*this);}
#define SYS_DEF_VISIT_H void Visit(Vis& vis) override;
#define SYS_DEF_VISIT_I(cls, x) void cls::Visit(Vis& vis) {x; vis.VisitT<System<CLASSNAME>>("Base",*this);}

class Machine :
	public MetaMachineBase
{
	int64 ticks = 0;
	Index<String> last_warnings;
	double warning_age;
	
public:
	int64 GetTicks() const {return ticks;}
	
	
    /*template<typename SystemT>
    Ptr<SystemT> Get() {
        auto system = Find<SystemT>();
        ASSERT_(system, "System " << AsTypeName<SystemT>() << " was not found in the Machine");
        return system;
    }*/

    /*template<typename SystemT>
    SystemT* GetPtr() {
        SystemCollection::PtrIterator it = FindSystemPtr(AsTypeCls<SystemT>());
        return CastPtr<SystemT>(&*it);
    }*/

    template<typename SystemT>
    Ptr<SystemT> Find()
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
        auto v = this->val.template FindAll<SystemT>();
        return v.IsEmpty() ? 0 : v[0];
    }
	
    template<typename SystemT, typename... Args>
    Ptr<SystemT> Add(Args&&... args)
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");
		
		VfsValue& n = val.Add();
		SystemT* syst = new SystemT(n, args...);
        n.ext = syst;
        n.type_hash = syst->GetTypeHash();
        ASSERT(n.type_hash);
        return syst;
    }
    
    template<typename SystemT, typename... Args>
    Ptr<SystemT> FindAdd(Args&&... args)
    {
		Ptr<SystemT> ret = Find<SystemT>();
		if (ret)
			return ret;
		
		return Add<SystemT>(args...);
    }
    
    /*
    template<typename SystemT>
    void Remove()
    {
        CXX2A_STATIC_ASSERT(IsSystem<SystemT>::value, "T should derive from System");

        ASSERT(is_initialized && is_started);
        Remove(AsTypeCls<SystemT>());
    }*/
    
    

	CLASSTYPE(Machine)
    Machine(VfsValue& n);
    virtual ~Machine();

    bool HasStarted() const;
    bool HasFailed() const {return is_failed;}

	bool CommandLineInitializer(bool skip_eon_file);
    bool Start();
    void Update(double dt);
    void Stop();
    void Suspend();
    void Resume();
    void DieFast();
	void Clear();
	
    bool IsStarted() const {return is_started;}
    bool IsRunning() const {return is_running;}
	void SetNotRunning() {is_running = false;}
	void SetFailed(String msg="") {is_failed = true; fail_msg = msg;}
	void Visit(Vis& vis) override;
	void WarnDeveloper(String msg);
	void Run(bool main_loop, String app_name, String override_eon_file="", VectorMap<String,Value>* extra_args=0, const char* extra_str=0);
	void Main(bool main_loop, String script_content, String script_file, VectorMap<String,Value>& args, bool dbg_ref_visits=false, uint64 dbg_ref=0);
	void Main(String script_content, String script_file, VectorMap<String,Value>& args, MachineVerifier* ver, bool dbg_ref_visits=false, uint64 dbg_ref=0);
	void StopRunner();
	void MainLoop(bool (*fn)(void*)=0, void* arg=0);
	
	MachineVerifier*	GetMachineVerifier() const {return mver;}
	Ecs::Engine&		GetEngine();
	Loop&				GetRootLoop();
	Space&				GetRootSpace();
	
	using ObjMap = VectorMap<String,Value>;
	
	Event<> WhenEnterUpdate;
	Event<SystemBase&> WhenEnterSystemUpdate;
	
	Event<> WhenLeaveUpdate;
	Event<> WhenLeaveSystemUpdate;
	
	Event<Machine&> WhenBoot;
	Event<Machine&> WhenInitialize;
	Event<Machine&> WhenUserProgram;
	Event<Machine&> WhenPostInitialize;
	Event<Machine&> WhenPreFirstUpdate;
	
private:
    bool is_started = false;
    bool is_initialized = false;
    bool is_suspended = false;
    bool is_running = false;
    bool is_failed = false;
    String fail_msg;
	ObjMap eon_params;
	String eon_script;
	String eon_file;
	uint64 break_addr = 0;
    
    SystemBase* FindSystem(TypeCls type_id);
    //SystemCollection::Iterator FindSystem(TypeCls type_id) {return systems.Find(type_id);}
    //SystemCollection::PtrIterator FindSystemPtr(TypeCls type_id) {return systems.FindPtr(type_id);}
    void Add(TypeCls type_id, SystemBase* system);
    void Remove(TypeCls typeId);
    
    
protected:
	friend class MachineVerifier;
	
    MachineVerifier* mver = 0;
    
    
};



#if 0
class SingleMachine {
	
protected:
	Machine mach;
	
	bool Open(void(*arg_fn)());
	void Close();
	
public:
	SingleMachine()		{SetActiveMachine(mach);}
	~SingleMachine()	{ClearActiveMachine();}
	
	void Run(void(*fn)(), void(*arg_fn)());
	
	bool Start() {return Open(0);}
	void Stop() {Close();}
};
#endif

#endif
