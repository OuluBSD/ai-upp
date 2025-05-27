#include "Eon.h"


NAMESPACE_UPP


ComponentBase::ComponentBase(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

ComponentBase::~ComponentBase() {
	DBG_DESTRUCT
}

Engine& ComponentBase::GetEngine() {
	return *val.FindOwnerRoot<Engine>();
}

Entity* ComponentBase::GetEntity() {
	return val.FindOwnerRoot<Ecs::Entity>();
}

String ComponentBase::ToString() const {
	return GetTypeCls().GetName();
}

void ComponentBase::GetComponentPath(Vector<String>& path) {
	path.Clear();
	
	String name = ComponentFactory::GetComponentName(GetTypeCls());
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

void ComponentBase::AddToUpdateList() {GetEngine().AddToUpdateList(this);}
void ComponentBase::RemoveFromUpdateList() {GetEngine().RemoveFromUpdateList(this);}

String ComponentBase::GetDynamicName() const {
	return GetTypeCls().GetName();
}



#if 0
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
#endif


END_UPP_NAMESPACE
