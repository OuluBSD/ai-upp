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
			int q = plugins.Find(id);
			if(q >= 0)
				plugins[q].plugin = std::move(p); // later registration wins (GUI subclass overrides headless base)
			else
				plugins.Add(id).plugin = std::move(p);
		}
	}

	// ScriptIDE must use the GUI-capable built-in plugins, not the headless
	// ScriptCommon variants that share the same plugin IDs.
	auto force_gui_plugin = [this](One<IPlugin> p) {
		if(!p)
			return;
		String id = p->GetID();
		int q = plugins.Find(id);
		if(q >= 0)
			plugins[q].plugin = std::move(p);
		else
			plugins.Add(id).plugin = std::move(p);
	};
	force_gui_plugin(One<IPlugin>(new GameStatePluginGUI()));
	force_gui_plugin(One<IPlugin>(new CardGamePluginGUI()));
}

void PluginManager::EnablePlugin(const String& id, bool enable)
{
	int q = plugins.Find(id);
	if(q < 0) return;
	
	PluginInstance& inst = plugins[q];
	if(inst.enabled == enable) return;
	
	if(enable) {
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
	
	DockableCtrl& odc = ide->plugin_panes.Add(id);
	odc.Title(title);
	odc.Add(ctrl.SizePos());
	
	ide->SyncPluginPanes();
}

void PluginManager::UnregisterDockPane(const String& id)
{
	if(!ide) return;
	ide->plugin_panes.RemoveKey(id);
	ide->SyncPluginPanes();
}

IFileTypeHandler* PluginManager::FindFileTypeHandler(const String& path)
{
	for(int i = file_handlers.GetCount() - 1; i >= 0; --i) {
		if(file_handlers[i]->CanHandle(path))
			return file_handlers[i];
	}
	return nullptr;
}

void PluginManager::SyncBindings(PyVM& vm)
{
	for(int i = 0; i < binding_providers.GetCount(); i++)
		binding_providers[i]->SyncBindings(vm);
}

ICustomExecuteProvider* PluginManager::FindCustomExecuteProvider(const String& path)
{
	for(int i = execute_providers.GetCount() - 1; i >= 0; --i) {
		if(execute_providers[i]->CanExecute(path))
			return execute_providers[i];
	}
	return nullptr;
}

void PluginManager::NotifyRunStateChanged()
{
	if(!ide)
		return;

	ScriptRunState state;
	if(ide->active_file >= 0 && ide->active_file < ide->open_files.GetCount()) {
		state.path = ide->open_files[ide->active_file].path;
		state.host = ide->open_files[ide->active_file].editor;
	}

	if(state.host && state.host->CanRun()) {
		state.can_run = true;
		state.running = state.host->IsRunning();
		state.paused = state.host->IsPaused();
		state.mode = state.host->GetRunMode();
	}
	else if(ide->run_manager.IsRunning()) {
		state.running = true;
		switch(ide->run_manager.GetMode()) {
		case RunManager::RUN_DEBUG:   state.mode = IDocumentHost::RUNMODE_DEBUG;   break;
		case RunManager::RUN_PROFILE: state.mode = IDocumentHost::RUNMODE_PROFILE; break;
		default:                      state.mode = IDocumentHost::RUNMODE_RUN;     break;
		}
	}

	for(int i = 0; i < run_state_listeners.GetCount(); i++)
		run_state_listeners[i]->OnRunStateChanged(*ide, state);
}

void PluginManager::ClearRegistry()
{
	file_handlers.Clear();
	pane_providers.Clear();
	preferences_providers.Clear();
	run_state_listeners.Clear();
	binding_providers.Clear();
	execute_providers.Clear();
}

END_UPP_NAMESPACE
