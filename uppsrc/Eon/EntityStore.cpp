#include "Eon.h"


NAMESPACE_UPP namespace Ecs {

void EntityStore::InitRoot() {
	node.RemoveAllShallow<Pool>();
	MetaNode& n = node.Add();
	n.kind = 0; TODO // solve kind
	Pool* p = new Pool(n);
	n.ext = p;
	p->SetName("root");
	p->SetId(Pool::GetNextId());
}

bool EntityStore::Initialize() {
	return true;
}

void EntityStore::Uninitialize() {
	GetRoot()->Clear();
}

void EntityStore::Update(double dt) {
	// Slow: GetRoot()->PruneFromContainer();
	
	if (this->destroy_list.GetCount()) {
		Vector<Entity*> destroy_list;
		Swap(this->destroy_list, destroy_list);
		for (Entity* e : destroy_list) {
			if (e)
				e->GetPool().RemoveEntity(e);
		}
	}
	
	
}

void EntityStore::AddToDestroyList(Entity* e) {
	VectorFindAdd(destroy_list, e);
}

EntityPtr EntityStore::FindEntity(String path) {
	Vector<String> parts = Split(path, ".", false);
	
	PoolPtr pool = GetRoot();
	for(int i = 0; i < parts.GetCount(); i++) {
		PoolPtr new_pool;
		for(int c = i+1; c <= parts.GetCount(); c++) {
			bool is_ent = c == parts.GetCount();
			String p = parts[i];
			for(int j = i+1; j < c; j++)
				p << "." << parts[j];
			
			if (is_ent) {
				return pool->FindEntityByName(p);
			}
			else {
				new_pool = pool->FindPool(p);
				if (new_pool)
					break;
			}
		}
		if (!new_pool)
			break;
		pool = new_pool;
	}
	return EntityPtr();
}


}




Ecs::Engine& Machine::GetEngine() {
	TODO
	#if 0
	EntitySystemPtr es = Get<EntitySystem>();
	if (es)
		return es->GetEngine();
	
	Panic("No EntitySystem in machine");
	UNREACHABLE;
	#endif
	throw Exc("unreachable");
}


END_UPP_NAMESPACE
