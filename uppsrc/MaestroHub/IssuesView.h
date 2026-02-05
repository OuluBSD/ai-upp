#ifndef _MaestroHub_IssuesView_h_
#define _MaestroHub_IssuesView_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

NAMESPACE_UPP

class IssuesPane : public ParentCtrl {
public:
	Splitter  split;
	ArrayCtrl list;
	RichTextView detail;
	
	String    current_root;
	
	void Load(const String& root);
	void OnMenu(Bar& bar);
	void OnTriage();
	void OnResolve();
	void OnEdit();
	void OnIgnore();
	void OnCreateTask();
	
	Vector<String> GetSelectedIssueIds() const;
	void BulkSetStatus();
	void BulkSetSeverity();

	typedef IssuesPane CLASSNAME;
	IssuesPane();
};

END_UPP_NAMESPACE

#endif