#include "ScriptIDE.h"

NAMESPACE_UPP

PluginManager::PluginManager(PythonIDE& ide) : ide(&ide)
{
}

PluginManager::~PluginManager()
{
	for(int i = 0; i < plugins.GetCount(); i++) {
		if(plugins[i].enabled)
			plugins[i].plugin->Shutdown();
	}
}

void PluginManager::LoadPlugins()
{
	plugins.Clear();
	Vector<PluginFactory>& factories = GetInternalPluginFactories();
	for(int i = 0; i < factories.GetCount(); i++) {
		One<IPlugin> p(factories[i]());
		if(p) {
			String id = p->GetID();
			plugins.Add(id).plugin = std::move(p);
		}
	}
}

void PluginManager::EnablePlugin(const String& id, bool enable)
{
	int q = plugins.Find(id);
	if(q < 0) return;
	
	PluginInstance& inst = plugins[q];
	if(inst.enabled == enable) return;
	
	if(enable) {
		ClearRegistry();
		inst.plugin->Init(*this);
		inst.enabled = true;
	}
	else {
		inst.plugin->Shutdown();
		inst.enabled = false;
		ClearRegistry();
		// Re-init other enabled plugins to rebuild registry
		for(int i = 0; i < plugins.GetCount(); i++) {
			if(plugins[i].enabled)
				plugins[i].plugin->Init(*this);
		}
	}
	
	ide->SyncPluginPanes();
}

bool PluginManager::IsPluginEnabled(const String& id) const
{
	int q = plugins.Find(id);
	return q >= 0 && plugins[q].enabled;
}

PyVM* PluginManager::GetVM()
{
	return ide ? &ide->vm : nullptr;
}

void PluginManager::RegisterDockPane(const String& id, const String& title, Ctrl& ctrl)
{
	if(!ide) return;
	if(ide->plugin_panes.Find(id) >= 0) return;
	
	One<DockableCtrl>& odc = ide->plugin_panes.Add(id);
	odc.Create();
	odc->Title(title);
	odc->Add(ctrl.SizePos());
	
	ide->SyncPluginPanes();
}

void PluginManager::UnregisterDockPane(const String& id)
{
	if(!ide) return;
	ide->plugin_panes.RemoveKey(id);
	ide->SyncPluginPanes();
}

IFileTypeHandler* PluginManager::FindFileTypeHandler(const String& ext)
{
	for(int i = 0; i < file_handlers.GetCount(); i++) {
		if(file_handlers[i].GetExtension() == ext)
			return &file_handlers[i];
	}
	return nullptr;
}

Array<IDockPaneProvider*> PluginManager::GetDockPaneProviders()
{
	Array<IDockPaneProvider*> res;
	for(int i = 0; i < pane_providers.GetCount(); i++)
		res.Add(&pane_providers[i]);
	return res;
}

void PluginManager::SyncBindings(PyVM& vm)
{
	for(int i = 0; i < binding_providers.GetCount(); i++)
		binding_providers[i].SyncBindings(vm);
}

ICustomExecuteProvider* PluginManager::FindCustomExecuteProvider(const String& path)
{
	for(int i = 0; i < execute_providers.GetCount(); i++) {
		if(execute_providers[i].CanExecute(path))
			return &execute_providers[i];
	}
	return nullptr;
}

void PluginManager::ClearRegistry()
{
	file_handlers.Clear();
	pane_providers.Clear();
	binding_providers.Clear();
	execute_providers.Clear();
}

Vector<PluginFactory>& GetInternalPluginFactories()
{
	static Vector<PluginFactory> factories;
	return factories;
}

END_UPP_NAMESPACE
