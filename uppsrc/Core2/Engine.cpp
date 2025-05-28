#include "Core.h"


NAMESPACE_UPP






System::System(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

System::~System() {
	DBG_DESTRUCT
}

Engine& System::GetEngine() const {
	Engine* e = val.GetOwnerExt<Engine>();
	ASSERT(e);
	if (!e) throw Exc("no engine");
	return *e;
}





Engine::Engine(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

Engine::~Engine() {
	ASSERT_(!is_initialized && !is_started, "Engine should be in a clean state upon destruction");
	DBG_DESTRUCT
}


bool Engine::Start() {
	ASSERT_(!is_initialized && !is_started, "Shouldn't call Start if we already started");
	
	WhenInitialize(*this);
	
	is_looping_systems = true;
	
	auto systems = val.FindAll<System>();
	for (auto it : systems) {
		if (!it->Initialize()) {
			LOG("Could not initialize system " << it->GetTypeCls().GetName());
			return false;
		}
	}
	
	is_initialized = true;
	
	for (auto it : systems) {
		it->Start();
	}
	
	is_looping_systems = false;
	
	is_started = true;
	is_running = true;
	return true;
}

void Engine::Update(double dt) {
	ASSERT_(is_started, "Shouldn't call Update if we haven't been started");
	
	if (!ticks)
		WhenPreFirstUpdate(*this);
	
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
	
	for (Component* cb : update_list) {
		cb->Update(dt);
	}
	
	is_looping_systems = true;
	
	auto systems = val.FindAll<System>();
	for (System* b : systems) {
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
	
	auto systems = val.FindAll<System>();
	for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
		(*it)->Stop();
	}
	
	is_initialized = false;
	
	for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
		(*it)->Uninitialize();
	}
	
	is_looping_systems = false;
}

void Engine::Suspend() {
	is_suspended = true;
}

void Engine::Resume() {
	is_suspended = false;
}

bool Engine::IsStarted() const {
	return is_started;
}

void Engine::SystemStartup(TypeCls type_id, System* system) {
	ASSERT(is_started);
	if (system->Initialize()) {
		RTLOG("Engine::SystemStartup: added system to already running engine: " << system->GetTypeCls().GetName());
		
		bool has_already = false;
		auto systems = val.FindAll<System>();
		for (auto r : systems)
			if (&*r == system)
				has_already = true;
		if (!has_already) {
			Panic("error"); // previously system was added to systems list here, but now it has owner already
		}
		system->Start();
	}
	else {
		RTLOG("Engine::SystemStartup: error: could not initialize system in already running engine: " << system->GetTypeCls().GetName());
		delete system;
	}
}

#if 0
void Engine::Add(TypeCls type_id, System* system, bool startup) {
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
#endif

void Engine::Visit(Vis& vis) {
	vis && update_list;
}

void Engine::AddToUpdateList(ComponentPtr c) {
	VectorFindAdd(update_list, c);
}

void Engine::RemoveFromUpdateList(ComponentPtr c) {
	VectorRemoveKey(update_list, c);
}

Val& Engine::GetRootPool() {
	int i = val.Find("pool");
	if (i >= 0)
		return val.sub[i];
	return val.Add("pool");
}

Val& Engine::GetRootSpace() {
	int i = val.Find("space");
	if (i >= 0)
		return val.sub[i];
	return val.Add("space");
}

Ptr<System> Engine::Add(TypeCls type, bool startup)
{
	int i = VfsValueExtFactory::FindTypeClsFactory(type);
	ASSERT(i >= 0);
	if (i < 0)
		return 0;
	hash_t type_hash = VfsValueExtFactory::List()[i].type_hash;
	VfsValue& new_val = val.Add(type.GetName(), type_hash);
	System* syst = new_val.FindExt<System>();
	ASSERT(syst);
	
	if (syst && startup && is_started)
		SystemStartup(type, syst);
	
    return syst;
}

Ptr<System> Engine::GetAdd(String id, bool startup) {
    int i = VfsValueExtFactory::EonToType().Find(id);
    if (i < 0)
        return Ptr<System>();
    TypeCls type = VfsValueExtFactory::EonToType()[i];
    auto v = val.FindAll(type);
    if (v.GetCount())
        return v[0]->ext ? CastPtr<System>(&*v[0]->ext) : 0;
    return Add(type, startup);
}





END_UPP_NAMESPACE

