#include "Script.h"

NAMESPACE_UPP
namespace Eon {


ScriptStateLoader::ScriptStateLoader(ScriptChainLoader& parent, int id, Eon::StateDeclaration& def):
	Base(parent, id, def)
{
	
}

String ScriptStateLoader::GetTreeString(int indent) {
	String s;
	s.Cat('\t', indent);
	s << "state " << id.ToString() << "\n";
	return s;
}

bool ScriptStateLoader::Load() {
	id = def.id;
	ScriptLoader& loader = GetLoader();

	if (id.parts.IsEmpty())
		return true;

	String state_leaf = id.parts.Top();
	Vector<String> parent_parts;
	parent_parts <<= id.parts;
	if (!parent_parts.IsEmpty())
		parent_parts.SetCount(parent_parts.GetCount() - 1);

	VfsValue* space_parent = nullptr;
	VfsValue* loop_parent = nullptr;
	Engine* mach = loader.val.FindOwner<Engine>();
	ASSERT(mach);
	if (!mach)
		return false;

	if (parent_parts.IsEmpty()) {
		loop_parent = &mach->GetRootLoop();
		space_parent = &mach->GetRootSpace();
	}
	else {
		Eon::Id loop_id;
		loop_id.parts <<= parent_parts;
		loop_parent = loader.ResolveLoop(loop_id, &space_parent);
		if (!loop_parent) {
			AddError(def.loc, String("Could not resolve state parent loop: ") + loop_id.ToString());
			return false;
		}
	}

	if (!space_parent)
		space_parent = &mach->GetRootSpace();

	EnvState& env = loop_parent->GetAdd<EnvState>(state_leaf);
	space_parent->GetAdd(state_leaf, 0);
	env.SetName(state_leaf);

	return true;
}

bool ScriptStateLoader::PostInitialize() {
	return true;
}


}
END_UPP_NAMESPACE
