#include "Script.h"

NAMESPACE_UPP
namespace Eon {

ScriptComponentLoader::ScriptComponentLoader(ScriptEntityLoader& parent, int id, Eon::ComponentDefinition& def) :
	Base(parent, id, def) {
	
}

bool ScriptComponentLoader::Load() {
	// Component loader implementation - typically just needs to register itself
	// Components don't have further nested components to load
	return true;
}


}
END_UPP_NAMESPACE
