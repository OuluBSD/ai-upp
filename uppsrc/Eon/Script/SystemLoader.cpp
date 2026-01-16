#include "Script.h"

NAMESPACE_UPP
namespace Eon {

ScriptSystemLoader::ScriptSystemLoader(ScriptLoader& parent, int id, Eon::GlobalScope& def) :
	Base(parent, id, def)
{
	LOG("ScriptSystemLoader::ctor: def.worlds.GetCount()=" << def.worlds.GetCount() << ", def.machs.GetCount()=" << def.machs.GetCount());

	for (Eon::MachineDefinition& mach : def.machs) {
		ScriptMachineLoader& loader = machs.Add(new ScriptMachineLoader(*this, machs.GetCount(), mach));
	}

	for (Eon::WorldDefinition& world : def.worlds) {
		LOG("ScriptSystemLoader::ctor: creating ScriptWorldLoader for world.id=" << world.id.ToString());
		ScriptWorldLoader& loader = worlds.Add(new ScriptWorldLoader(*this, worlds.GetCount(), world));
	}
	LOG("ScriptSystemLoader::ctor: done, this->worlds.GetCount()=" << worlds.GetCount());
}

bool ScriptSystemLoader::Load() {
	for (auto& loader : machs) {
		if (!loader.Load())
			return false;
	}
	for (auto& loader : worlds) {
		if (!loader.Load())
			return false;
	}
	
	return true;
}

bool ScriptSystemLoader::LoadEcs() {
	LOG("ScriptSystemLoader::LoadEcs: called, worlds.GetCount()=" << worlds.GetCount());

	if (worlds.GetCount() > 1) {
		AddError(def.loc, "Only one world is supported currently, and script got " + IntStr(worlds.GetCount()));
		return false;
	}

	for (ScriptWorldLoader& e : worlds) {
		LOG("ScriptSystemLoader::LoadEcs: loading world, __ecs_script_loader=" << (void*)__ecs_script_loader);
		if (!__ecs_script_loader) {
			LOG("ScriptSystemLoader::LoadEcs: error: no ecs script loader present");
			AddError(def.loc, "no ecs script loader present in system");
			return false;
		}

		LOG("ScriptSystemLoader::LoadEcs: calling __ecs_script_loader->Load()");
		if (!__ecs_script_loader->Load(e)) {
			LOG("ScriptSystemLoader::LoadEcs: error: " << __ecs_script_loader->GetErrorString());
			AddError(def.loc, __ecs_script_loader->GetErrorString());
			return false;
		}
		LOG("ScriptSystemLoader::LoadEcs: world loaded successfully");
	}

	LOG("ScriptSystemLoader::LoadEcs: completed successfully");
	return true;
}

String ScriptSystemLoader::GetTreeString(int indent) {
	String s;
	s.Cat('\t', indent);
	s << "System " << id;
	s.Cat('\n');
	
	for (auto& loader : machs) {
		s << loader.GetTreeString(indent+1);
	}
	for (auto& loader : worlds) {
		s << loader.GetTreeString(indent+1);
	}
	
	return s;
}

void ScriptSystemLoader::GetLoops(Vector<ScriptLoopLoader*>& v) {
	for (auto& loader : machs) {
		loader.GetLoops(v);
	}
}

void ScriptSystemLoader::GetStates(Vector<ScriptStateLoader*>& v) {
	for (auto& loader : machs) {
		loader.GetStates(v);
	}
}


}
END_UPP_NAMESPACE
