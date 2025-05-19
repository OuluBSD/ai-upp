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

//INITIALIZER_COMPONENT(Agent);



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
