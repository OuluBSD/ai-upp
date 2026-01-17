#include "Ecs.h"

NAMESPACE_UPP

bool Entity::InitializeComponents(const WorldState& ws) {
	bool b = true;  // Start optimistic, set false on any failure
	auto comps = val.FindAll<Component>();
	for(auto& comp : comps) {
		if (!comp->IsInitialized()) {
			bool success = comp->Initialize(ws);
			if (!success) {
				LOG("Entity::InitializeComponents: component " << comp->GetTypeName() << " failed to initialize");
				b = false;
			}
			comp->SetInitialized(success);
		}
	}
	return b;
}

void Entity::UninitializeComponents() {
	auto comps = val.FindAll<Component>();
	int dbg_i = 0;
	for (auto it = comps.End()-1; it != comps.Begin()-1; --it) {
		if ((*it)->IsInitialized()) {
			(*it)->Uninitialize();
			(*it)->SetInitialized(false);
		}
		dbg_i++;
	}
}

void Entity::ClearComponents() {
	val.RemoveAllDeep<Component>();
}

END_UPP_NAMESPACE
