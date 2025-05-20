#include "AICore.h"


NAMESPACE_UPP


Agent::Agent(MetaNode& n) : Component(n) {
	ASSERT(AgentInteractionSystem::sys);
	if (AgentInteractionSystem::sys) {
		AgentInteractionSystem::sys->Attach(this);
	}
}

Agent::~Agent() {
	if (AgentInteractionSystem::sys) {
		AgentInteractionSystem::sys->Detach(this);
	}
}

bool Agent::RealizeLibrary(MsgCb WhenMessage) {
	if (global.IsEmpty()) {
		Escape(global, "Print(x)", Proxy(WhenPrint));
		Escape(global, "Input()", Proxy(WhenInput));
		StdLib(global);
	}
	return true;
}

bool Agent::Catch(Event<> cb, Vector<ProcMsg>& msgs) {
	bool succ = true;
	try {
		cb();
	}
	catch(CParser::Error e) {
		ProcMsg& m = msgs.Add();
		m.msg = e;
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	catch(Exc e) {
		ProcMsg& m = msgs.Add();
		m.msg = "Exception: " + e;
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	catch(...) {
		ProcMsg& m = msgs.Add();
		m.msg = "Unknown error";
		m.severity = PROCMSG_ERROR;
		succ = false;
	}
	return succ;
}

bool Agent::Compile(String esc, MsgCb WhenMessage) {
	hash_t h = esc.GetHashValue();
	if (compiled_hash && compiled_hash == h)
		return true;
	
	compiled_hash = 0;
	bool succ = true;
	
	if (!RealizeLibrary(WhenMessage))
		return false;
	
	Vector<ProcMsg> msgs;
	succ = Catch([this,&esc]{
		Scan(global, esc);
	}, msgs);
	
	if (msgs.GetCount())
		WhenMessage(msgs);
	
	if (succ)
		compiled_hash = h;
	
	return succ;
}

bool Agent::Run(MsgCb WhenMessage) {
	bool succ = true;
	
	Vector<ProcMsg> msgs;
	succ = Catch([this]{
		Execute(global, "main", oplimit);
	}, msgs);
	
	if (msgs.GetCount())
		WhenMessage(msgs);
	
	return succ;
}

INITIALIZER_COMPONENT(Agent);



void AgentInteractionSystem::Start() {
	if (flag.IsRunning()) return;
	flag.Start();
	Thread::Start(THISBACK(Runner));
}

void AgentInteractionSystem::Stop() {
	flag.Stop();
}

void AgentInteractionSystem::Runner() {
	
	while (flag.IsRunning() && !Thread::IsShutdownThreads()) {
		
		
		Sleep(1);
	}
	
	flag.SetStopped();
}

void AgentInteractionSystem::Attach(Agent* a) {
	for (auto& ap : agents)
		if (ap == a)
			return;
	
}

void AgentInteractionSystem::Detach(Agent* a) {
	for(int i = 0; i < agents.GetCount(); i++) {
		if (agents[i] == a) {
			agents.Remove(i);
			
			
			return;
		}
	}
}

void AgentInteractionSystem::Setup() {
	AgentInteractionSystem& sys = MetaEnv().root.Add<AgentInteractionSystem>("agentsys");
	AgentInteractionSystem::sys = &sys;
	sys.Start();
}

void AgentInteractionSystem::Uninstall() {
	if (sys)
		sys->Stop();
}

AgentInteractionSystem* AgentInteractionSystem::sys;


END_UPP_NAMESPACE
