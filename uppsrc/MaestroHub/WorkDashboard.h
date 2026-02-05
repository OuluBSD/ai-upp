#ifndef _MaestroHub_WorkDashboard_h_
#define _MaestroHub_WorkDashboard_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

class WorkPane : public ParentCtrl {
	Splitter     vsplit;
	Splitter     hsplit;
	
	RichTextView task_details;
	ArrayCtrl    plan_steps;
	RichTextView breadcrumbs;
	
	Button       btn_approve;
	Button       btn_reject;
	Button       btn_subwork;
	Button       btn_refresh;
	
	String       current_root;
	String       active_session_id;

public:
	void Load(const String& root);
	void Refresh();
	void OnApprove();
	void OnReject();
	void OnSubwork();

	typedef WorkPane CLASSNAME;
	WorkPane();
};

END_UPP_NAMESPACE

#endif
