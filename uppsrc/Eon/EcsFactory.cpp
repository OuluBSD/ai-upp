#include "Eon.h"

NAMESPACE_UPP namespace Ecs {



void ComponentFactory::Dump() {
	const auto& fns = CompDataMap();
	
	LOG("ComponentFactory::Dump:");
	LOG("\tcomponents (" << fns.GetCount() << "):");
	for(int i = 0; i < fns.GetCount(); i++) {
		const auto& d = fns[i];
		LOG("\t\t" << i << ": " << d.name);
	}
}

ComponentBase* ComponentFactory::CreateComponent(MetaNode& n, TypeCls type) {
	int i = CompDataMap().Find(type);
	if (i < 0) return 0;
	CompData& d = CompDataMap()[i];
	MetaNode& sub = n.sub.Add();
	ComponentBase* c = d.new_fn(sub);
	sub.ext = c;
	sub.type_hash = c->GetTypeHash();
	return c;
}

String ComponentFactory::GetComponentName(TypeCls type) {
	for (CompData& c : CompDataMap().GetValues()) {
		if (c.rtti_cls == type)
			return c.name;
	}
	return "";
}

TypeCls ComponentFactory::GetComponentType(String name) {
	for (CompData& c : CompDataMap().GetValues()) {
		if (c.name == name)
			return c.rtti_cls;
	}
	return TypeCls();
}



} END_UPP_NAMESPACE
