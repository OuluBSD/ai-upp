#include "Script.h"


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
    RTLOG("ScriptEngineLoader::Load: got root pool, pools count=" << l.pools.GetCount());
    RTLOG("ScriptEngineLoader::Load: root pool id='" << pool.id << "' sub.GetCount()=" << pool.sub.GetCount());

    for (ScriptPoolLoader& loader : l.pools) {
        String name = loader.def.id.ToString();
        RTLOG("ScriptEngineLoader::Load: getting pool '" << name << "'");
        VfsValue& pool0 = pool.GetAdd(name, 0);
        RTLOG("ScriptEngineLoader::Load: got pool0, id='" << pool0.id << "' sub.GetCount()=" << pool0.sub.GetCount());
        //ASSERT(pool0);
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
    PoolContext pool_ctx(pool, &l.parent.parent.parent);
    
    for (ScriptEntityLoader& e : l.entities) {
        String name = e.def.id.ToString();
        EntityContext ent_ctx = pool_ctx.AddEntity(name, &e.def.args, &e.def.loc);
        if (!Load(e, ent_ctx.ent))
            return false;
    }
    
    for (ScriptPoolLoader& p : l.pools) {
        String name = p.def.id.ToString();
        PoolContext sub = pool_ctx.AddPool(name, &p.def.args, &p.def.loc);
        if (!Load(p, sub.v))
            return false;
    }
    
    return true;
}

bool ExtScriptEcsLoader::Load(ScriptEntityLoader& l, Entity& ent) {
    EntityContext ent_ctx(ent, &l.parent.parent.parent.parent);

    for (ScriptComponentLoader& c : l.comps) {
        String name = c.def.id.ToString();
        ComponentContext comp_ctx = ent_ctx.AddComponent(name, &c.def.args, &c.def.loc);
        // component args were already applied in AddComponent
        (void)comp_ctx;
    }

    // NOTE: Do NOT initialize here! Initialize needs full tree to exist.
    // Initialization happens later in a separate phase.
    
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
