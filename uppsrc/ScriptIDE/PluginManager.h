#ifndef _ScriptIDE_PluginManager_h_
#define _ScriptIDE_PluginManager_h_

#include "PluginInterfacesGUI.h"

class PluginManager : public IPluginContextGUI, public IPluginRegistryGUI {
	struct PluginInstance {
		One<IPlugin> plugin;
		bool enabled = false;
	};

	ArrayMap<String, PluginInstance> plugins;
	PythonIDE* ide = nullptr;

	Vector<IFileTypeHandler*> file_handlers;
	Vector<IDockPaneProvider*> pane_providers;
	Vector<IPythonBindingProvider*> binding_providers;
	Vector<ICustomExecuteProvider*> execute_providers;

public:
	PluginManager(PythonIDE& ide);
	~PluginManager();

	void LoadPlugins();
	void EnablePlugin(const String& id, bool enable = true);
	void DisablePlugin(const String& id) { EnablePlugin(id, false); }
	bool IsPluginEnabled(const String& id) const;

	const ArrayMap<String, PluginInstance>& GetPlugins() const { return plugins; }

	// IPluginContextGUI
	virtual PythonIDE& GetIDE() override { ASSERT(ide); return *ide; }
	virtual PyVM*      GetVM() override;
	virtual void       RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) override;
	virtual void       UnregisterDockPane(const String& id) override;

	// IPluginRegistryGUI
	virtual void RegisterFileTypeHandler(IFileTypeHandler& h) override { file_handlers.Add(&h); }
	virtual void RegisterDockPaneProvider(IDockPaneProvider& p) override { pane_providers.Add(&p); }
	
	// IPluginRegistry (from IPluginRegistryGUI)
	virtual void RegisterPythonBindingProvider(IPythonBindingProvider& p) override { binding_providers.Add(&p); }
	virtual void RegisterCustomExecuteProvider(ICustomExecuteProvider& p) override { execute_providers.Add(&p); }

	// Dispatchers
	IFileTypeHandler*      FindFileTypeHandler(const String& ext);
	Vector<IDockPaneProvider*> GetDockPaneProviders();
	void                   SyncBindings(PyVM& vm);
	ICustomExecuteProvider* FindCustomExecuteProvider(const String& path);

private:
	void ClearRegistry();
};

#endif
