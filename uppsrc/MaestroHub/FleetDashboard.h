#ifndef _MaestroHub_FleetDashboard_h_
#define _MaestroHub_FleetDashboard_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class FleetDashboard : public WithFleetDashboardLayout<ParentCtrl> {
public:
	TreeArrayCtrl  project_grid;
	ArrayCtrl      automation_queue;
	
	void LoadProjects(const Vector<String>& paths);
	void UpdateQueue();
	
	typedef FleetDashboard CLASSNAME;
	FleetDashboard();
};

END_UPP_NAMESPACE

#endif
