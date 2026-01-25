#ifndef _GuboCore_HandleSystemT_h_
#define _GuboCore_HandleSystemT_h_

NAMESPACE_GUBO_BEGIN



#ifdef flagEON
using BaseSystem = System;
#else
class BaseSystem : public Pte<BaseSystem> {
public:
	BaseSystem(Engine&) {}
	virtual ~BaseSystem() {}
	virtual bool Initialize(const WorldState&) { return true; }
    virtual bool Start() { return true; }
    virtual void Update(double dt) {}
    virtual void Stop() {}
    virtual void Uninitialize() {}
};
#endif

template <class Dim>
class HandleSystemT :
	public BaseSystem {
	
public:
	using Base = HandleSystemT<Dim>;
	using Scope = ScopeT<Dim>;
	using Handle = HandleT<Dim>;
	using HandleSystem = HandleSystemT<Dim>;
	using Interaction = typename Dim::Interaction;
	using Container = typename Dim::Container;
	using ContainerFrame = typename Dim::ContainerFrame;
	using TopContainer = typename Dim::TopContainer;
	using Event = typename Dim::Event;
	using EventCollection = typename Dim::EventCollection;
	
	void (*set_mouse_cursor)(void*,const Image&);
	Image (*get_mouse_cursor)(void*);
	void* set_mouse_cursor_arg;
	void* get_mouse_cursor_arg;
	
private:
	Array<Scope> scopes;
	int active_pos = -1;
	bool close_machine_when_empty = false;
	double time = 0;
	
	Array<TopContainer> owned_wins;
	Mutex lock;
	
protected:
    bool Initialize(const WorldState&) override;
    bool Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
public:
    typedef HandleSystemT<Dim> CLASSNAME;
	HandleSystemT(Engine& m);
	
#ifdef flagEON
	SYS_DEF_VISIT
#endif
	
	
	Scope& AddScope();
	void Close();
	void CloseContainer(TopContainer* tc);
	void SetCloseMachineWhenEmpty(bool b=true) {close_machine_when_empty = b;}
	void Set_SetMouseCursor(void (*fn)(void*,const Image&), void* arg);
	void Set_GetMouseCursor(Image (*fn)(void*), void* arg);
	
	void RealizeScope();
	Scope& GetDesktop() {return GetActiveScope();}
	Scope& GetActiveScope();
	Scope& GetScope(int i);
	void DoEvents(const EventCollection& ev);
	
	int GetScopeCount() const {return scopes.GetCount();}
	int GetScreenCount() const {return GetScopeCount();}
	double GetTime() const {return time;}
	
	Image OverrideCursor(const Image& img);
	Image DefaultCursor();
	
	
};

using GuboSystem = HandleSystemT<Ctx3D>;
using SurfaceSystem = HandleSystemT<Ctx2D>;
using GuboSystemPtr = Ptr<GuboSystem>;
using SurfaceSystemPtr = Ptr<SurfaceSystem>;

NAMESPACE_GUBO_END

#endif
