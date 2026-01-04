#include "Script.h"

NAMESPACE_UPP
namespace Eon {

ScriptPoolLoader::ScriptPoolLoader(ScriptWorldLoader& parent, ScriptPoolLoader* chain_parent, int id, Eon::PoolDefinition& def) :
	Base(parent, id, def),
	chain_parent(chain_parent)
{
	
	for (Eon::EntityDefinition& e : def.ents) {
		entities.Add(new ScriptEntityLoader(*this, entities.GetCount(), e));
	}
	
	for (Eon::PoolDefinition& p : def.pools) {
		pools.Add(new ScriptPoolLoader(parent, this, pools.GetCount(), p));
	}
	
}

bool ScriptPoolLoader::Load() {
	// Load all entity definitions in this pool
	for (ScriptEntityLoader& entity : entities) {
		if (!entity.Load()) {
			LOG("ScriptPoolLoader::Load: failed to load entity " << entity.GetId());
			return false;
		}
	}

	// Load nested pools
	for (ScriptPoolLoader& pool : pools) {
		if (!pool.Load()) {
			LOG("ScriptPoolLoader::Load: failed to load pool " << pool.GetId());
			return false;
		}
	}

	return true;
}


}
END_UPP_NAMESPACE
