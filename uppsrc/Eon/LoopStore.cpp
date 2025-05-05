#include "Eon.h"

#if 0
NAMESPACE_UPP


void LoopStore::InitRoot() {
	root.Clear();
	Loop& p = root.Add();
	p.SetParent(LoopParent(this,0));
	p.SetName("root");
	p.SetId(Loop::GetNextId());
	
	Ptr<SpaceStore> ss = GetMachine().Find<SpaceStore>();
	if (ss) {
		p.space = &*ss->GetRoot();
	}
}

bool LoopStore::Initialize() {
	return true;
}

void LoopStore::Uninitialize() {
	GetRoot()->Clear();
}

void LoopStore::Update(double dt) {
	// pass
}




END_UPP_NAMESPACE
#endif
