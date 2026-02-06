#include "MaestroHub.h"

NAMESPACE_UPP

MaintenancePane::MaintenancePane() {
	Add(chat.SizePos());
	Add(active_info.BottomPos(0, 20).HSizePos());
	active_info.SetAlign(ALIGN_CENTER);
}

void MaintenancePane::Load(const String& maestro_root) {
	root = maestro_root;
	chat.Load(root);
}

void MaintenancePane::SessionStatus(const String& backend, const String& session_id) {
	active_info.SetLabel("Active Session: " + session_id + " [" + backend + "]");
}

END_UPP_NAMESPACE