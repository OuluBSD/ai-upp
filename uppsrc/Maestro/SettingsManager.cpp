#include "Maestro.h"

namespace Upp {

SettingsManager::SettingsManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	config_path = AppendFileName(base_path, ".maestro/config.json");
}

ValueMap SettingsManager::LoadSettings()
{
	ValueMap settings;
	if(FileExists(config_path))
		LoadFromJsonFile(settings, config_path);
	return settings;
}

bool SettingsManager::SaveSettings(const ValueMap& settings)
{
	RealizeDirectory(GetFileDirectory(config_path));
	return StoreAsJsonFile(settings, config_path, true);
}

Value SettingsManager::GetSetting(const String& key)
{
	ValueMap settings = LoadSettings();
	return settings.Get(key, Value());
}

bool SettingsManager::SetSetting(const String& key, const Value& value)
{
	ValueMap settings = LoadSettings();
	settings.Set(key, value);
	return SaveSettings(settings);
}

}
