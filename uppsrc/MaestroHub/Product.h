#ifndef _MaestroHub_Product_h_
#define _MaestroHub_Product_h_

#include "MaestroHub.h"

NAMESPACE_UPP

class ProductPane : public ParentCtrl {
public:
	Splitter split;
	Splitter vsplit_rb;
	Splitter vsplit_wg;
	
	ArrayCtrl    runbooks;
	RichTextView rb_detail;
	
	ArrayCtrl    workflows;
	RichTextView wg_detail;
	
	Array<Runbook>  runbook_data;
	Array<WorkGraph> workflow_data;
	
	Function<void(String runbook_title, int step_n, String instruction)> WhenEnactStep;
	
	void Load(const String& root);
	
	void OnRunbookSelect();
	void OnWorkflowSelect();

	typedef ProductPane CLASSNAME;
	ProductPane();
};

END_UPP_NAMESPACE

#endif
