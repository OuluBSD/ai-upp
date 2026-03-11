#include "ScriptCommon.h"

NAMESPACE_UPP

Vector<PluginFactory>& GetInternalPluginFactories()
{
	static Vector<PluginFactory> factories;
	return factories;
}

void HeadlessPluginContext::SyncBindings()
{
	if(!vm) return;
	for(auto* p : binding_providers)
		p->SyncBindings(*vm);
}

ICustomExecuteProvider* HeadlessPluginContext::FindExecuteProvider(const String& path)
{
	for(auto* p : execute_providers)
		if(p->CanExecute(path))
			return p;
	return nullptr;
}

END_UPP_NAMESPACE
