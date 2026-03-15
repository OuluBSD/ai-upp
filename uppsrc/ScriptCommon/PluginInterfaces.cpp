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

IFileTypeHandler* HeadlessPluginContext::FindFileTypeHandler(const String& path)
{
	IFileTypeHandler* viewer = nullptr;
	for(int i = file_handlers.GetCount() - 1; i >= 0; i--) {
		IFileTypeHandler* handler = file_handlers[i];
		if(!handler->CanHandle(path))
			continue;
		if(handler->SupportsHostRole(IFileTypeHandler::HOSTROLE_EDITOR))
			return handler;
		if(!viewer && handler->SupportsHostRole(IFileTypeHandler::HOSTROLE_VIEWER))
			viewer = handler;
	}
	return viewer;
}

END_UPP_NAMESPACE
