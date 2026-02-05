#ifndef _MaestroHub_MaestroHub_h_
#define _MaestroHub_MaestroHub_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

#include "MaestroAssistant.h"
#include "FleetDashboard.h"
#include "IntelligenceHub.h"
#include "Technology.h"
#include "Product.h"
#include "Maintenance.h"
#include "IssuesView.h"
#include "IssueDialogs.h"
#include "TriageDialog.h"
#include "RunbookEditor.h"
#include "StateEditor.h"
#include "SubworkManager.h"
#include "TUBrowser.h"
#include "LogAnalyzer.h"
#include "NewSessionDialog.h"
#include "InitDialog.h"
#include "ConfigurationDialog.h"
#include "OpsRunner.h"
#include "WorkDashboard.h"
#include "SessionManagement.h"
#include "AuditTrail.h"
#include "DebugWorkspace.h"
#include "Tools.h"

NAMESPACE_UPP

bool CreateIssueTaskFile(const String& root, const MaestroIssue& iss, const String& title, String& task_path);

class MaestroHub : public TopWindow {
	MenuBar   menu;
	ToolBar   toolbar;
	StatusBar statusbar;

	Splitter  main_split;
	Splitter  center_split;

	TabCtrl   left_tabs;
	TabCtrl   center_tabs;
	TabCtrl   bottom_tabs;
	
	One<MaestroAssistant>      assistant;
	
	One<FleetDashboard>        fleet;
	One<IntelligenceHub>       intelligence;
	One<AuditTrailCorrelator>  audit_trail;
	One<DebugWorkspace>        debug_workspace;
	One<TechnologyPane>        technology;
	One<ProductPane>           product;
	One<MaintenancePane>       maintenance;
	One<IssuesPane>            issues;
	One<WorkPane>              work;
	One<SessionManagementPane> sessions;
	
	RecentConfig config;
	String       current_root;
	String       active_track;
	String       active_phase;
	String       active_task;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void SelectRoot();
	
	void OnToggleAssistant();
	void OnEnact(String track, String phase, String task);
	void OnEnactStep(String runbook_title, int step_n, String instruction);
	void OnSessionSelect(String backend, String session_id);
	void OnNewSession();
	void OnCreateIssue();
	void OnInitMaestro();
	void OnTriageWizard();
	void OnRunbookEditor();
	void OnStateEditor();
	void OnTUBrowser();
	void OnLogAnalyzer();
	void OnSettings();
	void OnOpsRunner();
	void OnSuggestEnact();
	
	void SyncStatus();
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
