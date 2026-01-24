#ifndef _MaestroHub_MaestroHub_h_
#define _MaestroHub_MaestroHub_h_

#include <Maestro/Maestro.h>

NAMESPACE_UPP

// Forward declarations
class TechnologyPane;
class ProductPane;
class MaintenancePane;

class MaestroHub : public TopWindow {
	MenuBar menu;
	TabCtrl tabs;
	
	// Pointers to avoid circular include issues during refactor
	One<TechnologyPane>  technology;
	One<ProductPane>     product;
	One<MaintenancePane> maintenance;
	
	RecentConfig config;
	String       current_root;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void SelectRoot();
	
	void OnEnact(String track, String phase, String task);
	void OnEnactStep(String runbook_title, int step_n, String instruction);
	
	void LoadData();

public:
	typedef MaestroHub CLASSNAME;
	MaestroHub();
};

END_UPP_NAMESPACE

#endif
