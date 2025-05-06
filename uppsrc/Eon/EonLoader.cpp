#include "Eon.h"


NAMESPACE_UPP
namespace Eon {
using namespace Ecs;


bool ExtScriptEcsLoader::Load(ScriptWorldLoader& l) {
	Ecs::Engine& eng = Ecs::GetActiveEngine();
	
	for (ScriptEcsSystemLoader& loader : l.systems) {
		String id = loader.def.id.ToString();
		RTLOG("ScriptEngineLoader::Load: " << id);
		
		Ptr<Ecs::SystemBase> sys = eng.GetAdd(id, false); // skip startup
		if (!sys) {
			SetError("could not find ecs system with id '" + id + "'");
			return false;
		}
		
		if (!Load(loader, *sys)) {
			SetError(l.def.id.ToString() + ": " + loader.GetErrorString());
			return false;
		}
	}
	
	TODO
	#if 0
	Ptr<EntityStore> ents = eng.Get<EntityStore>();
	PoolPtr pool = ents->GetRoot();
	
	for (ScriptPoolLoader& loader : l.pools) {
		String name = loader.def.id.ToString();
		PoolPtr pool0 = pool->GetAddPool(name);
		ASSERT(pool0);
		if (!Load(loader, *pool0)) {
			SetError(l.def.id.ToString() + ": " + loader.GetErrorString());
			return false;
		}
	}
	#endif
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptEcsSystemLoader& l, Ecs::SystemBase& sys) {
	for(int i = 0; i < l.def.args.GetCount(); i++) {
		String key = l.def.args.GetKey(i);
		const Value& value = l.def.args[i];
		if (!value.IsVoid()) {
			if (!sys.Arg(key, value))
				return false;
		}
	}
	
	Ecs::Engine& eng = Ecs::GetActiveEngine();
	TypeCls type = sys.GetTypeCls();
	eng.SystemStartup(type, &sys);
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptPoolLoader& l, Ecs::Pool& pool) {
	auto ents = pool.node.FindAll<Ecs::Entity>();
	auto pools = pool.node.FindAll<Pool>();
	
	for (ScriptEntityLoader& e : l.entities) {
		String name = e.def.id.ToString();
		EntityPtr ent = pool.GetAddEmpty(name);
		ASSERT(ent);
		if (!Load(e, *ent))
			return false;
	}
	
	for (ScriptPoolLoader& p : l.pools) {
		String name = p.def.id.ToString();
		PoolPtr pool0 = pool.GetAddPool(name);
		ASSERT(pool0);
		if (!Load(p, *pool0))
			return false;
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptEntityLoader& l, Ecs::Entity& ent) {
	
	for (ScriptComponentLoader& c : l.comps) {
		String name = c.def.id.ToString();
		ComponentBasePtr cb = ent.CreateEon(name);
		if (!cb) {
			SetError("could not create component with id '" + name + "'");
			return false;
		}
		
		if (!Load(c, *cb))
			return false;
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptComponentLoader& l, Ecs::ComponentBase& cb) {
	for(int i = 0; i < l.def.args.GetCount(); i++) {
		String key = l.def.args.GetKey(i);
		const Value& value = l.def.args[i];
		if (!value.IsVoid()) {
			if (!cb.Arg(key, value))
				return false;
		}
	}
	
	return true;
}


}
END_UPP_NAMESPACE

