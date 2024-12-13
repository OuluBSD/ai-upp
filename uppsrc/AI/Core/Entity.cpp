#include "Core.h"


NAMESPACE_UPP

DatasetPtrs Component::GetDataset() const {
	DatasetPtrs p;
	MetaNode& n = node;
	
	FillDataset(p, n, const_cast<Component*>(this));
	return p;
}

void FillDataset(DatasetPtrs& p, MetaNode& n, Component* this_comp) {
	if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN && n.kind <= METAKIND_ECS_COMPONENT_END) {
		if (n.owner && n.owner->kind == METAKIND_ECS_ENTITY && n.owner->ext)
			p.entity = dynamic_cast<Entity*>(&*n.owner->ext);
		p.lyric_struct = n.owner ? n.owner->Find<LyricalStructure>(METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE) : 0;
		p.script = n.owner ? n.owner->Find<Script>(METAKIND_ECS_COMPONENT_SCRIPT) : 0;
		p.lyrics = n.owner ? n.owner->Find<Lyrics>(METAKIND_ECS_COMPONENT_LYRICS) : 0;
		p.song = n.owner ? n.owner->Find<Song>(METAKIND_ECS_COMPONENT_SONG) : 0;
		if (this_comp) {
			p.component = this_comp;
			switch (n.kind) {
				case METAKIND_ECS_COMPONENT_LYRICAL_STRUCTURE: p.lyric_struct = dynamic_cast<LyricalStructure*>(this_comp); break;
				case METAKIND_ECS_COMPONENT_SCRIPT: p.script = dynamic_cast<Script*>(this_comp); break;
				case METAKIND_ECS_COMPONENT_LYRICS: p.lyrics = dynamic_cast<Lyrics*>(this_comp); break;
				case METAKIND_ECS_COMPONENT_SONG: p.song = dynamic_cast<Song*>(this_comp); break;
				default: break;
			}
		}
		
	}
	MetaNode* db_src = 0;
	if (n.kind == METAKIND_DATABASE_SOURCE) {
		db_src = &n;
	}
	
	if (p.entity) {
		p.env = MetaEnv().FindNodeEnv(*p.entity);
		if (p.env && !db_src) {
			bool found_db_src = false;
			for (MetaNode& s : p.env->sub) {
				if (s.kind == METAKIND_DB_REF) {
					for (auto db : ~DatasetIndex()) {
						MetaNodeExt& ext = *db.value;
						if (ext.node.kind == METAKIND_DATABASE_SOURCE) {
							db_src = &ext.node;
							found_db_src = true;
							break;
						}
					}
				}
				if (found_db_src) break;
			}
		}
	}
	
	if (db_src) {
		if (db_src->ext) {
			p.src = dynamic_cast<SrcTxtHeader*>(&*db_src->ext);
			ASSERT(p.src);
			p.src->RealizeData();
			db_src = &n;
		}
	}
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
