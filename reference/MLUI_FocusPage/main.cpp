#include "MLUI_FocusPage.h"

NAMESPACE_UPP

namespace MLUI {

static VectorMap<String, FocusPageDef>& FocusPages()
{
	static VectorMap<String, FocusPageDef> pages;
	return pages;
}

FocusPageDef& FocusPageDef::Add(Ctrl& c)
{
	String id = c.GetLayoutId();
	if(id.IsEmpty())
		id = String().Cat() << c.GetTypeName() << "@" << HexStr((uintptr_t)&c);
	if(FindIndex(linked_ctrls, id) < 0)
		linked_ctrls.Add(id);
	return *this;
}

FocusPageDef& FocusPageDef::Action(const String& action_id, const String& label, const String& hint, bool enabled)
{
	FocusAction& a = actions.Add();
	a.id = action_id;
	a.label = label;
	a.hint = hint;
	a.enabled = enabled;
	return *this;
}

FocusPageDef& FocusPageDef::Context(const String& key, const Value& value)
{
	static_context.GetAdd(key) = value;
	return *this;
}

FocusPageDef& FocusPageDef::ClearRuntime()
{
	runtime_values.Clear();
	runtime_value_desc.Clear();
	runtime_ctrls.Clear();
	runtime_ctrl_desc.Clear();
	runtime_action_enabled.Clear();
	runtime_action_desc.Clear();
	return *this;
}

FocusPageDef& FocusPageDef::AddValue(const String& key, const Value& value, const String& description)
{
	runtime_values.GetAdd(key) = value;
	runtime_value_desc.GetAdd(key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddCtrl(const String& key, const Ctrl& ctrl, const String& description)
{
	String id = ctrl.GetLayoutId();
	if(id.IsEmpty())
		id = String().Cat() << ctrl.GetTypeName() << "@" << HexStr((uintptr_t)&ctrl);
	runtime_ctrls.GetAdd(key) = id;
	runtime_ctrl_desc.GetAdd(key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddState(const String& key, const Value& value, const String& description)
{
	String state_key = String().Cat() << "state." << key;
	runtime_values.GetAdd(state_key) = value;
	runtime_value_desc.GetAdd(state_key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddAction(const String& action_id, bool enabled, const String& description)
{
	runtime_action_enabled.GetAdd(action_id) = enabled;
	runtime_action_desc.GetAdd(action_id) = description;
	return *this;
}

FocusPageDef& RegisterFocusPage(const String& id, const String& title, const String& description)
{
	VectorMap<String, FocusPageDef>& pages = FocusPages();
	int i = pages.Find(id);
	if(i < 0) {
		FocusPageDef& p = pages.Add(id);
		p.id = id;
		p.title = title;
		p.description = description;
		return p;
	}
	FocusPageDef& p = pages[i];
	p.title = title;
	p.description = description;
	return p;
}

FocusPageDef& GetFocusPage(const String& id)
{
	VectorMap<String, FocusPageDef>& pages = FocusPages();
	int i = pages.Find(id);
	if(i < 0)
		return RegisterFocusPage(id, id, "No description");
	return pages[i];
}

bool HasFocusPage(const String& id)
{
	return FocusPages().Find(id) >= 0;
}

void EmitFocusPages(AutomationVisitor& av)
{
	av.AccessLabel("MLUI FocusPages");
	av.AccessValue(FocusPages().GetCount());

	for(int i = 0; i < FocusPages().GetCount(); i++) {
		const FocusPageDef& p = FocusPages()[i];
		String header = String().Cat() << "FocusPage : " << p.id << " (" << p.title << ")";
		av.AccessLabel(~header);
		av.AccessLabel(~p.description);

		for(int j = 0; j < p.static_context.GetCount(); j++) {
			String line = String().Cat() << "ctx." << p.static_context.GetKey(j) << "=" << AsString(p.static_context[j]);
			av.AccessLabel(~line);
		}

		for(const FocusAction& a : p.actions) {
			String line = String().Cat() << "action." << a.id
			                             << " label=\"" << a.label << "\""
			                             << " enabled=" << (a.enabled ? "true" : "false")
			                             << " hint=\"" << a.hint << "\"";
			av.AccessLabel(~line);
		}

		for(int j = 0; j < p.runtime_values.GetCount(); j++) {
			String key = p.runtime_values.GetKey(j);
			String desc;
			int di = p.runtime_value_desc.Find(key);
			if(di >= 0)
				desc = p.runtime_value_desc[di];
			av.AccessLabel(String().Cat() << "runtime.value." << p.id << "." << key << "=" << AsString(p.runtime_values[j]));
			if(!desc.IsEmpty())
				av.AccessLabel(String().Cat() << "runtime.value_desc." << p.id << "." << key << "=" << desc);
		}

		for(int j = 0; j < p.runtime_ctrls.GetCount(); j++) {
			String key = p.runtime_ctrls.GetKey(j);
			String desc;
			int di = p.runtime_ctrl_desc.Find(key);
			if(di >= 0)
				desc = p.runtime_ctrl_desc[di];
			av.AccessLabel(String().Cat() << "runtime.ctrl." << p.id << "." << key << "=" << p.runtime_ctrls[j]);
			if(!desc.IsEmpty())
				av.AccessLabel(String().Cat() << "runtime.ctrl_desc." << p.id << "." << key << "=" << desc);
		}

		for(int j = 0; j < p.runtime_action_enabled.GetCount(); j++) {
			String key = p.runtime_action_enabled.GetKey(j);
			String desc;
			int di = p.runtime_action_desc.Find(key);
			if(di >= 0)
				desc = p.runtime_action_desc[di];
			av.AccessLabel(String().Cat() << "runtime.action." << p.id << "." << key
			                              << " enabled=" << (p.runtime_action_enabled[j] ? "true" : "false"));
			if(!desc.IsEmpty())
				av.AccessLabel(String().Cat() << "runtime.action_desc." << p.id << "." << key << "=" << desc);
		}
	}
}

}

INITBLOCK {
	MLUI::RegisterFocusPage(
		"issue_board",
		"Issue Board",
		"Compact issue list for active workspace and sprint")
		.Context("purpose", "Navigate and triage issues quickly")
		.Action("select_issue", "Select issue", "Set active issue from board row")
		.Action("set_status", "Set status", "Move issue to new workflow state")
		.Action("set_filter", "Set filter", "Filter board by text or owner");

	MLUI::RegisterFocusPage(
		"issue_editor",
		"Issue Editor",
		"Focused metadata view for selected issue")
		.Context("purpose", "Edit issue details and execution readiness")
		.Action("set_assignee", "Set assignee", "Assign issue to owner")
		.Action("set_severity", "Set severity", "Update severity level")
		.Action("toggle_repro", "Toggle repro", "Mark whether issue has reproduction steps")
		.Action("toggle_crash", "Toggle crash", "Mark whether issue is crash-level")
		.Action("update_notes", "Update notes", "Write short engineering notes");
}

IssueTrackerDemo::IssueTrackerDemo()
{
	Title("Issue Tracker Demo");
	Sizeable().Zoomable();
	SetRect(0, 0, 1150, 740);

	workspace <<= "ai-upp";
	sprint <<= "Sprint 19";

	issue_list.AddColumn("Key", 90);
	issue_list.AddColumn("Title", 380);
	issue_list.AddColumn("Status", 120);
	issue_list.AddColumn("Owner", 130);
	issue_list.WhenSel = THISBACK(OnIssueSelection);
	PopulateIssues();

	status.Add("Backlog");
	status.Add("In Progress");
	status.Add("Review");
	status.Add("Done");

	for(int i = 0; i <= 5; i++)
		severity.Add(i);

	workspace_label.SetLabel("Workspace");
	sprint_label.SetLabel("Sprint");
	issue_key_label.SetLabel("Issue Key");
	title_label.SetLabel("Title");
	status_label.SetLabel("Status");
	assignee_label.SetLabel("Assignee");
	severity_label.SetLabel("Severity");
	has_repro.SetLabel("Has repro steps");
	crash.SetLabel("Crash-level issue");

	inspector.Add(workspace_label.LeftPos(10, 100).TopPos(10, 20));
	inspector.Add(workspace.LeftPos(120, 250).TopPos(10, 20));
	inspector.Add(sprint_label.LeftPos(390, 70).TopPos(10, 20));
	inspector.Add(sprint.LeftPos(465, 180).TopPos(10, 20));

	inspector.Add(issue_key_label.LeftPos(10, 100).TopPos(40, 20));
	inspector.Add(issue_key.LeftPos(120, 180).TopPos(40, 20));
	inspector.Add(title_label.LeftPos(310, 50).TopPos(40, 20));
	inspector.Add(issue_title.LeftPos(365, 420).TopPos(40, 20));

	inspector.Add(status_label.LeftPos(10, 100).TopPos(70, 20));
	inspector.Add(status.LeftPos(120, 180).TopPos(70, 20));
	inspector.Add(assignee_label.LeftPos(310, 70).TopPos(70, 20));
	inspector.Add(assignee.LeftPos(385, 180).TopPos(70, 20));
	inspector.Add(severity_label.LeftPos(580, 70).TopPos(70, 20));
	inspector.Add(severity.LeftPos(655, 80).TopPos(70, 20));

	inspector.Add(has_repro.LeftPos(10, 180).TopPos(100, 20));
	inspector.Add(crash.LeftPos(200, 180).TopPos(100, 20));

	inspector.Add(notes.HSizePos(10, 10).VSizePos(130, 10));

	hsplit.Horz(issue_list, inspector);
	hsplit.SetPos(3800);
	Add(hsplit.SizePos());

	MLUI::GetFocusPage("issue_board")
		.Add(issue_list)
		.Add(workspace)
		.Add(sprint)
		.Context("domain", "issue-tracker");

	MLUI::GetFocusPage("issue_editor")
		.Add(issue_key)
		.Add(issue_title)
		.Add(status)
		.Add(assignee)
		.Add(severity)
		.Add(has_repro)
		.Add(crash)
		.Add(notes)
		.Context("editing_scope", "single-issue");

	RefreshInspector();
}

void IssueTrackerDemo::PopulateIssues()
{
	issues.Clear();
	issues.Add({ "OVR-101", "MLUI click timeout in menu action", "In Progress", "sblo", 4, true, false,
	             "Investigate UI thread blocking path in click handlers." });
	issues.Add({ "OVR-118", "Duplicate semantic rows in MLUI viewer", "Review", "ai-agent", 3, true, false,
	             "Root cause likely path key instability during incremental updates." });
	issues.Add({ "OVR-123", "FocusPage API draft in reference package", "Backlog", "team", 2, false, false,
	             "Agree minimal MCP shape: list/get/action first." });
	issues.Add({ "OVR-130", "Exit action reports success but process stays alive", "In Progress", "sblo", 5, true, true,
	             "Need deterministic shutdown path and better action confirmation." });

	issue_list.Clear();
	for(const Issue& i : issues)
		issue_list.Add(i.key, i.title, i.status, i.assignee);

	if(issue_list.GetCount() > 0)
		issue_list.SetCursor(0);
}

void IssueTrackerDemo::OnIssueSelection()
{
	RefreshInspector();
}

String IssueTrackerDemo::GetSelectedIssueKey() const
{
	int row = issue_list.GetCursor();
	if(row < 0 || row >= issue_list.GetCount())
		return String();
	return (String)issue_list.Get(row, 0);
}

void IssueTrackerDemo::RefreshInspector()
{
	int row = issue_list.GetCursor();
	if(row < 0 || row >= issues.GetCount()) {
		issue_key <<= "";
		issue_title <<= "";
		status.SetIndex(0);
		assignee <<= "";
		severity.SetIndex(0);
		has_repro = false;
		crash = false;
		notes.Clear();
		return;
	}

	const Issue& i = issues[row];
	issue_key <<= i.key;
	issue_title <<= i.title;
	status.SetData(i.status);
	assignee <<= i.assignee;
	severity.SetIndex(minmax(i.severity, 0, 5));
	has_repro = i.has_repro;
	crash = i.crash;
	notes.Set(i.notes);
}

bool IssueTrackerDemo::Access(Visitor& v)
{
	bool base_handled = TopWindow::Access(v);

	if(auto *av = dynamic_cast<AutomationVisitor*>(&v)) {
		MLUI::FocusPageDef& board_page = MLUI::GetFocusPage("issue_board");
		board_page.ClearRuntime();
		String board_workspace = workspace.GetData();
		String board_sprint = sprint.GetData();
		String board_selected_issue = GetSelectedIssueKey();
		bool board_has_selection = !board_selected_issue.IsEmpty();
		MLUI_USE_VAR(board_page, board_workspace, "Current workspace name");
		MLUI_USE_VAR(board_page, board_sprint, "Current sprint name");
		MLUI_USE_VAR(board_page, board_selected_issue, "Selected issue key in board list");
		MLUI_USE_CTRL(board_page, issue_list, "Main issue board list control");
		MLUI_USE_CTRL(board_page, workspace, "Workspace editor control");
		MLUI_USE_CTRL(board_page, sprint, "Sprint editor control");
		MLUI_USE_STATE(board_page, "selected_issue", board_selected_issue, "Board selection mirror");
		MLUI_USE_ACTION(board_page, "select_issue", board_has_selection, "Select current issue row");
		MLUI_USE_ACTION(board_page, "set_status", board_has_selection, "Set selected issue status");
		MLUI_USE_ACTION(board_page, "set_filter", true, "Filter board by text or owner");

		MLUI::FocusPageDef& editor_page = MLUI::GetFocusPage("issue_editor");
		editor_page.ClearRuntime();
		String editor_issue_key = issue_key.GetData();
		String editor_title = issue_title.GetData();
		String editor_status = status.GetData();
		String editor_assignee = assignee.GetData();
		int editor_severity = severity.GetIndex();
		bool editor_has_repro = has_repro;
		bool editor_is_crash = crash;
		bool editor_has_issue = !editor_issue_key.IsEmpty();
		MLUI_USE_VAR(editor_page, editor_issue_key, "Issue key shown in editor");
		MLUI_USE_VAR(editor_page, editor_title, "Issue title shown in editor");
		MLUI_USE_VAR(editor_page, editor_status, "Editor status value");
		MLUI_USE_VAR(editor_page, editor_assignee, "Editor assignee value");
		MLUI_USE_VAR(editor_page, editor_severity, "Editor severity index");
		MLUI_USE_VAR(editor_page, editor_has_repro, "Editor repro flag");
		MLUI_USE_VAR(editor_page, editor_is_crash, "Editor crash flag");
		MLUI_USE_CTRL(editor_page, issue_key, "Issue key control");
		MLUI_USE_CTRL(editor_page, issue_title, "Issue title control");
		MLUI_USE_CTRL(editor_page, status, "Issue status selector");
		MLUI_USE_CTRL(editor_page, assignee, "Issue assignee editor");
		MLUI_USE_CTRL(editor_page, severity, "Issue severity selector");
		MLUI_USE_CTRL(editor_page, notes, "Issue notes editor");
		MLUI_USE_STATE(editor_page, "selected_issue", editor_issue_key, "Editor selection mirror");
		MLUI_USE_ACTION(editor_page, "set_assignee", editor_has_issue, "Assign selected issue to owner");
		MLUI_USE_ACTION(editor_page, "set_severity", editor_has_issue, "Change selected issue severity");
		MLUI_USE_ACTION(editor_page, "toggle_repro", editor_has_issue, "Toggle reproduction availability");
		MLUI_USE_ACTION(editor_page, "toggle_crash", editor_has_issue, "Toggle crash-level marker");
		MLUI_USE_ACTION(editor_page, "update_notes", editor_has_issue, "Update selected issue notes");

		MLUI::EmitFocusPages(*av);
		return true;
	}

	return base_handled;
}

GUI_APP_MAIN
{
	IssueTrackerDemo().Run();
}

END_UPP_NAMESPACE
