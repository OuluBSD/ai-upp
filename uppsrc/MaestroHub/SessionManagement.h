#ifndef _MaestroHub_SessionManagement_h_
#define _MaestroHub_SessionManagement_h_

NAMESPACE_UPP

class SessionManagementPane : public ParentCtrl {
public:
	Splitter        split;
	ArrayCtrl       dirs;
	TabCtrl         list_tabs;
	SessionListView sessions;     // AI Engine sessions (legacy)
	ArrayCtrl       work_sessions; // Maestro Work Sessions
	
	CliMaestroEngine engine;
	String          current_root;
	
	Function<void(String backend, String session_id)> WhenSelect;
	
	void Load(const String& root);
	void OnDirCursor();
	void OnSessionMenu(Bar& bar);
	void DeleteSession();

	typedef SessionManagementPane CLASSNAME;
	SessionManagementPane();
};

END_UPP_NAMESPACE

#endif
