#include "TextCore.h"


NAMESPACE_UPP

DatasetPtrs Component::GetDataset() {
	DatasetPtrs p;
	MetaNode& n = *node;
	if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN && n.kind <= METAKIND_ECS_COMPONENT_END) {
		if (n.owner && n.owner->kind == METAKIND_ECS_ENTITY && n.owner->ext)
			p.entity = dynamic_cast<Entity*>(&*n.owner->ext);
		p.component = this;
		switch (n.kind) {
			case METAKIND_ECS_COMPONENT_SCRIPT:
				p.script = dynamic_cast<Script*>(this);
				p.lyrics = n.owner ? n.owner->Find<Lyrics>(METAKIND_ECS_COMPONENT_LYRICS) : 0;
				break;
			case METAKIND_ECS_COMPONENT_LYRICS:
				p.script = n.owner ? n.owner->Find<Script>(METAKIND_ECS_COMPONENT_SCRIPT) : 0;
				p.lyrics = dynamic_cast<Lyrics*>(this); break;
			default: break;
		}
		MetaNode* env = MetaEnv().FindNodeEnv(n);
		if (env) {
			TODO//p.src = ; // DatasetIndex().Find(filepath);
		}
	}
	return p;
}


INITIALIZER_COMPONENT(Entity);

END_UPP_NAMESPACE
