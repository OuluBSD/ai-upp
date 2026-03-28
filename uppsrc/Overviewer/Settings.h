#ifndef _Overviewer_Settings_h_
#define _Overviewer_Settings_h_

#include <Core/Core.h>

using namespace Upp;

struct OverviewerSettings {
	bool autosave_enabled = true;
	int autosave_interval_minutes = 5;
	int backup_mode = 0; // 0: alongside project, 1: recovery dir
	bool restore_layout = true;
	int max_backups = 10;

	void Jsonize(JsonIO& jio) {
		jio("autosave_enabled", autosave_enabled)
		   ("autosave_interval_minutes", autosave_interval_minutes)
		   ("backup_mode", backup_mode)
		   ("restore_layout", restore_layout)
		   ("max_backups", max_backups);
	}

	void Load();
	void Save();
};

OverviewerSettings& GetSettings();

#endif
