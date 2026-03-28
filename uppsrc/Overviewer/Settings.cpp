#include "Settings.h"

void OverviewerSettings::Load() {
	LoadFromJsonFile(*this, ConfigFile("settings.json"));
}

void OverviewerSettings::Save() {
	StoreAsJsonFile(*this, ConfigFile("settings.json"));
}

OverviewerSettings& GetSettings() {
	static OverviewerSettings s;
	static bool loaded = false;
	if(!loaded) {
		s.Load();
		loaded = true;
	}
	return s;
}
