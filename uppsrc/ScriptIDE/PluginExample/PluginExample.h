#ifndef _PluginExample_PluginExample_h_
#define _PluginExample_PluginExample_h_

#include <ScriptIDE/ScriptIDE.h>

NAMESPACE_UPP

struct PluginExampleConfig {
	bool enabled = true;
	String message = "PluginExample is active.";

	void Serialize(Stream& s) {
		s % enabled % message;
	}
};

class PluginExamplePage : public ParentCtrl, public IPluginPreferencesPage {
public:
	typedef PluginExamplePage CLASSNAME;

	PluginExamplePage();
	static String ConfigPath();

	virtual Ctrl& GetCtrl() override { return *this; }
	virtual void  LoadConfig() override;
	virtual void  SaveConfig() override;
	virtual void  ApplyConfig(IDEContext& ctx) override;
	virtual void  SetDefaults() override;
	virtual bool  IsModified() const override;

private:
	PluginExampleConfig ReadConfig() const;
	void WriteConfig(const PluginExampleConfig& cfg);

	PluginExampleConfig loaded;
	Option enable_feature;
	Label message_lbl;
	EditString message;
};

class PluginExamplePane : public ParentCtrl {
public:
	typedef PluginExamplePane CLASSNAME;

	PluginExamplePane();

	void SetState(const ScriptRunState& state, const String& message);

private:
	Label title;
	Label state_lbl;
	Label path_lbl;
	Label note_lbl;
};

class PluginExample :
	public IPlugin,
	public IPluginPreferencesProvider,
	public IRunStateListener
{
public:
	PluginExample();

	virtual String GetID() const override { return "scriptide.plugin_example"; }
	virtual String GetName() const override { return "Plugin Example"; }
	virtual String GetDescription() const override { return "Reference package for external-style ScriptIDE plugins."; }
	virtual void   Init(IPluginContext& context) override;
	virtual void   Shutdown() override;

	virtual int    GetPreferencesPageCount() const override { return 1; }
	virtual String GetPreferencesPageCategory(int index) const override;
	virtual String GetPreferencesPageID(int index) const override;
	virtual String GetPreferencesPageTitle(int index) const override;
	virtual Image  GetPreferencesPageIcon(int index) const override;
	virtual IPluginPreferencesPage& GetPreferencesPage(int index) override;

	virtual void OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state) override;

private:
	IPluginContextGUI* ctx = nullptr;
	IPluginRegistryGUI* registry = nullptr;
	PluginExamplePage prefs;
	PluginExamplePane pane;
	String last_message;
};

END_UPP_NAMESPACE

#endif
