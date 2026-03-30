#ifndef _MLUI_FocusPage_MLUI_FocusPage_h_
#define _MLUI_FocusPage_MLUI_FocusPage_h_

#include <CtrlLib/CtrlLib.h>
#include <CtrlLib/GuiAutomation.h>

NAMESPACE_UPP

namespace MLUI {

struct FocusAction : Moveable<FocusAction> {
	String id;
	String label;
	String hint;
	bool   enabled = true;
};

struct FocusPageDef : Moveable<FocusPageDef> {
	String id;
	String title;
	String description;

	Vector<String> linked_ctrls;
	Vector<FocusAction> actions;
	VectorMap<String, Value> static_context;
	VectorMap<String, Value> runtime_values;
	VectorMap<String, String> runtime_value_desc;
	VectorMap<String, String> runtime_ctrls;
	VectorMap<String, String> runtime_ctrl_desc;
	VectorMap<String, bool> runtime_action_enabled;
	VectorMap<String, String> runtime_action_desc;

	FocusPageDef& Add(Ctrl& c);
	FocusPageDef& Action(const String& id, const String& label, const String& hint, bool enabled = true);
	FocusPageDef& Context(const String& key, const Value& value);
	FocusPageDef& ClearRuntime();
	FocusPageDef& AddValue(const String& key, const Value& value, const String& description);
	FocusPageDef& AddCtrl(const String& key, const Ctrl& ctrl, const String& description);
	FocusPageDef& AddState(const String& key, const Value& value, const String& description);
	FocusPageDef& AddAction(const String& action_id, bool enabled, const String& description);
};

FocusPageDef& RegisterFocusPage(const String& id, const String& title, const String& description);
FocusPageDef& GetFocusPage(const String& id);
bool          HasFocusPage(const String& id);
void          EmitFocusPages(AutomationVisitor& av);

}

#ifndef MLUI_USE_VAR
#define MLUI_USE_VAR(page, var, description) \
	(page).AddValue(#var, Value(var), (description))
#endif

#ifndef MLUI_USE_CTRL
#define MLUI_USE_CTRL(page, ctrl, description) \
	(page).AddCtrl(#ctrl, (ctrl), (description))
#endif

#ifndef MLUI_USE_STATE
#define MLUI_USE_STATE(page, key, value, description) \
	(page).AddState((key), Value(value), (description))
#endif

#ifndef MLUI_USE_ACTION
#define MLUI_USE_ACTION(page, action_id, enabled, description) \
	(page).AddAction((action_id), (enabled), (description))
#endif

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
	String GetSelectedIssueKey() const;

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
