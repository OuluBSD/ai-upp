#include "Eon.h"

NAMESPACE_UPP


EcsService::EcsService() {
	
}

bool EcsService::Init(String name) {
	
	return true;
}

void EcsService::Update() {
	
	// Require EnetServiceClient
	if (!server) {
		server = base->FindServiceT<EnetServiceServer>();
		if (!server)
			return;
		
		server->AddStream(NET_GEOM_LOAD_ENGINE, THISBACK(ReceiveGeoms));
		server->AddStream(NET_GEOM_STORE_ENGINE, THISBACK(SendEngine));
	}
	
}

void EcsService::Stop() {
	
}

void EcsService::Deinit() {
	
}

void EcsService::ReceiveGeoms(Stream& in, Stream& out) {
	TODO
	#if 0
	Machine* m = node.FindOwner<Machine>();
	ASSERT(m);
	Ecs::Engine& eng = m->GetEngine();
	Ecs::PoolPtr root = eng.Get<Ecs::EntityStore>()->GetRoot();
	
	in % *root;
	#endif
	/*for (GeomSerializer::Item& item : read.items) {
		
		switch (item.type) {
		case GEOMVAR_ENTITY_KEY:
			bound_entity = ResolveEntity(root, item.str);
			break;
		case GEOMVAR_RESET_ENTITY_KEY:
			bound_entity.Clear();
			break;
		case GEOMVAR_ORIENTATION: {
				if (!bound_entity) {LOG("EcsService::ReceiveGeoms: error: no entity"); return;}
				Ecs::TransformPtr trans = bound_entity->GetAdd<Ecs::Transform>();
				for(int i = 0; i < 4; i++)
					trans->data.orientation.data.data[i] = item.f[i];
			}
			break;
		case GEOMVAR_POSITION: {
				if (!bound_entity) {LOG("EcsService::ReceiveGeoms: error: no entity"); return;}
				Ecs::TransformPtr trans = bound_entity->GetAdd<Ecs::Transform>();
				for(int i = 0; i < 3; i++)
					trans->data.position.data[i] = item.f[i];
			}
			break;
		case GEOMVAR_MODEL: {
				if (!bound_entity) {LOG("EcsService::ReceiveGeoms: error: no entity"); return;}
				ModelCachePtr mc = mach.Find<ModelCache>();
				if (!mc) {LOG("EcsService::ReceiveGeoms: error: no ModelCache"); return;}
				ModelPtr mdl = mc->Attach(read.DetachModel(item.mdl));
				Ecs::ModelComponentPtr comp = bound_entity->GetAdd<Ecs::ModelComponent>();
				comp->SetModel(mdl);
			}
			break;
		case GEOMVAR_NULL:
			break;
		default:
			break;
		}
	}*/
	
}

void EcsService::SendEngine(Stream& in, Stream& out) {
	TODO
	#if 0
	Machine* m = node.FindOwner<Machine>();
	ASSERT(m);
	Ecs::Engine& eng = m->GetEngine();
	Ecs::Engine& eng = mach.GetEngine();
	Ecs::PoolPtr root = eng.Get<Ecs::EntityStore>()->GetRoot();
	
	out % *root;
	#endif
}

Ecs::EntityPtr EcsService::ResolveEntity(Ecs::PoolPtr& root, String path) {
	Vector<String> names = Split(path, "/");
	if (names.IsEmpty())
		return Ecs::EntityPtr();
	
	Ecs::PoolPtr pool = root;
	for(int i = 0; i < names.GetCount()-1; i++) {
		String n = names[i];
		pool = pool->GetAddPool(n);
	}
	
	Ecs::EntityPtr ent = pool->GetAddEmpty(names.Top());
	
	return ent;
}


END_UPP_NAMESPACE
