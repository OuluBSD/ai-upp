#ifndef _Aria_PluginManager_h_
#define _Aria_PluginManager_h_

class BaseAIProvider {
public:
	virtual ~BaseAIProvider() {}
	virtual String Generate(const String& prompt, const String& context = "", const String& output_format = "text") = 0;
};

class BasePlugin {
public:
	virtual ~BasePlugin() {}
	virtual void OnLoad() {}
	virtual VectorMap<String, BaseAIProvider*> GetAIProviders() { return VectorMap<String, BaseAIProvider*>(); }
};

class PluginManager {
	Vector<One<BasePlugin>> plugins;
	VectorMap<String, BaseAIProvider*> ai_providers;
	String plugins_dir;
public:
	PluginManager(const String& plugins_dir = "");
	
	void LoadPlugins();
	void RegisterPlugin(BasePlugin* plugin);
	
	BaseAIProvider* GetAIProvider(const String& name);
	Vector<String> ListAIProviders() const;
	
	void TriggerHook(const String& hook_name, const ValueMap& args);
};

#endif
