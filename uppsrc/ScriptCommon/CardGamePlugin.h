#ifndef _ScriptCommon_CardGamePlugin_h_
#define _ScriptCommon_CardGamePlugin_h_

class CardGamePlugin : public IPlugin,
                       public ICustomExecuteProvider,
                       public IPythonBindingProvider {
public:
	CardGamePlugin();
	virtual ~CardGamePlugin();

	// IPlugin
	virtual String GetID() const override { return "CardGamePlugin"; }
	virtual String GetName() const override { return "Card Game Engine"; }
	virtual String GetDescription() const override { return "Provides .gamestate rendering and .form layout support."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	// ICustomExecuteProvider
	virtual bool CanExecute(const String& path) override;
	virtual void Execute(const String& path) override;

	// IPythonBindingProvider
	virtual void SyncBindings(PyVM& vm) override;

	// GUI view — set by ScriptIDE before Execute(); nullptr in headless mode
	void            SetView(IHeartsView* v) { view = v; }
	IPluginContext* GetContext() const      { return context; } PyVM* GetVM() { return context ? context->GetVM() : nullptr; }

protected:
	IPluginContext* context = nullptr;
	IHeartsView*   view    = nullptr;
	String         entry_module_name;

public:
	PyValue         GetGameFunction(const String& name) const;
};

// Strategy bridge vtable — default stubs; PKR registers real implementation.
struct StrategyBridgeAPI {
	bool   (*init)(void*& eval_ptr, void*& strategy_ptr, const String& model_path);
	bool   (*is_ready)(void* strategy_ptr);
	void   (*cleanup)(void* eval_ptr, void* strategy_ptr);
	String (*get_advice)(const Vector<int>& hole, const Vector<int>& board,
	                     int pot, const Vector<byte>& history,
	                     void* strategy_ptr, Vector<double>& out_probs);
};

// Call this once at startup to install the real implementation.
// If never called, strategy_bridge is a no-op stub.
void CardGamePlugin_RegisterStrategyBridge(const StrategyBridgeAPI& api);

#endif
