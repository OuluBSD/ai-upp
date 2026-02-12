#ifndef _Maestro_Plugin_h_
#define _Maestro_Plugin_h_

class MaestroPlugin {
public:
	virtual ~MaestroPlugin() {}
	virtual String GetName() const = 0;
	virtual void   RegisterTools(MaestroToolRegistry& reg) {}
	virtual void   RegisterViews(TabCtrl& tabs) {}
	virtual void   RegisterMenu(Bar& menu) {}
};

class PluginManager {
	Array<MaestroPlugin> plugins;
public:
	void LoadPlugins(const String& maestro_root);
	void RegisterAll(MaestroToolRegistry& reg, TabCtrl& tabs);
	void RegisterMenu(Bar& bar);
	void AddPlugin(MaestroPlugin* p) { plugins.Add(p); }
	
	static PluginManager& Get();
};

#endif
