#include "Script.h"

NAMESPACE_UPP
namespace Eon {

ScriptEntityLoader::ScriptEntityLoader(ScriptPoolLoader& parent, int id, Eon::EntityDefinition& def) :
	Base(parent, id, def) {
	
	for (Eon::ComponentDefinition& sys : def.comps) {
		comps.Add(new ScriptComponentLoader(*this, comps.GetCount(), sys));
	}
	
}

bool ScriptEntityLoader::Load() {
	// Load all component definitions in this entity
	for (ScriptComponentLoader* comp : comps) {
		if (!comp->Load()) {
			LOG("ScriptEntityLoader::Load: failed to load component " << comp->GetId());
			return false;
		}
	}

	return true;
}


}
END_UPP_NAMESPACE
