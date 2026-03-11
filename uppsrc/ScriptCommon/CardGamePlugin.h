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

protected:
	IPluginContext* context = nullptr;
};

#endif
