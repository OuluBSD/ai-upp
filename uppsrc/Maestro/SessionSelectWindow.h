#ifndef _Maestro_SessionSelectWindow_h_
#define _Maestro_SessionSelectWindow_h_

#include <CtrlLib/CtrlLib.h>
#include "CliEngine.h"

class SessionSelectWindow : public TopWindow {
public:
	Splitter  split;
	ArrayCtrl dirs;
	ArrayCtrl sessions;
	
	CliMaestroEngine* engine = nullptr;
	String    selected_id;
	
	void DataDirectories();
	void OnDirCursor();
	void OnSessionDouble();
	
	void Load(CliMaestroEngine& engine);

	typedef SessionSelectWindow CLASSNAME;
	SessionSelectWindow(CliMaestroEngine& engine);
};

#endif