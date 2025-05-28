#include "Eon.h"


#if 0
NAMESPACE_UPP



void VfsValueExtFactory::Dump() {
	const auto& fns = AtomDataMap();
	
	LOG("Factory::Dump:");
	LOG("\tatoms (" << fns.GetCount() << "):");
	for(int i = 0; i < fns.GetCount(); i++) {
		const auto& d = fns[i];
		LOG("\t\t" << i << ": " << d.name << "(" << d.cls.ToString() << ")");
	}
	LOG("\tlinks (" << fns.GetCount() << "):");
	for(int i = 0; i < fns.GetCount(); i++) {
		const auto& d = fns[i];
		LOG("\t\t" << i << ": " << d.name << "(" << d.cls.ToString() << ")");
	}
}



END_UPP_NAMESPACE
#endif
