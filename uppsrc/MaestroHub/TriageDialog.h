#ifndef _MaestroHub_TriageDialog_h_
#define _MaestroHub_TriageDialog_h_

#include "MaestroHub.h"

NAMESPACE_UPP

// Ensure layout is only included once in the blitz unit
#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class TriageDialog : public WithTriageLayout<TopWindow> {
	One<IssueManager> ism;
	Array<MaestroIssue> pending;
	int cursor = 0;
	String root;

public:
	void Load(const String& maestro_root);
	void UpdateUI();
	
	void OnAccept();
	void OnSkip();
	void OnIgnore();
	
	typedef TriageDialog CLASSNAME;
	TriageDialog();
};

END_UPP_NAMESPACE

#endif
