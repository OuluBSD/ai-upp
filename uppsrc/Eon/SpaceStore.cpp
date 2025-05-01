#include "Eon.h"


NAMESPACE_UPP


void SpaceStore::InitRoot() {
	root.Clear();
	Space& p = root.Add();
	p.SetParent(SpaceParent(this,0));
	p.SetName("root");
	p.SetId(Space::GetNextId());
}

bool SpaceStore::Initialize() {
	return true;
}

void SpaceStore::Stop() {
	GetRootPtr()->StopDeep();
}

void SpaceStore::Uninitialize() {
	GetRootPtr()->Clear();
}

void SpaceStore::Update(double dt) {
	// pass
}


END_UPP_NAMESPACE
