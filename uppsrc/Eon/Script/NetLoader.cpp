#include "Script.h"

NAMESPACE_UPP
namespace Eon {


ScriptNetLoader::ScriptNetLoader(ScriptMachineLoader& parent, int id, Eon::NetDefinition& def) :
	Base(parent, id, def)
{
	// TODO: Load states when we support them with nets
	// For now, states require ScriptChainLoader parent, so we skip them
	// We'll refactor StateLoader to support multiple parent types later
}

bool ScriptNetLoader::Load() {
	RTLOG("ScriptNetLoader::Load() - Building net: " << def.id.ToString());
	RTLOG("\tAtoms: " << def.atoms.GetCount());
	RTLOG("\tStates: " << def.states.GetCount());
	RTLOG("\tConnections: " << def.connections.GetCount());

	// Call ScriptLoader::BuildNet to handle the actual net construction
	// BuildNet validates atom definitions, connections, and creates PacketRouter
	if (!GetLoader().BuildNet(def)) {
		AddError(def.loc, "Failed to build net: " + def.id.ToString());
		return false;
	}

	LOG("ScriptNetLoader::Load() - Net loaded successfully: " << def.id.ToString());
	return true;
}

String ScriptNetLoader::GetTreeString(int indent) {
	String s;
	s.Cat('\t', indent);
	s << "Net " << id << " (" << def.id.ToString() << ")";
	s.Cat('\n');

	// Show atoms
	s.Cat('\t', indent+1);
	s << "Atoms: " << def.atoms.GetCount();
	s.Cat('\n');

	// Show connections
	s.Cat('\t', indent+1);
	s << "Connections: " << def.connections.GetCount();
	s.Cat('\n');

	return s;
}

void ScriptNetLoader::GetLoops(Vector<ScriptLoopLoader*>& v) {
	// Nets don't have loops
}

void ScriptNetLoader::GetStates(Vector<ScriptStateLoader*>& v) {
	// TODO: Add state support when we refactor StateLoader
	// for (ScriptStateLoader& state : states) {
	//     v.Add(&state);
	// }
}

Eon::Id ScriptNetLoader::GetDeepId() const {
	Eon::Id id = parent.GetDeepId();
	id.Append(def.id);
	return id;
}


}
END_UPP_NAMESPACE
