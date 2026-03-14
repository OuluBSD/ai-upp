#include "PluginExample.h"

NAMESPACE_UPP

String PluginExamplePage::ConfigPath()
{
	return ConfigFile("pluginexample.bin");
}

PluginExamplePage::PluginExamplePage()
{
	Add(enable_feature.LeftPosZ(8, 220).TopPosZ(8, 20));
	enable_feature.SetLabel("Enable example feature");
	Add(message_lbl.LeftPosZ(8, 120).TopPosZ(36, 20));
	message_lbl.SetLabel("Status message:");
	Add(message.HSizePosZ(136, 8).TopPosZ(36, 20));
}

PluginExampleConfig PluginExamplePage::ReadConfig() const
{
	PluginExampleConfig cfg;
	cfg.enabled = enable_feature;
	cfg.message = message.GetData();
	return cfg;
}

void PluginExamplePage::WriteConfig(const PluginExampleConfig& cfg)
{
	enable_feature = cfg.enabled;
	message.SetData(cfg.message);
}

void PluginExamplePage::LoadConfig()
{
	loaded = PluginExampleConfig();
	LoadFromFile(loaded, ConfigPath());
	WriteConfig(loaded);
}

void PluginExamplePage::SaveConfig()
{
	loaded = ReadConfig();
	StoreToFile(loaded, ConfigPath());
}

void PluginExamplePage::ApplyConfig(IDEContext& ctx)
{
	if(ctx.main_window)
		ctx.main_window->Log("PluginExample preferences applied.");
}

void PluginExamplePage::SetDefaults()
{
	WriteConfig(PluginExampleConfig());
}

bool PluginExamplePage::IsModified() const
{
	PluginExampleConfig cfg = ReadConfig();
	StringStream a, b;
	cfg.Serialize(a);
	const_cast<PluginExampleConfig&>(loaded).Serialize(b);
	return a.GetResult() != b.GetResult();
}

PluginExamplePane::PluginExamplePane()
{
	Add(title.LeftPosZ(8, 220).TopPosZ(8, 20));
	title.SetLabel("Plugin Example");
	title.SetFont(StdFont().Bold());
	Add(state_lbl.HSizePosZ(8, 8).TopPosZ(36, 20));
	Add(path_lbl.HSizePosZ(8, 8).TopPosZ(62, 36));
	Add(note_lbl.HSizePosZ(8, 8).VSizePosZ(106, 8));
	note_lbl.SetFrame(InsetFrame());
}

void PluginExamplePane::SetState(const ScriptRunState& state, const String& message)
{
	String run_state = "Idle";
	if(state.running)
		run_state = state.paused ? "Paused" : "Running";
	state_lbl.SetLabel("State: " + run_state);
	path_lbl.SetLabel("Path: " + (state.path.IsEmpty() ? String("<none>") : state.path));
	note_lbl.SetLabel(message);
}

PluginExample::PluginExample()
{
	last_message = "Reference plugin loaded.";
}

void PluginExample::Init(IPluginContext& context)
{
	ctx = dynamic_cast<IPluginContextGUI*>(&context);
	registry = dynamic_cast<IPluginRegistryGUI*>(&context);
	if(!ctx || !registry)
		return;

	registry->RegisterPreferencesProvider(*this);
	registry->RegisterRunStateListener(*this);
	ctx->RegisterDockPane("plugin.example", "Plugin Example", pane);
	pane.SetState(ScriptRunState(), last_message);
	ctx->GetIDE().Log("PluginExample initialized.");
}

void PluginExample::Shutdown()
{
	if(ctx)
		ctx->UnregisterDockPane("plugin.example");
	ctx = nullptr;
	registry = nullptr;
}

String PluginExample::GetPreferencesPageCategory(int index) const
{
	ASSERT(index == 0);
	return "Plugins";
}

String PluginExample::GetPreferencesPageID(int index) const
{
	ASSERT(index == 0);
	return "plugin_example";
}

String PluginExample::GetPreferencesPageTitle(int index) const
{
	ASSERT(index == 0);
	return "Plugin example";
}

Image PluginExample::GetPreferencesPageIcon(int index) const
{
	ASSERT(index == 0);
	return Icons::Help();
}

IPluginPreferencesPage& PluginExample::GetPreferencesPage(int index)
{
	ASSERT(index == 0);
	return prefs;
}

void PluginExample::OnRunStateChanged(PythonIDE& ide, const ScriptRunState& state)
{
	PluginExampleConfig cfg;
	LoadFromFile(cfg, PluginExamplePage::ConfigPath());
	last_message = cfg.message;
	pane.SetState(state, last_message);
	if(cfg.enabled && state.running)
		ide.Log("PluginExample observed a running document.");
}

END_UPP_NAMESPACE
