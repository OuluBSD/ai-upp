#include "Script.h"

NAMESPACE_UPP
namespace Eon {

ScriptEcsSystemLoader::ScriptEcsSystemLoader(ScriptWorldLoader& parent, int id, Eon::EcsSysDefinition& def) :
	Base(parent, id, def) {
	
}

bool ScriptEcsSystemLoader::Load() {
	// ECS System loader implementation - typically just needs to register itself
	// Systems don't have further nested systems to load in this context
	return true;
}

}
END_UPP_NAMESPACE
