#include "MaestroHub.h"

NAMESPACE_UPP

ConfigurationDialog::ConfigurationDialog() {
	CtrlLayoutOKCancel(*this, "Maestro Settings");
	
	settings_grid.AddColumn("Key").FixedWidth(200);
	settings_grid.AddColumn("Value").Edit(value_editor);
}

void ConfigurationDialog::Load(const String& maestro_root) {
	root = maestro_root;
	sm.Create(root);
	
	ValueMap settings = sm->LoadSettings();
	settings_grid.Clear();
	for(int i = 0; i < settings.GetCount(); i++)
		settings_grid.Add(settings.GetKey(i), settings.GetValue(i));
}

void ConfigurationDialog::Save() {
	ValueMap settings;
	for(int i = 0; i < settings_grid.GetCount(); i++)
		settings.Add(settings_grid.Get(i, 0), settings_grid.Get(i, 1));
	
	if(sm->SaveSettings(settings))
		PromptOK("Settings saved successfully.");
	else
		Exclamation("Failed to save settings.");
}

END_UPP_NAMESPACE