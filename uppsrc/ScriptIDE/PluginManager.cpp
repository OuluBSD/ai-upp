#include "ScriptIDE.h"

NAMESPACE_UPP

PluginManager::PluginManager(PythonIDE& ide) : ide(&ide)
{
}

PluginManager::~PluginManager()
{
}

void PluginManager::LoadPlugins()
{
	ClearRegistry();
	for(auto& f : GetInternalPluginFactories()) {
		One<IPlugin> p(f());
		String id = p->GetID();
		plugins.GetAdd(id).plugin = pick(p);
	}
}

void PluginManager::EnablePlugin(const String& id, bool enable)
{
	int q = plugins.Find(id);
	if(q >= 0) {
		PluginInstance& pi = plugins[q];
		if(enable && !pi.enabled) {
			pi.enabled = true;
			pi.plugin->Init(*this);
		}
		else if(!enable && pi.enabled) {
			pi.plugin->Shutdown();
			pi.enabled = false;
		}
	}
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
	if(ide) {
		DockableCtrl& dc = ide->plugin_panes.GetAdd(id);
		dc.Title(title);
		if(!ctrl.GetParent())
			dc.Add(ctrl.SizePos());
		ide->SyncPluginPanes();
	}
}

void PluginManager::UnregisterDockPane(const String& id)
{
	if(ide) {
		int q = ide->plugin_panes.Find(id);
		if(q >= 0) {
			ide->plugin_panes[q].Remove();
			ide->plugin_panes.Remove(q);
		}
	}
}

IFileTypeHandler* PluginManager::FindFileTypeHandler(const String& path)
{
	IFileTypeHandler* viewer = nullptr;
	for(int i = file_handlers.GetCount() - 1; i >= 0; --i) {
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

Vector<IDockPaneProvider*> PluginManager::GetDockPaneProviders()
{
	return pick(Vector<IDockPaneProvider*>(pane_providers, 0));
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
	if(!ide) return;
	ScriptRunState st;
	st.running = ide->run_manager.IsRunning();
	st.mode = IDocumentHost::RUNMODE_RUN;
	if(ide->run_manager.GetMode() == RunManager::RUN_DEBUG) st.mode = IDocumentHost::RUNMODE_DEBUG;
	if(ide->run_manager.GetMode() == RunManager::RUN_PROFILE) st.mode = IDocumentHost::RUNMODE_PROFILE;
	
	if(ide->active_file >= 0 && ide->active_file < ide->open_files.GetCount()) {
		st.path = ide->open_files[ide->active_file].path;
		st.host = ide->open_files[ide->active_file].editor;
	}
	
	for(int i = 0; i < run_state_listeners.GetCount(); i++)
		run_state_listeners[i]->OnRunStateChanged(*ide, st);
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
