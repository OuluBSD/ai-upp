#ifndef _MaestroHub_LogAnalyzer_h_
#define _MaestroHub_LogAnalyzer_h_

#include "MaestroHub.h"

NAMESPACE_UPP

#ifndef _MaestroHub_Layout_Included_
#define _MaestroHub_Layout_Included_
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>
#endif

class LogAnalyzer : public WithLogAnalyzerLayout<TopWindow> {
	ArrayCtrl    scan_list;
	
	Splitter     hsplit;
	ArrayCtrl    finding_list;
	RichTextView detail_view;
	
	One<LogManager> lm;
	String       root;

public:
	void Load(const String& maestro_root);
	void UpdateScans();
	void OnScanCursor();
	void OnFindingCursor();
	void OnCreateIssue();
	
	typedef LogAnalyzer CLASSNAME;
	LogAnalyzer();
};

END_UPP_NAMESPACE

#endif
