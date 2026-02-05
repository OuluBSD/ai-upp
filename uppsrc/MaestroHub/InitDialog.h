#ifndef _MaestroHub_InitDialog_h_
#define _MaestroHub_InitDialog_h_

#include "MaestroHub.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class InitDialog : public WithInitLayout<TopWindow> {
public:
	void OnBrowse();
	
	typedef InitDialog CLASSNAME;
	InitDialog();
};

END_UPP_NAMESPACE

#endif
