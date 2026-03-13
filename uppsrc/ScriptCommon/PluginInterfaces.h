#ifndef _ScriptCommon_PluginInterfaces_h_
#define _ScriptCommon_PluginInterfaces_h_

// No includes here - they are in ScriptCommon.h or Main package header

class PyVM;

class IPythonBindingProvider {
public:
	virtual ~IPythonBindingProvider() {}
	virtual void SyncBindings(PyVM& vm) = 0;
};

class ICustomExecuteProvider {
public:
	virtual ~ICustomExecuteProvider() {}
	virtual bool CanExecute(const String& path) = 0;
	virtual void Execute(const String& path) = 0;
};

class IPluginRegistry {
public:
	virtual ~IPluginRegistry() {}
	virtual void RegisterPythonBindingProvider(IPythonBindingProvider& provider) = 0;
	virtual void RegisterCustomExecuteProvider(ICustomExecuteProvider& provider) = 0;
};

class IPluginContext : public IPluginRegistry {
public:
	virtual ~IPluginContext() {}
	virtual PyVM* GetVM() = 0;
};

class HeadlessPluginContext : public IPluginContext {
	PyVM* vm;
	Vector<IPythonBindingProvider*> binding_providers;
	Vector<ICustomExecuteProvider*> execute_providers;

public:
	HeadlessPluginContext(PyVM& vm) : vm(&vm) {}

	virtual PyVM* GetVM() override { return vm; }

	// IPluginRegistry
	virtual void RegisterPythonBindingProvider(IPythonBindingProvider& p) override { binding_providers.Add(&p); }
	virtual void RegisterCustomExecuteProvider(ICustomExecuteProvider& p) override { execute_providers.Add(&p); }

	void SyncBindings();
	ICustomExecuteProvider* FindExecuteProvider(const String& path);
};

// View interface for card game UI — implemented by CardGameDocumentHost in ScriptIDE.
// ScriptCommon holds only this interface (no GUI dependency).
class IHeartsView {
public:
	virtual ~IHeartsView() {}
	// Place/update a card sprite at absolute pixel position
	virtual void SetCard(const String& card_id, const String& asset_path, int x, int y, int rotation_deg = 0) = 0;
	// Move card sprite to the center of a named zone
	virtual void MoveCardToZone(const String& card_id, const String& zone_id, int offset, bool animated) = 0;
	// Return zone rect as a dict-like Value {x,y,w,h}
	virtual Value GetZoneRect(const String& zone_id) = 0;
	// Clear all card sprites
	virtual void ClearSprites() = 0;
	// Remove a single card sprite
	virtual void RemoveSprite(const String& card_id) = 0;
	// Set expected rendered sprite count for a named zone; host asserts after UI sync
	virtual void SetExpectedSpriteCount(const String& zone_id, int count) = 0;
	// Set a text label inside a named zone
	virtual void SetLabel(const String& zone_id, const String& text) = 0;
	// Set a clickable button inside a named zone
	virtual void SetButton(const String& zone_id, const String& text, bool enabled) = 0;
	// Highlight or clear a named zone
	virtual void SetHighlight(const String& zone_id, bool enabled) = 0;
	// Set transient status text shown on the table
	virtual void SetStatus(const String& text) = 0;
	// Log a message to the game log
	virtual void Log(const String& msg) = 0;
	// Schedule a named Python callback after a delay
	virtual void SetTimeout(int delay_ms, const String& callback_name) = 0;
};

class IPlugin {
public:
	virtual ~IPlugin() {}
	virtual String GetID() const = 0;
	virtual String GetName() const = 0;
	virtual String GetDescription() const = 0;
	virtual void   Init(IPluginContext& context) = 0;
	virtual void   Shutdown() = 0;
};

typedef IPlugin* (*PluginFactory)();

Vector<PluginFactory>& GetInternalPluginFactories();

#define REGISTER_PLUGIN(T) \
static IPlugin* T##_Factory() { return new T(); } \
static bool T##_Registered = []() { \
    GetInternalPluginFactories().Add(&T##_Factory); \
    return true; \
}();

#endif
