#include "Eon.h"

#if 0

NAMESPACE_UPP


Component::Component(VfsValue& n) : VfsValueExt(n) {
	DBG_CONSTRUCT
}

Component::~Component() {
	DBG_DESTRUCT
}

Engine& Component::GetEngine() {
	return *val.FindOwnerRoot<Engine>();
}

Entity* Component::GetEntity() {
	return val.FindOwnerRoot<Entity>();
}

String Component::ToString() const {
	return GetTypeCls().GetName();
}

void Component::GetComponentPath(Vector<String>& path) {
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

void Component::AddToUpdateList() {GetEngine().AddToUpdateList(this);}
void Component::RemoveFromUpdateList() {GetEngine().RemoveFromUpdateList(this);}

String Component::GetDynamicName() const {
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

void ComponentMap::ReturnComponent(ComponentStore& s, Component* c) {
	s.ReturnComponent(c);
}
#endif


END_UPP_NAMESPACE

#endif
