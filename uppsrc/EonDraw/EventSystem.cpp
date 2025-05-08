#include "EonDraw.h"


NAMESPACE_UPP namespace Ecs {


bool EventSystem::Initialize() {
	return true;
}

/*void EventSystem::Attach(Serial::EcsEventsBase* b) {
	ASSERT(b);
	EventStateBase::AddBinder(this);
	serial = b;
}*/

void EventSystem::Start() {
	
}

void EventSystem::Update(double dt) {
	
}

void EventSystem::Stop() {
	
}

void EventSystem::Uninitialize() {
	#ifdef flagSCREEN
	EventStateBase::RemoveBinder(this);
	#endif
}


} END_UPP_NAMESPACE

