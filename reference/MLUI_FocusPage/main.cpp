#include "MLUI_FocusPage.h"

NAMESPACE_UPP

static ValueMap FocusError(const String& error)
{
	ValueMap out;
	out.Add("ok", false);
	out.Add("error", error);
	return out;
}

INITBLOCK
{
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

	auto AddIssue = [&](const char *key, const char *title, const char *status_, const char *assignee_,
	                    int severity_, bool has_repro_, bool crash_, const char *notes_) {
		Issue& i = issues.Add();
		i.key = key;
		i.title = title;
		i.status = status_;
		i.assignee = assignee_;
		i.severity = severity_;
		i.has_repro = has_repro_;
		i.crash = crash_;
		i.notes = notes_;
	};

	AddIssue("OVR-101", "MLUI click timeout in menu action", "In Progress", "sblo", 4, true, false,
	         "Investigate UI thread blocking path in click handlers.");
	AddIssue("OVR-118", "Duplicate semantic rows in MLUI viewer", "Review", "ai-agent", 3, true, false,
	         "Root cause likely path key instability during incremental updates.");
	AddIssue("OVR-123", "FocusPage API draft in reference package", "Backlog", "team", 2, false, false,
	         "Agree minimal MCP shape: list/get/action first.");
	AddIssue("OVR-130", "Exit action reports success but process stays alive", "In Progress", "sblo", 5, true, true,
	         "Need deterministic shutdown path and better action confirmation.");

	issue_list.Clear();
	for(const Issue& i : issues)
		issue_list.Add(i.key, i.title, i.status, i.assignee);

	if(issue_list.GetCount() > 0)
		issue_list.SetCursor(0);
}

int IssueTrackerDemo::GetSelectedIssueIndex() const
{
	int row = issue_list.GetCursor();
	return (row >= 0 && row < issues.GetCount()) ? row : -1;
}

void IssueTrackerDemo::OnIssueSelection()
{
	RefreshInspector();
}

String IssueTrackerDemo::GetSelectedIssueKey() const
{
	int row = GetSelectedIssueIndex();
	return row >= 0 ? issues[row].key : String();
}

void IssueTrackerDemo::SetSelectedIssueByKey(const String& key)
{
	if(key.IsEmpty())
		return;
	for(int i = 0; i < issues.GetCount(); i++) {
		if(issues[i].key == key) {
			issue_list.SetCursor(i);
			issue_list.SetFocus();
			RefreshInspector();
			return;
		}
	}
}

void IssueTrackerDemo::RefreshInspector()
{
	int row = GetSelectedIssueIndex();
	if(row < 0) {
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

void IssueTrackerDemo::RefreshFocusPages()
{
	MLUI::FocusPage& board_page = MLUI::GetFocusPage("issue_board");
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

	board_page.ActionHandler("select_issue", [this](const ValueMap& args) -> Value {
		String key = args["key"];
		if(key.IsEmpty())
			key = args["value"];
		if(key.IsEmpty())
			return FocusError("Missing key/value");
		SetSelectedIssueByKey(key);
		ValueMap out;
		out.Add("ok", true);
		out.Add("selected_issue", GetSelectedIssueKey());
		return out;
	});
	board_page.ActionHandler("set_status", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		String new_status = args["status"];
		if(new_status.IsEmpty())
			new_status = args["value"];
		if(new_status.IsEmpty())
			return FocusError("Missing status/value");
		issues[row].status = new_status;
		issue_list.Set(row, 2, new_status);
		status.SetData(new_status);
		ValueMap out;
		out.Add("ok", true);
		out.Add("status", new_status);
		out.Add("selected_issue", GetSelectedIssueKey());
		return out;
	});
	board_page.ActionHandler("set_filter", [this](const ValueMap& args) -> Value {
		String q = args["query"];
		if(q.IsEmpty())
			q = args["value"];
		q = ToLower(TrimBoth(q));
		issue_list.Clear();
		for(const Issue& i : issues) {
			if(q.IsEmpty() || ToLower(i.key + " " + i.title + " " + i.assignee).Find(q) >= 0)
				issue_list.Add(i.key, i.title, i.status, i.assignee);
		}
		if(issue_list.GetCount() > 0)
			issue_list.SetCursor(0);
		RefreshInspector();
		ValueMap out;
		out.Add("ok", true);
		out.Add("rows", issue_list.GetCount());
		out.Add("query", q);
		return out;
	});

	MLUI::FocusPage& editor_page = MLUI::GetFocusPage("issue_editor");
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

	editor_page.ActionHandler("set_assignee", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		String value = args["assignee"];
		if(value.IsEmpty())
			value = args["value"];
		if(value.IsEmpty())
			return FocusError("Missing assignee/value");
		issues[row].assignee = value;
		assignee <<= value;
		issue_list.Set(row, 3, value);
		ValueMap out;
		out.Add("ok", true);
		out.Add("assignee", value);
		return out;
	});
	editor_page.ActionHandler("set_severity", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		Value sv = args["severity"];
		if(IsNull(sv))
			sv = args["value"];
		if(IsNull(sv))
			return FocusError("Missing severity/value");
		int value = minmax((int)sv, 0, 5);
		issues[row].severity = value;
		severity.SetIndex(value);
		ValueMap out;
		out.Add("ok", true);
		out.Add("severity", value);
		return out;
	});
	editor_page.ActionHandler("toggle_repro", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		bool value = issues[row].has_repro;
		Value vv = args["value"];
		if(!IsNull(vv))
			value = (bool)vv;
		else
			value = !value;
		issues[row].has_repro = value;
		has_repro = value;
		ValueMap out;
		out.Add("ok", true);
		out.Add("has_repro", value);
		return out;
	});
	editor_page.ActionHandler("toggle_crash", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		bool value = issues[row].crash;
		Value vv = args["value"];
		if(!IsNull(vv))
			value = (bool)vv;
		else
			value = !value;
		issues[row].crash = value;
		crash = value;
		ValueMap out;
		out.Add("ok", true);
		out.Add("crash", value);
		return out;
	});
	editor_page.ActionHandler("update_notes", [this](const ValueMap& args) -> Value {
		int row = GetSelectedIssueIndex();
		if(row < 0)
			return FocusError("No selection");
		String value = args["notes"];
		if(value.IsEmpty())
			value = args["value"];
		issues[row].notes = value;
		notes.Set(value);
		ValueMap out;
		out.Add("ok", true);
		out.Add("notes_len", value.GetCount());
		return out;
	});
}

bool IssueTrackerDemo::Access(Visitor& v)
{
	bool base_handled = TopWindow::Access(v);
	RefreshFocusPages();
	return base_handled;
}

END_UPP_NAMESPACE

CONSOLE_APP_MAIN
{
	Upp::IssueTrackerDemo().Run();
}
