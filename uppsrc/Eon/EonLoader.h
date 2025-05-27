#ifndef _Eon_EonLoader_h_
#define _Eon_EonLoader_h_

namespace Eon {

struct ExtScriptEcsLoader : Eon::ExtScriptEcsLoaderBase {
	
	
	
	bool Load(ScriptWorldLoader& l) override;
	bool Load(ScriptEcsSystemLoader& l, Ecs::System& sys);
	bool Load(ScriptPoolLoader& l, Ecs::Pool& pool);
	bool Load(ScriptEntityLoader& l, Ecs::Entity& ent);
	bool Load(ScriptComponentLoader& l, Ecs::ComponentBase& ent);
	
};

}

#endif
