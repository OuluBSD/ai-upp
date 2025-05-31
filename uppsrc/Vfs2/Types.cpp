#include "Vfs.h"
#include <AICore/AICore.h>

NAMESPACE_UPP

void DatasetPtrs::operator=(const DatasetPtrs& p) {
	#define DATASET_ITEM(type, name, desc) name = p.name;
	DATASET_LIST
	#undef DATASET_ITEM
	
	editable_biography = p.editable_biography;
}

void DatasetPtrs::Clear() {
	#define DATASET_ITEM(type, name, desc) name = 0;
	DATASET_LIST
	#undef DATASET_ITEM
	editable_biography = 0;
}

void FillDataset(DatasetPtrs& p, VfsValue& n, Component* this_comp) {
	p.Clear();
	p.component = this_comp;
	//if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN && n.kind <= METAKIND_ECS_COMPONENT_END) {
	if (n.owner) {
		if (n.owner->IsTypeHash<Entity>() && n.owner->ext)
			p.entity = dynamic_cast<Entity*>(&*n.owner->ext);
		for (auto& sub : n.owner->sub) {
			if (!sub.ext) continue;
			VfsValueExt* ext = &*sub.ext;
			TODO
			#if 0
			switch (ext->val.kind) {
				#define DATASET_ITEM(type, name, desc) \
					case kind: {p.name = dynamic_cast<type*>(ext); ASSERT(p.name);} break;
				COMPONENT_LIST
				#undef DATASET_ITEM
				default: break;
			}
			#endif
		}
		if (this_comp) {
			TODO
			#if 0
			switch (n.kind) {
				#define DATASET_ITEM(type, name, desc) \
				case kind: {p.name = dynamic_cast<type*>(this_comp); ASSERT(p.name);} break;
				COMPONENT_LIST
				#undef DATASET_ITEM
				default: break;
			}
			#endif
		}
	}
	
	
// see SRC_TXT_HEADER_ENABLE
	VfsValue* db_src = 0;
	TODO
	#if 0
	if (n.IsTypeHash<SrcTxtHeader>()) {
		db_src = &n;
	}
	
	if (p.entity) {
		p.env = IdeMetaEnv().FindNodeEnv(*p.entity);
		if (p.env && !db_src) {
			bool found_db_src = false;
			for (VfsValue& s : p.env->sub) {
				if (s.kind == METAKIND_DB_REF) {
					for (auto db : ~DatasetIndex()) {
						VfsValueExt& ext = *db.value;
						if (ext.val.kind == AsTypeHash<SrcTxtHeader>()) {
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
			
			// UGLY, so UGLY, just filling values randomly kind of ugly, but it's correct
			// This ugliness is related to the SRC_TXT_HEADER_ENABLE hotfix...
			// Generalize the hotfix or remove it... (don't remove it, it's a good one).
			if (p.src->data)
				p.srctxt = &*p.src->data;
		}
	}
	#endif
}

extern void (*FillDatasetPtr)(DatasetPtrs& p, VfsValue& n, Component* this_comp);

INITBLOCK {
	FillDatasetPtr = &FillDataset;
}

END_UPP_NAMESPACE
