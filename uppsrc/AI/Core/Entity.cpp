#include "Core.h"


NAMESPACE_UPP

DatasetPtrs Component::GetDataset() const {
	DatasetPtrs p;
	MetaNode& n = node;
	
	FillDataset(p, n, const_cast<Component*>(this));
	return p;
}

void FillDataset(DatasetPtrs& p, MetaNode& n, Component* this_comp) {
	p.Clear();
	p.component = this_comp;
	//if (n.kind >= METAKIND_ECS_COMPONENT_BEGIN && n.kind <= METAKIND_ECS_COMPONENT_END) {
	if (n.owner) {
		if (n.owner->kind == METAKIND_ECS_ENTITY && n.owner->ext)
			p.entity = dynamic_cast<Entity*>(&*n.owner->ext);
		for (auto& sub : n.owner->sub) {
			if (!sub.ext) continue;
			MetaNodeExt* ext = &*sub.ext;
			switch (ext->node.kind) {
				#define DATASET_ITEM(type, name, kind, group, desc) \
					case kind: {p.name = dynamic_cast<type*>(ext); ASSERT(p.name);} break;
				COMPONENT_LIST
				#undef DATASET_ITEM
				default: break;
			}
		}
		if (this_comp) {
			switch (n.kind) {
				#define DATASET_ITEM(type, name, kind, group, desc) \
				case kind: {p.name = dynamic_cast<type*>(this_comp); ASSERT(p.name);} break;
				COMPONENT_LIST
				#undef DATASET_ITEM
				default: break;
			}
		}
	}
	
	
// see SRC_TXT_HEADER_ENABLE
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

EntityData* Entity::FindData(const VfsPath& path) {
	int i = objs.Find(path);
	if (i >= 0)
		return &objs[i];
	return 0;
}

void Entity::Visit(NodeVisitor& v) {
	v.Ver(2)
	(1)	("data",data);
	if (v.file_ver >= 2) {
		struct Item : Moveable<Item> {
			VfsPath path;
			int kind;
			void Visit(NodeVisitor& v) {v("path",path,VISIT_NODE)("kind",kind);}
		};
		if (v.IsLoading()) {
			Vector<Item> header;
			v(2)(".header", header, VISIT_VECTOR);
			this->objs.Clear();
			for(auto& it : header) {
				#define DATASET_ITEM(a,b,c,d,e) \
				if (it.kind == c) {\
					a* o = new a(); \
					this->objs.Add(it.path, o); \
					v((String)it.path, *o, VISIT_NODE); \
					continue;\
				}
				VIRTUALNODE_DATASET_LIST
				#undef DATASET_ITEM
				ASSERT_(0,"Invalid kind");
				v.SetError("Invalid kind");
				break;
			}
		}
		else {
			Vector<Item> header;
			header.Reserve(objs.GetCount());
			for (auto it : ~objs) {
				Item& i = header.Add();
				i.path = it.key;
				i.kind = it.value.GetKind();
			}
			v(2)(".header", header, VISIT_VECTOR);
			for (auto it : ~objs) {
				v((String)it.key, it.value, VISIT_NODE); \
			}
		}
	}
}

INITIALIZER_COMPONENT(Entity);

END_UPP_NAMESPACE
