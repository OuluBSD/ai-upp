#include "FleetDashboard.h"

NAMESPACE_UPP

FleetDashboard::FleetDashboard() {
	CtrlLayout(*this);
	
	project_grid.AddColumn("Project", 200);
	project_grid.AddColumn("Status", 100);
	project_grid.AddColumn("Active Tasks", 100);
	project_grid.AddColumn("Health", 100);
	
	automation_queue.AddColumn("Time", 100);
	automation_queue.AddColumn("Project", 150);
	automation_queue.AddColumn("Action", 200);
	automation_queue.AddColumn("Progress", 150);
	
	vsplit.Vert(project_grid, automation_queue);
	vsplit.SetPos(6000); // 60% projects, 40% queue
}

void FleetDashboard::LoadProjects(const Vector<String>& paths) {
	project_grid.Clear();
	int root = project_grid.Add(0, CtrlImg::Dir(), "Active Workspace");
	
	for(const auto& p : paths) {
		int id = project_grid.Add(root, CtrlImg::Dir(), GetFileName(p));
		project_grid.SetRowValue(id, 1, "Monitored");
		project_grid.SetRowValue(id, 2, "3 Tasks");
		project_grid.SetRowValue(id, 3, "OK");
	}
	
	project_grid.Open(root);
}

void FleetDashboard::UpdateQueue() {
	automation_queue.Clear();
	// Stub automation data
	automation_queue.Add(GetSysTime(), "MegaFileUtil", "Log Scan", "[====------] 40%");
	automation_queue.Add(GetSysTime(), "Project X", "Triage Wizard", "Waiting for User");
}

END_UPP_NAMESPACE
