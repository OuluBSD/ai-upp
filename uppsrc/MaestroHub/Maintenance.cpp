#include "Maintenance.h"

NAMESPACE_UPP

MaintenancePane::MaintenancePane() {
	Add(active_info.TopPos(0, 20).HSizePos(5, 5));
	Add(chat.TopPos(25).BottomPos(0).HSizePos());
	
	active_info.SetFont(StdFont().Bold());
	active_info.SetText("No active session");
}

void MaintenancePane::Load(const String& root) {
	// If AIChatCtrl doesn't have Load, we might need to set its root via another method or ignore
	// Let's assume it has it based on context or we need to add it.
}

void MaintenancePane::SessionStatus(const String& backend, const String& session_id) {
	active_info.SetText("Active Session: [" + backend + "] " + session_id);
}

END_UPP_NAMESPACE
