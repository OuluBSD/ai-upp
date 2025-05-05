#include "Eon.h"

NAMESPACE_UPP


SystemBase::SystemBase(MetaNode& n) : MetaSystemBase(n) {
	DBG_CONSTRUCT
}

SystemBase::~SystemBase() {
	DBG_DESTRUCT
}

Machine& SystemBase::GetMachine() const {
	TODO; throw Exc("no machine"); //return *GetParent();
}


Callback Machine::WhenUserProgram;
Callback Machine::WhenInitialize;
Callback Machine::WhenPostInitialize;
Callback Machine::WhenPreFirstUpdate;




Machine::Machine(MetaNode& n) : MetaMachineBase(n) {
	DBG_CONSTRUCT
}

Machine::~Machine() {
	ASSERT_(!is_initialized && !is_started, "Machine should be in a clean state upon destruction");
	systems.Clear();
	DBG_DESTRUCT
}


bool Machine::Start() {
	ASSERT_(!is_initialized && !is_started, "Shouldn't call Start if we already started");
	ASSERT_(!is_failed, "Machine have already failed");
	if (is_failed)
		return false;
	
	Cout() << "Machine::Start " << ((bool)WhenInitialize ? "with" : "without") << " initializer callback\n";
	WhenInitialize();
	
	if (is_failed)
		return false;
	
	is_started = true;
	
	auto systems = node.FindAll<SystemBase>();
	for (auto system : systems) {
		if (!system->Initialize()) {
			LOG("Could not initialize system " << system->GetType().GetName());
			return false;
		}
	}
	
	WhenPostInitialize();
	
	is_initialized = true;
	
	for (auto& system : systems) {
		system->Start();
	}
	
	warning_age = 0;
	
	is_running = true;
	return true;
}

void Machine::Update(double dt) {
	ASSERT_(is_started, "Shouldn't call Update if we haven't been started");
	
	warning_age += dt;
	if (warning_age > 3.0) {
		warning_age = 0;
		last_warnings.Clear();
	}
	
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
	
	auto systems = node.FindAll<SystemBase>();
	for (auto& system : systems) {
		SystemBase* b = &*system;
		WhenEnterSystemUpdate(*b);
		
		system->Update(dt);
		
		WhenLeaveSystemUpdate();
		
		if (!IsRunning())
			break;
	}
	
	++ticks;
	
	WhenLeaveUpdate();
}

void Machine::Stop() {
	bool was_started = is_started;
	auto systems = node.FindAll<SystemBase>();
	
	is_running = false;
	is_started = false;
	
	DBG_BEGIN_UNREF_CHECK
	
	if (was_started) {
		for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
			(*it)->Stop();
		}
	}
	
	is_initialized = false;
	
	for (auto it = systems.End()-1; it != systems.Begin()-1; --it) {
		(*it)->Uninitialize();
	}
	
	if (is_failed) {
		LOG("Machine::Stop: failure: " << fail_msg);
		SetExitCode(1);
	}
}

void Machine::Suspend() {
	is_suspended = true;
}

void Machine::Resume() {
	is_suspended = false;
}

bool Machine::HasStarted() const {
	return is_started;
}

void Machine::Add(TypeCls type_id, SystemBase* system) {
	ASSERT_(!is_started, "Invalid to add systems after the machine has started");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	ASSERT(system->node.owner);
	systems.Add(type_id, system);
}

void Machine::Remove(TypeCls type_id) {
	ASSERT_(!is_started, "Invalid to remove systems after the machine has started");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	systems.Remove(i);
}

void Machine::Visit(Vis& vis) {
	auto systems = node.FindAll<SystemBase>();
	for (auto& it : systems)
		it->Visit(vis);
}

void Machine::WarnDeveloper(String msg) {
	if (last_warnings.Find(msg) < 0) {
		last_warnings.Add(msg);
		
		#if !defined flagDEBUG_RT && defined flagDEBUG
		LOG("Machine: developer warning: " << msg);
		#endif
	}
}







Machine* __active_machine;

Machine& GetActiveMachine() {
	ASSERT(__active_machine);
	return *__active_machine;
}

void SetActiveMachine(Machine& m) {
	__active_machine = &m;
}

void ClearActiveMachine() {
	__active_machine = 0;
}


END_UPP_NAMESPACE



NAMESPACE_UPP

#if 0
void SingleMachine::Run(void(*fn)(), void(*arg_fn)()) {
	if (Open(arg_fn)) {
		fn();
		Close();
	}
}
#endif

END_UPP_NAMESPACE
