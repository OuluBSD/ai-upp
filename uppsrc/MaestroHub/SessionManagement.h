#ifndef _MaestroHub_SessionManagement_h_
#define _MaestroHub_SessionManagement_h_

#include "MaestroHub.h"
#include <Maestro/SessionSelectWindow.h>

NAMESPACE_UPP

class SessionManagementPane : public ParentCtrl {
public:
	Splitter        split;
	ArrayCtrl       dirs;
	SessionListView sessions;
	
	CliMaestroEngine engine;
	String          current_root;
	
	void Load(const String& root);
	void OnDirCursor();
	void OnSessionMenu(Bar& bar);
	void DeleteSession();

	typedef SessionManagementPane CLASSNAME;
	SessionManagementPane();
};

END_UPP_NAMESPACE

#endif
