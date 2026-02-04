#ifndef _MaestroHub_Maintenance_h_
#define _MaestroHub_Maintenance_h_

NAMESPACE_UPP

class MaintenancePane : public ParentCtrl {
public:
	AIChatCtrl chat;
	Label      active_info;
	
	void Load(const String& root);
	void SessionStatus(const String& backend, const String& session_id);

	typedef MaintenancePane CLASSNAME;
	MaintenancePane();
};

END_UPP_NAMESPACE

#endif