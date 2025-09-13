#include "Eon.h"


NAMESPACE_UPP
namespace Eon {


bool ExtScriptEcsLoader::Load(ScriptWorldLoader& l) {
	Engine* engp = l.parent.parent.val.FindOwnerWith<Engine>();
	if (!engp) {
		RLOG("ExtScriptEcsLoader::Load: error: no Engine");
		return false;
	}
	Engine& eng = *engp;
	
	for (ScriptEcsSystemLoader& loader : l.systems) {
		String id = loader.def.id.ToString();
		RTLOG("ScriptEngineLoader::Load: " << id);
		
		Ptr<System> sys = eng.GetAdd(id, false); // skip startup
		if (!sys) {
			SetError("could not find ecs system with id '" + id + "'");
			return false;
		}
		
		if (!Load(loader, *sys)) {
			SetError(l.def.id.ToString() + ": " + loader.GetErrorString());
			return false;
		}
	}
	
	Val& pool = eng.GetRootPool();
	
	for (ScriptPoolLoader& loader : l.pools) {
		String name = loader.def.id.ToString();
		VfsValue& pool0 = pool.GetAdd(name, 0);
		ASSERT(pool0);
		if (!Load(loader, pool0)) {
			SetError(l.def.id.ToString() + ": " + loader.GetErrorString());
			return false;
		}
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptEcsSystemLoader& l, System& sys) {
	for(int i = 0; i < l.def.args.GetCount(); i++) {
		String key = l.def.args.GetKey(i);
		const Value& value = l.def.args[i];
		if (!value.IsVoid()) {
			if (!sys.Arg(key, value))
				return false;
		}
	}
	
	Engine* eng = sys.val.FindOwnerWith<Engine>();
	ASSERT(eng);
	if (!eng) return false;
	
	TypeCls type = sys.GetTypeCls();
	eng->SystemStartup(type, &sys);
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptPoolLoader& l, VfsValue& pool) {
	
	for (ScriptEntityLoader& e : l.entities) {
		String name = e.def.id.ToString();
		Entity& ent = pool.GetAdd<Entity>(name);
		if (!Load(e, ent))
			return false;
	}
	
	for (ScriptPoolLoader& p : l.pools) {
		String name = p.def.id.ToString();
		VfsValue& pool0 = pool.GetAdd(name, 0);
		ASSERT(pool0);
		if (!Load(p, pool0))
			return false;
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptEntityLoader& l, Entity& ent) {
	
	for (ScriptComponentLoader& c : l.comps) {
		String name = c.def.id.ToString();
		ComponentPtr cb = ent.CreateEon(name);
		if (!cb) {
			SetError("could not create component with id '" + name + "'");
			return false;
		}
		
		if (!Load(c, *cb))
			return false;
	}
	
	return true;
}

bool ExtScriptEcsLoader::Load(ScriptComponentLoader& l, Component& cb) {
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
