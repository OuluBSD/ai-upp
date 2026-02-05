#ifndef _MaestroHub_OpsRunner_h_
#define _MaestroHub_OpsRunner_h_

#include "MaestroHub.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class OpsRunner : public WithOpsRunnerLayout<TopWindow> {
	String root;

public:
	void Load(const String& maestro_root);
	void OnRun();
	
	typedef OpsRunner CLASSNAME;
	OpsRunner();
};

END_UPP_NAMESPACE

#endif
