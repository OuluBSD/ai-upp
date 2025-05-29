#include "Core.h"

// Keeping 3 separate Engine files for historical git-log reasons

NAMESPACE_UPP


System::System(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

System::~System() {
	DBG_DESTRUCT
}

Engine& System::GetEngine() const {
	Engine* mach = val.FindOwner<Engine>();
	ASSERT(mach);
	if (!mach) throw Exc("no machine");
	return *mach;
}

#if 0



Machine::Machine(VfsValue& n) : MetaMachineBase(n) {
	DBG_CONSTRUCT
}

Machine::~Machine() {
	ASSERT_(!is_initialized && !is_started, "Machine should be in a clean state upon destruction");
	DBG_DESTRUCT
}

Engine& Machine::GetEngine() {
	if (val.owner) {
		auto engs = val.owner->FindAll<Engine>();
		if (engs.GetCount())
			return *engs[0];
	}
	Engine* e = val.FindOwner<Engine>();
	ASSERT(e);
	if (!e) throw Exc("cannot find engine");
	return *e;
}

#endif

Val& Engine::GetRootPool() {
	return val.GetAdd("pool", 0);
}

Val& Engine::GetRootLoop() {
	return val.GetAdd("loop", 0);
}

Val& Engine::GetRootSpace() {
	return val.GetAdd("space", 0);
}

#if 0
bool Machine::Start() {
	ASSERT_(!is_initialized && !is_started, "Shouldn't call Start if we already started");
	ASSERT_(!is_failed, "Machine have already failed");
	if (is_failed)
		return false;
	
	Cout() << "Machine::Start " << ((bool)WhenInitialize ? "with" : "without") << " initializer callback\n";
	WhenInitialize(*this);
	
	if (is_failed)
		return false;
	
	is_started = true;
	
	auto systems = val.FindAll<System>();
	for (auto system : systems) {
		if (!system->Initialize()) {
			LOG("Could not initialize system " << system->GetTypeCls().GetName());
			return false;
		}
		system->SetInitialized();
	}
	
	WhenPostInitialize(*this);
	
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
	
	auto systems = val.FindAll<System>();
	for (auto& system : systems) {
		System* b = &*system;
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
	auto systems = val.FindAll<System>();
	
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
		if ((*it)->IsInitialized()) {
			(*it)->Uninitialize();
			(*it)->SetInitialized(false);
		}
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

void Machine::DieFast() {Start(); Update(0); Stop();}

void Machine::Clear() {
	ticks=0; is_started=0; is_initialized=0; is_suspended=0; is_running=0;
	val.RemoveAllDeep<System>();
}

bool Machine::HasStarted() const {
	return is_started;
}

System* Machine::FindSystem(TypeCls type_id) {
	VfsValue* n = val.FindDeep(type_id);
	return n && n->ext ? CastPtr<System>(&*n->ext) : 0;
}

void Machine::Add(TypeCls type_id, System* system) {
	ASSERT_(!is_started, "Invalid to add systems after the machine has started");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	ASSERT(system->val.owner);
	systems.Add(type_id, system);
}

void Machine::Remove(TypeCls type_id) {
	ASSERT_(!is_started, "Invalid to remove systems after the machine has started");
	
	int i = FindSystem(type_id);
	ASSERT(i >= 0);
	
	systems.Remove(i);
}

void Machine::Visit(Vis& v) {
	_VIS_(ticks)
	 VIS_(last_warnings)
	 VIS_(warning_age)
	 VIS_(is_started)
	 VIS_(is_initialized)
	 VIS_(is_suspended)
	 VIS_(is_running)
	 VIS_(is_failed)
	 VIS_(fail_msg);
	v & mver;
	VIS_THIS(MetaMachineBase);
}

#endif

void Engine::WarnDeveloper(String msg) {
	if (last_warnings.Find(msg) < 0) {
		last_warnings.Add(msg);
		
		#if !defined flagDEBUG_RT && defined flagDEBUG
		LOG("Machine: developer warning: " << msg);
		#endif
	}
}

bool Engine::Start(String app_name, String override_eon_file, VectorMap<String,Value>* extra_args, const char* extra_str) {
	
	if (!override_eon_file.IsEmpty())
		eon_file = override_eon_file;
	
	if (extra_args) {
		if (eon_params.Is<ValueMap>())
			eon_params = ValueMap();
		
		for(int i = 0; i < extra_args->GetCount(); i++)
			eon_params(extra_args->GetKey(i)) = (*extra_args)[i];
	}
	
	if (extra_str) {
		Vector<String> args = Split(extra_str, ";");
		for (String& arg : args) {
			int i = arg.Find("=");
			if (i >= 0) {
				String a = arg.Left(i);
				String b = arg.Mid(i+1);
				eon_params(a) = b;
			}
		}
	}
	
	WhenBoot(*this);
	
	//break_addr = 1;
	if (is_failed)
		return false;
	
	if (!break_addr)
		Start(eon_script, eon_file, eon_params);
	else
		Start(eon_script, eon_file, eon_params, 1, break_addr);
	
	return !is_failed;
}

#if 0
void Machine::StopRunner() {
	SetNotRunning();
	Stop();
	Clear();
}





void SingleMachine::Run(void(*fn)(), void(*arg_fn)()) {
	if (Open(arg_fn)) {
		fn();
		Close();
	}
}
#endif

END_UPP_NAMESPACE

