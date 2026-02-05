#ifndef _MaestroHub_MaestroHub_h_
#define _MaestroHub_MaestroHub_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

#include "Technology.h"
#include "Product.h"
#include "Maintenance.h"
#include "IssuesView.h"
#include "WorkDashboard.h"
#include "SessionManagement.h"

NAMESPACE_UPP

class MaestroHub : public TopWindow {
	MenuBar menu;
	TabCtrl tabs;
	
	// Pointers to avoid circular include issues during refactor
	One<TechnologyPane>        technology;
	One<ProductPane>           product;
	One<MaintenancePane>       maintenance;
	One<IssuesPane>            issues;
	One<WorkPane>              work;
	One<SessionManagementPane> sessions;
	
	RecentConfig config;
	String       current_root;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void SelectRoot();
	
	void OnEnact(String track, String phase, String task);
	void OnEnactStep(String runbook_title, int step_n, String instruction);
	void OnSessionSelect(String backend, String session_id);
	void OnSuggestEnact();
	void OnTriageWizard();
	void OnRunbookEditor();
	void OnStateEditor();
	void OnTUBrowser();
	void OnLogAnalyzer();
	void OnSettings();
	void OnOpsRunner();
	
	void LoadData();
	void PlanWatcher();
	void ScanForUnblockedTasks();
	Time last_plan_check;

public:
	typedef MaestroHub CLASSNAME;
	MaestroHub();
	~MaestroHub();
};

END_UPP_NAMESPACE

#endif