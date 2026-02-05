#ifndef _MaestroHub_NewSessionDialog_h_
#define _MaestroHub_NewSessionDialog_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class NewSessionDialog : public WithNewSessionLayout<TopWindow> {
public:
	String session_id;
	
	void OnOK();
	
	typedef NewSessionDialog CLASSNAME;
	NewSessionDialog();
};

END_UPP_NAMESPACE

#endif