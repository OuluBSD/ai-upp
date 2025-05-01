#include "Eon.h"


NAMESPACE_UPP




void Factory::Dump() {
	const auto& fns = AtomDataMap();
	
	LOG("Factory::Dump:");
	LOG("\tatoms (" << fns.GetCount() << "):");
	for(int i = 0; i < fns.GetCount(); i++) {
		const auto& d = fns[i];
		LOG("\t\t" << i << ": " << d.name << "(" << d.cls.ToString() << ")");
	}
}




END_UPP_NAMESPACE
