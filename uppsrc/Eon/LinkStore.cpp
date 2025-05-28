#include "Eon.h"

NAMESPACE_UPP


void LinkStore::ReturnLink(Base* c) {
	ASSERT(c);
	LinkTypeCls type = c->GetLinkType();
	
	auto iter = LinkFactory::refurbishers.Find(type);
	if (iter)
		iter.Get()(c);
}

LinkBase* LinkStore::CreateLink(LinkTypeCls cls) {
	auto iter = LinkFactory::producers.Find(cls);
	ASSERT_(iter, "Invalid to create non-existant atom");
	
	LinkBase* obj = iter.value()();
	return obj;
}

LinkBase* LinkStore::CreateLinkTypeCls(LinkTypeCls cls) {
	TODO
	#if 0
	auto it = VfsValueExtFactory::producers.Find(cls);
	if (!it) {
		auto new_fn = VfsValueExtFactory::LinkDataMap().Get(cls).new_fn;
		std::function<LinkBase*()> p([new_fn] { return new_fn();});
		std::function<void(LinkBase*)> r([] (Base* b){ delete b;});
		Factory::producers.Add(cls) = p;
		Factory::refurbishers.Add(cls) = r;
	}
	
	return CreateLink(cls);
	#endif
	return 0;
}


END_UPP_NAMESPACE
