#ifndef _MaestroHub_SubworkManager_h_
#define _MaestroHub_SubworkManager_h_

#include "MaestroHub.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class SubworkManagerDialog : public WithSubworkLayout<TopWindow> {
	String root;
	String active_session_id;

public:
	void Load(const String& maestro_root, const String& session_id);
	void UpdateUI();
	
	void OnPush();
	void OnPop();
	
	typedef SubworkManagerDialog CLASSNAME;
	SubworkManagerDialog();
};

END_UPP_NAMESPACE

#endif
