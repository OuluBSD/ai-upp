#ifndef _MaestroHub_SessionManagement_h_
#define _MaestroHub_SessionManagement_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>
#include <Maestro/SessionSelectWindow.h>

NAMESPACE_UPP

class SessionManagementPane : public ParentCtrl {
public:
	Splitter        split;
	Splitter        detail_split;
	
	ParentCtrl      left_pane;
	ArrayCtrl       dirs;
	
	TabCtrl         list_tabs;
	SessionListView sessions;     // AI Engine sessions (legacy)
	
	ParentCtrl      work_pane;
	ParentCtrl      filter_bar;
	DropList        filter_type;
	DropList        filter_status;
	EditString      filter_search;
	ArrayCtrl       work_sessions; // Maestro Work Sessions
	
	TabCtrl         detail_tabs;
	RichTextView    context_view;
	ArrayCtrl       breadcrumbs;
	
	CliMaestroEngine engine;
	String          current_root;
	
	Function<void(String backend, String session_id)> WhenSelect;
	
	void Load(const String& root);
	void OnDirCursor();
	void OnSessionMenu(Bar& bar);
	void DeleteSession();
	
	void UpdateWorkSessionList();
	void OnWorkSessionCursor();

	typedef SessionManagementPane CLASSNAME;
	SessionManagementPane();
};

END_UPP_NAMESPACE

#endif
