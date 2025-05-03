#include "Eon.h"

NAMESPACE_UPP namespace Ecs {








ComponentBase::ComponentBase() {
	DBG_CONSTRUCT
}

ComponentBase::~ComponentBase() {
	DBG_DESTRUCT
}

Engine& ComponentBase::GetEngine() {
	return GetParent()->GetEngine();
}

EntityPtr ComponentBase::GetEntity() {
	return GetParent()->AsRefT();
}

String ComponentBase::ToString() const {
	return GetDynamicName();
}

void ComponentBase::GetComponentPath(Vector<String>& path) {
	path.Clear();
	
	String name = ComponentFactory::GetComponentName(GetTypeId());
	ASSERT(!name.IsEmpty());
	path.Add(name);
	
	EntityPtr ent = GetEntity();
	String ent_name = ent->GetName();
	ASSERT(!ent_name.IsEmpty());
	path.Add(ent_name);
	
	Pool* pool = &ent->GetPool();
	while (pool) {
		String pool_name = pool->GetName();
		path.Add(pool_name);
		pool = pool->GetParent();
	}
	
	Reverse(path);
}






void ComponentMap::Dump() {
	auto iter = ComponentMapBase::begin();
	for(int i = 0; iter; ++iter, ++i) {
		LOG(i << ": " <<
			iter.value().GetDynamicName() << ": \"" <<
			iter.value().ToString() << "\"");
	}
}

void ComponentMap::ReturnComponent(ComponentStore& s, ComponentBase* c) {
	s.ReturnComponent(c);
}




















} END_UPP_NAMESPACE
