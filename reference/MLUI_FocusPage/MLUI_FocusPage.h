#ifndef _MLUI_FocusPage_MLUI_FocusPage_h_
#define _MLUI_FocusPage_MLUI_FocusPage_h_

#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

class IssueTrackerDemo : public TopWindow {
public:
	typedef IssueTrackerDemo CLASSNAME;

	IssueTrackerDemo();
	virtual bool Access(Visitor& v) override;

private:
	struct Issue : Moveable<Issue> {
		String key;
		String title;
		String status;
		String assignee;
		int    severity = 0;
		bool   has_repro = false;
		bool   crash = false;
		String notes;
	};

	void PopulateIssues();
	void OnIssueSelection();
	void RefreshInspector();
	void RefreshFocusPages();
	int  GetSelectedIssueIndex() const;
	String GetSelectedIssueKey() const;
	void SetSelectedIssueByKey(const String& key);

	Splitter   hsplit;
	ArrayCtrl  issue_list;
	ParentCtrl inspector;

	Label      workspace_label;
	EditString workspace;
	Label      sprint_label;
	EditString sprint;
	Label      issue_key_label;
	EditString issue_key;
	Label      title_label;
	EditString issue_title;
	Label      status_label;
	DropList   status;
	Label      assignee_label;
	EditString assignee;
	Label      severity_label;
	DropList   severity;
	Option     has_repro;
	Option     crash;
	DocEdit    notes;

	Vector<Issue> issues;
};

END_UPP_NAMESPACE

#endif
