#ifndef _MaestroHub_IntelligenceHub_h_
#define _MaestroHub_IntelligenceHub_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>
#include "TUBrowser.h"
#include "LogAnalyzer.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class IntelligenceHub : public WithIntelligenceHubLayout<ParentCtrl> {
public:
	RepoView    repo;
	TUBrowser   tu;
	LogAnalyzer log;
	
	void Load(const String& maestro_root);
	
	typedef IntelligenceHub CLASSNAME;
	IntelligenceHub();
};

END_UPP_NAMESPACE

#endif