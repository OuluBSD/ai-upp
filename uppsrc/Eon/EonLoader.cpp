#include "Eon.h"


NAMESPACE_UPP
namespace Eon {
using namespace Ecs;


bool ExtScriptEcsLoader::Load(ScriptWorldLoader& l) {
	Ecs::Engine* engp = l.parent.parent.val.FindOwnerWith<Ecs::Engine>();
	if (!engp) {
		RLOG("ExtScriptEcsLoader::Load: error: no Engine");
		return false;
	}
	Ecs::Engine& eng = *engp;
	
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
	
	PoolPtr pool = &eng.GetRootPool();
	
	for (ScriptPoolLoader& loader : l.pools) {
		String name = loader.def.id.ToString();
		PoolPtr pool0 = pool->GetAddPool(name);
		ASSERT(pool0);
		if (!Load(loader, *pool0)) {
			SetError(l.def.id.ToString() + ": " + loader.GetErrorString());
			return false;
		}
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptEcsSystemLoader& l, Ecs::System& sys) {
	for(int i = 0; i < l.def.args.GetCount(); i++) {
		String key = l.def.args.GetKey(i);
		const Value& value = l.def.args[i];
		if (!value.IsVoid()) {
			if (!sys.Arg(key, value))
				return false;
		}
	}
	
	Ecs::Engine* eng = sys.val.FindOwnerWith<Ecs::Engine>();
	ASSERT(eng);
	if (!eng) return false;
	
	TypeCls type = sys.GetTypeCls();
	eng->SystemStartup(type, &sys);
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptPoolLoader& l, Ecs::Pool& pool) {
	auto ents = pool.val.FindAll<Ecs::Entity>();
	auto pools = pool.val.FindAll<Pool>();
	
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

