#include "Script.h"

NAMESPACE_UPP
namespace Eon {


ScriptNetLoader::ScriptNetLoader(ScriptMachineLoader& parent, int id, Eon::NetDefinition& def) :
	Base(parent, id, def)
{
	// Load states for the net
	for (int i = 0; i < def.states.GetCount(); i++) {
		states.Add(new ScriptStateLoader(*this, i, def.states[i]));
	}
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
	// Add all state loaders to the vector
	for (ScriptStateLoader& state : states) {
		v.Add(&state);
	}
}

Eon::Id ScriptNetLoader::GetDeepId() const {
	Eon::Id id = parent.GetDeepId();
	id.Append(def.id);
	return id;
}


}
END_UPP_NAMESPACE
