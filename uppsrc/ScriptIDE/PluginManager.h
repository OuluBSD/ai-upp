#ifndef _ScriptIDE_PluginManager_h_
#define _ScriptIDE_PluginManager_h_

class PluginManager : public IPluginContext, public IPluginRegistry {
	struct PluginInstance {
		One<IPlugin> plugin;
		bool enabled = false;
	};

	ArrayMap<String, PluginInstance> plugins;
	PythonIDE* ide = nullptr;

	Array<IFileTypeHandler> file_handlers;
	Array<IDockPaneProvider> pane_providers;
	Array<IPythonBindingProvider> binding_providers;
	Array<ICustomExecuteProvider> execute_providers;

public:
	PluginManager(PythonIDE& ide);
	~PluginManager();

	void LoadPlugins();
	void EnablePlugin(const String& id, bool enable = true);
	void DisablePlugin(const String& id) { EnablePlugin(id, false); }
	bool IsPluginEnabled(const String& id) const;

	const ArrayMap<String, PluginInstance>& GetPlugins() const { return plugins; }

	// IPluginContext
	virtual PythonIDE& GetIDE() override { ASSERT(ide); return *ide; }
	virtual PyVM*      GetVM() override;
	virtual void       RegisterDockPane(const String& id, const String& title, Ctrl& ctrl) override;
	virtual void       UnregisterDockPane(const String& id) override;

	// IPluginRegistry
	virtual void RegisterFileTypeHandler(IFileTypeHandler& h) override { file_handlers.Add(&h); }
	virtual void RegisterDockPaneProvider(IDockPaneProvider& p) override { pane_providers.Add(&p); }
	virtual void RegisterPythonBindingProvider(IPythonBindingProvider& p) override { binding_providers.Add(&p); }
	virtual void RegisterCustomExecuteProvider(ICustomExecuteProvider& p) override { execute_providers.Add(&p); }

	// Dispatchers
	IFileTypeHandler*      FindFileTypeHandler(const String& ext);
	Array<IDockPaneProvider*> GetDockPaneProviders();
	void                   SyncBindings(PyVM& vm);
	ICustomExecuteProvider* FindCustomExecuteProvider(const String& path);

private:
	void ClearRegistry();
};

#endif
