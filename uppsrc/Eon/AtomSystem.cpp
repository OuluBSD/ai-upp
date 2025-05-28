#include "Eon.h"

#if 0


NAMESPACE_UPP


bool AtomSystem::Initialize() {
	return true;
}

void AtomSystem::Start() {
	
}

void AtomSystem::Update(double dt) {
	
	for (AtomBasePtr& c : updated) {
		c->Update(dt);
	}
	
}


void AtomSystem::Stop() {
	
}

void AtomSystem::Uninitialize() {
	
	WhenUninit()();
	
	updated.Clear();
}

void AtomSystem::AddUpdated(AtomBasePtr p) {
	if (p)
		updated.FindAdd(p);
}

void AtomSystem::RemoveUpdated(AtomBasePtr p) {
	updated.RemoveKey(p);
}

END_UPP_NAMESPACE

#endif

