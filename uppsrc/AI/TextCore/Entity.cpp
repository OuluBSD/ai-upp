#include "TextCore.h"


NAMESPACE_UPP

DatasetPtrs Component::GetDataset() {
	DatasetPtrs p;
	MetaNode& n = node;
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
		if (p.entity) {
			p.env = MetaEnv().FindNodeEnv(*p.entity);
			if (p.env) {
				bool found_db_src = false;
				for (MetaNode& s : p.env->sub) {
					if (s.kind == METAKIND_DB_REF) {
						for (auto db : ~DatasetIndex()) {
							MetaNodeExt& ext = *db.value;
							if (ext.node.kind == METAKIND_DATABASE_SOURCE) {
								p.src = dynamic_cast<SrcTxtHeader*>(&ext);
								ASSERT(p.src);
								p.src->RealizeData();
								found_db_src = true;
								break;
							}
						}
					}
					if (found_db_src) break;
				}
			}
		}
	}
	return p;
}

int Entity::GetGender() const {
	int i = data.Find("gender");
	if (i < 0)
		return 0;
	else
		return data[i];
}

INITIALIZER_COMPONENT(Entity);

END_UPP_NAMESPACE
