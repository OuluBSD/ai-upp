#include "TextCore.h"


NAMESPACE_UPP

DatasetPtrs Component::GetDataset() const {
	DatasetPtrs p;
	MetaNode& n = node;
	Component* this_comp = const_cast<Component*>(this);
	if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN && n.kind <= METAKIND_ECS_COMPONENT_END) {
		if (n.owner && n.owner->kind == METAKIND_ECS_ENTITY && n.owner->ext)
			p.entity = dynamic_cast<Entity*>(&*n.owner->ext);
		p.component = this_comp;
		p.lyric_struct = n.owner ? n.owner->Find<LyricalStructure>(METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE) : 0;
		p.script = n.owner ? n.owner->Find<Script>(METAKIND_ECS_COMPONENT_SCRIPT) : 0;
		p.lyrics = n.owner ? n.owner->Find<Lyrics>(METAKIND_ECS_COMPONENT_LYRICS) : 0;
		p.song = n.owner ? n.owner->Find<Song>(METAKIND_ECS_COMPONENT_SONG) : 0;
		switch (n.kind) {
			case METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE: p.lyric_struct = dynamic_cast<LyricalStructure*>(this_comp); break;
			case METAKIND_ECS_COMPONENT_SCRIPT: p.script = dynamic_cast<Script*>(this_comp); break;
			case METAKIND_ECS_COMPONENT_LYRICS: p.lyrics = dynamic_cast<Lyrics*>(this_comp); break;
			case METAKIND_ECS_COMPONENT_SONG: p.song = dynamic_cast<Song*>(this_comp); break;
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
	else {
		Value val = data[i];
		if (val.Is<int>() || val.Is<int64>() ||val.Is<double>())
			return (int)val;
		else
			return val.ToString() == "female";
	}
}

INITIALIZER_COMPONENT(Entity);

END_UPP_NAMESPACE
