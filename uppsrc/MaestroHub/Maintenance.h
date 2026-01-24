#ifndef _MaestroHub_Maintenance_h_
#define _MaestroHub_Maintenance_h_

#include "MaestroHub.h"

NAMESPACE_UPP

class MaintenancePane : public ParentCtrl {
public:
	AIChatCtrl chat;
	
	void Load(const String& root);

	typedef MaintenancePane CLASSNAME;
	MaintenancePane();
};

END_UPP_NAMESPACE

#endif
