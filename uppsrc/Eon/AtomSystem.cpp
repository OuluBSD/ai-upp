#include "Eon.h"

  
NAMESPACE_UPP




bool AtomSystem::Initialize() {
	ASSERT_(!GetMachine().Find<AtomStore>().IsEmpty(), "AtomStore must be added before AtomSystem");
	return true;
}

void AtomSystem::Start() {
	
}

void AtomSystem::Update(double dt) {
	
	for (AtomBaseRef& c : updated) {
		c->Update(dt);
	}
	
}


void AtomSystem::Stop() {
	
}

void AtomSystem::Uninitialize() {
	
	WhenUninit()();
	
	updated.Clear();
}

void AtomSystem::AddUpdated(AtomBaseRef p) {
	if (p)
		updated.FindAdd(p);
}

void AtomSystem::RemoveUpdated(AtomBaseRef p) {
	updated.RemoveKey(p);
}



END_UPP_NAMESPACE
