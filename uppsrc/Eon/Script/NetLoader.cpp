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
	// TODO: Implement net loading
	// This will:
	// 1. Load inline atom definitions from def.atoms
	// 2. Parse connection specifications from def.connections
	// 3. Create PacketRouter instance
	// 4. Register ports from atoms
	// 5. Wire connections via PacketRouter
	LOG("ScriptNetLoader::Load() - Net loader stub for: " << def.id.ToString());
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
