#include "Eon.h"


NAMESPACE_UPP

bool EntitySystem::Initialize() {
	engine = new Ecs::Engine;
	engine->sys = this;
	SetActiveEngine(*engine);
	return true;
}

void EntitySystem::Start() {
	engine->Start();
}

void EntitySystem::Update(double dt) {
	if (engine->IsRunning())
		engine->Update(dt);
}

void EntitySystem::Stop() {
	engine->Stop();
}

void EntitySystem::Uninitialize() {
	ClearEngine();
}

void EntitySystem::ClearEngine() {
	if (engine) {
		engine->Clear();
		delete engine;
		engine = 0;
	}
}

void EntitySystem::Visit(Vis& vis) {
	vis.VisitT<System<CLASSNAME>>("System",*this);
	if (engine)
		vis("engine",*engine);
}

END_UPP_NAMESPACE



NAMESPACE_UPP namespace Ecs {


SystemBase::SystemBase(MetaNode& n) : MetaSystemBase(n) {
	DBG_CONSTRUCT
}

SystemBase::~SystemBase() {
	DBG_DESTRUCT
}

Engine& SystemBase::GetEngine() const {
	Engine* e = node.GetOwnerExt<Engine>();
	ASSERT(e);
	if (!e) throw Exc("no engine");
	return *e;
}


Callback Engine::WhenGuiProgram;
Callback Engine::WhenInitialize;
Callback Engine::WhenPreFirstUpdate;




Engine::Engine(MetaNode& n) : MetaNodeExt(n) {
	DBG_CONSTRUCT
}

Engine::~Engine() {
	ASSERT_(!is_initialized && !is_started, "Engine should be in a clean state upon destruction");
	systems.Clear();
	DBG_DESTRUCT
}


bool Engine::Start() {
	ASSERT_(!is_initialized && !is_started, "Shouldn't call Start if we already started");
	
	WhenInitialize();
	
	is_looping_systems = true;
	
	for (auto it : ~systems) {
		if (!it.value.Initialize()) {
			LOG("Could not initialize system " << it.key.GetName());
			return false;
		}
	}
	
	is_initialized = true;
	
	for (auto it : ~systems) {
		it.value.Start();
	}
	
	is_looping_systems = false;
	
	is_started = true;
	is_running = true;
	return true;
}

void Engine::Update(double dt) {
	ASSERT_(is_started, "Shouldn't call Update if we haven't been started");
	
	if (!ticks)
		WhenPreFirstUpdate();
	
	WhenEnterUpdate();
	
	if (dt <= 0.0) {
		WhenLeaveUpdate();
		return;
	}
	if (dt > 0.5)
		dt = 0.5;
		
	if (is_suspended) {
		WhenLeaveUpdate();
		return;
	}
	
	for (ComponentBaseUpdater* cb : update_list) {
		cb->Update(dt);
	}
	
	is_looping_systems = true;
	
	for (auto it : ~systems) {
		SystemBase* b = &it.value;
		WhenEnterSystemUpdate(*b);
		
		b->Update(dt);
		
		WhenLeaveSystemUpdate();
	}
	
	is_looping_systems = false;
	
	++ticks;
	
	WhenLeaveUpdate();
}

void Engine::Stop() {
	if (!is_started)
		return;
	
	is_running = false;
	is_started = false;
	
	DBG_BEGIN_UNREF_CHECK
	
	is_looping_systems = true;
	
	for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
		it->Stop();
	}
	
	is_initialized = false;
	
	for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
		it->Uninitialize();
	}
	
	is_looping_systems = false;
}

void Engine::Suspend() {
	is_suspended = true;
}

void Engine::Resume() {
	is_suspended = false;
}

bool Engine::HasStarted() const {
	return is_started;
}

void Engine::SystemStartup(TypeCls type_id, SystemBase* system) {
	ASSERT(is_started);
	if (system->Initialize()) {
		RTLOG("Engine::SystemStartup: added system to already running engine: " << system->GetType().GetName());
		
		bool has_already = false;
		for (auto r : ~systems)
			if (&r.value == system)
				has_already = true;
		if (!has_already)
			systems.Add(type_id, system);
		
		system->Start();
	}
	else {
		RTLOG("Engine::SystemStartup: error: could not initialize system in already running engine: " << system->GetType().GetName());
		delete system;
	}
}

void Engine::Add(TypeCls type_id, SystemBase* system, bool startup) {
	ASSERT_(!is_looping_systems, "adding systems while systems are being iterated is error-prone");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	ASSERT(system->GetParent());
	if (startup && is_started)
		SystemStartup(type_id, system);
	else
		systems.Add(type_id, system);
}

void Engine::Remove(TypeCls type_id) {
	ASSERT_(!is_started, "removing systems after the machine has started is error-prone");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	systems.Remove(i);
}

void Engine::Visit(Vis& vis) {
	for (auto iter = systems.begin(); iter; ++iter)
		vis.Visit(iter());
}

void Engine::AddToUpdateList(ComponentBaseUpdater* c) {
	VectorFindAdd(update_list, c);
}

void Engine::RemoveFromUpdateList(ComponentBaseUpdater* c) {
	VectorRemoveKey(update_list, c);
}

Ptr<SystemBase> Engine::Add(TypeCls type, bool startup)
{
    NewSystemFn fn = TypeNewFn().Get(type, 0);
    ASSERT(fn);
    if (!fn)
        return Ptr<SystemBase>();
	SystemBase* syst = fn(*this);
    Add(type, syst, startup);
    return syst;
}

Ptr<SystemBase> Engine::GetAdd(String id, bool startup) {
    int i = EonToType().Find(id);
    if (i < 0)
        return Ptr<SystemBase>();
    TypeCls type = EonToType()[i];
    i = systems.Find(type);
    if (i >= 0)
        return &systems[i];
    return Add(type, startup);
}



Engine* __active_engine;

Engine& GetActiveEngine() {
	ASSERT(__active_engine);
	return *__active_engine;
}

void SetActiveEngine(Engine& e) {
	__active_engine = &e;
}

void ClearActiveEngine() {
	__active_engine = 0;
}


} END_UPP_NAMESPACE

