#include "Maintenance.h"

NAMESPACE_UPP

MaintenancePane::MaintenancePane() {
	Add(chat.SizePos());
}

void MaintenancePane::Load(const String& root) {
	chat.backend = "gemini";
	chat.engine.working_dir = root;
}

END_UPP_NAMESPACE
