#ifndef _ScriptCommon_GameStatePlugin_h_
#define _ScriptCommon_GameStatePlugin_h_

class GameStatePlugin : public IPlugin, 
                        public IPythonBindingProvider, 
                        public ICustomExecuteProvider {
public:
	GameStatePlugin();
	virtual ~GameStatePlugin();

	// IPlugin
	virtual String GetID() const override { return "GameStatePlugin"; }
	virtual String GetName() const override { return "Game State Viewer"; }
	virtual String GetDescription() const override { return "Provides .gamestate visualization and simulation."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	// IPythonBindingProvider
	virtual void SyncBindings(PyVM& vm) override;

	// ICustomExecuteProvider
	virtual bool CanExecute(const String& path) override;
	virtual void Execute(const String& path) override;

protected:
	IPluginContext* context = nullptr;
	virtual void OnUpdateStats(const String& json) {}

private:
	static PyValue GetScore(const Vector<PyValue>& args, void* user_data);
};

#endif
