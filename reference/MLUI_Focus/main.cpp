#include "MLUI_Focus.h"

NAMESPACE_UPP

namespace MLUIRef {

static VectorMap<String, FocusPageDef>& PageRegistry() { static VectorMap<String, FocusPageDef> x; return x; }
static VectorMap<String, FocusRouteDef>& RouteRegistry() { static VectorMap<String, FocusRouteDef> x; return x; }
static VectorMap<String, FocusSiteMapDef>& SiteMapRegistry() { static VectorMap<String, FocusSiteMapDef> x; return x; }
static VectorMap<String, FocusLinkDef>& LinkRegistry() { static VectorMap<String, FocusLinkDef> x; return x; }
static VectorMap<String, FocusQueryDef>& QueryRegistry() { static VectorMap<String, FocusQueryDef> x; return x; }
static VectorMap<String, FocusComponentDef>& ComponentRegistry() { static VectorMap<String, FocusComponentDef> x; return x; }
static VectorMap<String, FocusFormDef>& FormRegistry() { static VectorMap<String, FocusFormDef> x; return x; }
static VectorMap<String, FocusActionContractDef>& ActionRegistry() { static VectorMap<String, FocusActionContractDef> x; return x; }
static VectorMap<String, FocusStateDef>& StateRegistry() { static VectorMap<String, FocusStateDef> x; return x; }
static VectorMap<String, FocusPermissionDef>& PermissionRegistry() { static VectorMap<String, FocusPermissionDef> x; return x; }
static Vector<FocusHistoryEntry>& HistoryLog() { static Vector<FocusHistoryEntry> x; return x; }
static Vector<FocusDiffOp>& DiffLog() { static Vector<FocusDiffOp> x; return x; }
static Vector<FocusDevtoolsEntry>& DevtoolsLog() { static Vector<FocusDevtoolsEntry> x; return x; }

static ValueMap ActionError(const String& error)
{
	ValueMap out;
	out.Add("ok", false);
	out.Add("error", error);
	return out;
}

FocusPageDef& FocusPageDef::Route(const String& route_id) {
	if(FindIndex(route_ids, route_id) < 0)
		route_ids.Add(route_id);
	return *this;
}

FocusPageDef& FocusPageDef::Component(const String& component_id) {
	if(FindIndex(component_ids, component_id) < 0)
		component_ids.Add(component_id);
	return *this;
}

FocusPageDef& FocusPageDef::Action(const String& action_id) {
	if(FindIndex(action_ids, action_id) < 0)
		action_ids.Add(action_id);
	return *this;
}

FocusPageDef& FocusPageDef::ClearRuntime() {
	runtime_values.Clear();
	runtime_value_desc.Clear();
	runtime_ctrls.Clear();
	runtime_ctrl_desc.Clear();
	runtime_action_enabled.Clear();
	runtime_action_desc.Clear();
	return *this;
}

FocusPageDef& FocusPageDef::AddValue(const String& key, const Value& value, const String& description) {
	runtime_values.GetAdd(key) = value;
	runtime_value_desc.GetAdd(key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddCtrl(const String& key, const Ctrl& ctrl, const String& description) {
	String id = ctrl.GetLayoutId();
	if(id.IsEmpty())
		id = String().Cat() << "Ctrl@" << HexStr((uintptr_t)&ctrl);
	runtime_ctrls.GetAdd(key) = id;
	runtime_ctrl_desc.GetAdd(key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddState(const String& key, const Value& value, const String& description) {
	runtime_values.GetAdd(String().Cat() << "state." << key) = value;
	runtime_value_desc.GetAdd(String().Cat() << "state." << key) = description;
	return *this;
}

FocusPageDef& FocusPageDef::AddAction(const String& action_id, bool enabled, const String& description) {
	runtime_action_enabled.GetAdd(action_id) = enabled;
	runtime_action_desc.GetAdd(action_id) = description;
	return *this;
}

FocusSiteMapDef& FocusSiteMapDef::Route(const String& route_id) {
	if(FindIndex(route_ids, route_id) < 0)
		route_ids.Add(route_id);
	return *this;
}

FocusComponentDef& FocusComponentDef::Key(const String& payload_key) {
	if(FindIndex(payload_keys, payload_key) < 0)
		payload_keys.Add(payload_key);
	return *this;
}

FocusFormDef& FocusFormDef::Field(const String& field_id, const String& type, bool required, const String& summary) {
	FocusFormFieldDef& f = fields.Add();
	f.id = field_id;
	f.type = type;
	f.required = required;
	f.summary = summary;
	return *this;
}

FocusActionContractDef& FocusActionContractDef::SideEffect(const String& effect) {
	side_effects.Add(effect);
	return *this;
}

FocusStateDef& FocusStateDef::Set(const String& key, const Value& value) {
	values.GetAdd(key) = value;
	return *this;
}

template <class T>
static T& Upsert(VectorMap<String, T>& m, const String& id)
{
	int i = m.Find(id);
	if(i < 0) {
		T& t = m.Add(id);
		t.id = id;
		return t;
	}
	return m[i];
}

FocusPageDef& RegisterFocusPage(const String& id, const String& title, const String& summary) {
	FocusPageDef& p = Upsert(PageRegistry(), id);
	p.title = title;
	p.summary = summary;
	return p;
}

FocusPageDef& GetFocusPage(const String& id) {
	return Upsert(PageRegistry(), id);
}

FocusRouteDef& RegisterFocusRoute(const String& id, const String& path, const String& page_id, const String& summary, bool is_default) {
	FocusRouteDef& r = Upsert(RouteRegistry(), id);
	r.path = path;
	r.page_id = page_id;
	r.summary = summary;
	r.is_default = is_default;
	return r;
}

FocusSiteMapDef& RegisterFocusSiteMap(const String& id, const String& title) {
	FocusSiteMapDef& s = Upsert(SiteMapRegistry(), id);
	s.title = title;
	return s;
}

FocusLinkDef& RegisterFocusLink(const String& id, const String& from_route_id, const String& to_route_id, const String& label, const String& condition) {
	FocusLinkDef& l = Upsert(LinkRegistry(), id);
	l.from_route = from_route_id;
	l.to_route = to_route_id;
	l.label = label;
	l.condition = condition;
	return l;
}

FocusQueryDef& RegisterFocusQuery(const String& id, const String& route_id, const String& param, const String& default_value, const String& summary) {
	FocusQueryDef& q = Upsert(QueryRegistry(), id);
	q.route_id = route_id;
	q.param = param;
	q.default_value = default_value;
	q.summary = summary;
	return q;
}

FocusComponentDef& RegisterFocusComponent(const String& id, const String& page_id, const String& title, const String& summary) {
	FocusComponentDef& c = Upsert(ComponentRegistry(), id);
	c.page_id = page_id;
	c.title = title;
	c.summary = summary;
	return c;
}

FocusFormDef& RegisterFocusForm(const String& id, const String& page_id, const String& action_id) {
	FocusFormDef& f = Upsert(FormRegistry(), id);
	f.page_id = page_id;
	f.action_id = action_id;
	return f;
}

FocusActionContractDef& RegisterFocusActionContract(const String& id, const String& page_id, const String& summary, const String& input_schema, const String& output_schema) {
	FocusActionContractDef& a = Upsert(ActionRegistry(), id);
	a.page_id = page_id;
	a.summary = summary;
	a.input_schema = input_schema;
	a.output_schema = output_schema;
	return a;
}

FocusStateDef& RegisterFocusState(const String& id, const String& page_id) {
	FocusStateDef& s = Upsert(StateRegistry(), id);
	s.page_id = page_id;
	return s;
}

FocusStateDef& GetFocusState(const String& id) {
	return Upsert(StateRegistry(), id);
}

void PushFocusHistory(const String& from_route, const String& to_route, const String& action_id) {
	FocusHistoryEntry& e = HistoryLog().Add();
	e.from_route = from_route;
	e.to_route = to_route;
	e.action_id = action_id;
	e.stamp = GetSysTime();
	if(HistoryLog().GetCount() > 64)
		HistoryLog().Remove(0);
}

void PushFocusDiff(const String& component_id, const String& op, const String& path, const Value& value) {
	FocusDiffOp& d = DiffLog().Add();
	d.component_id = component_id;
	d.op = op;
	d.path = path;
	d.value = value;
	d.stamp = GetSysTime();
	if(DiffLog().GetCount() > 64)
		DiffLog().Remove(0);
}

void SetFocusPermission(const String& action_id, bool enabled, const String& disabled_reason) {
	FocusPermissionDef& p = Upsert(PermissionRegistry(), action_id);
	p.action_id = action_id;
	p.enabled = enabled;
	p.disabled_reason = disabled_reason;
}

void PushFocusDevtools(const String& channel, const String& level, const String& message) {
	FocusDevtoolsEntry& e = DevtoolsLog().Add();
	e.channel = channel;
	e.level = level;
	e.message = message;
	e.stamp = GetSysTime();
	if(DevtoolsLog().GetCount() > 120)
		DevtoolsLog().Remove(0);
}

static void EmitKV(AutomationVisitor& av, const String& k, const Value& v)
{
	String line = String().Cat() << k << "=" << AsString(v);
	av.AccessLabel(~line);
}

void EmitAllFocusArtifacts(AutomationVisitor& av)
{
	av.AccessLabel("MLUI Focus Artifacts");
	av.AccessValue(String().Cat()
		<< "pages=" << PageRegistry().GetCount()
		<< ", routes=" << RouteRegistry().GetCount()
		<< ", site_maps=" << SiteMapRegistry().GetCount()
		<< ", links=" << LinkRegistry().GetCount()
		<< ", queries=" << QueryRegistry().GetCount()
		<< ", components=" << ComponentRegistry().GetCount()
		<< ", forms=" << FormRegistry().GetCount()
		<< ", actions=" << ActionRegistry().GetCount()
		<< ", states=" << StateRegistry().GetCount());

	for(int i = 0; i < PageRegistry().GetCount(); i++) {
		const FocusPageDef& p = PageRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusPage[" << p.id << "] " << p.title);
		av.AccessLabel(~p.summary);
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
	for(int i = 0; i < RouteRegistry().GetCount(); i++) {
		const FocusRouteDef& r = RouteRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusRoute[" << r.id << "] " << r.path << " -> " << r.page_id);
	}
	for(int i = 0; i < SiteMapRegistry().GetCount(); i++) {
		const FocusSiteMapDef& s = SiteMapRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusSiteMap[" << s.id << "] routes=" << s.route_ids.GetCount());
	}
	for(int i = 0; i < LinkRegistry().GetCount(); i++) {
		const FocusLinkDef& l = LinkRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusLink[" << l.id << "] " << l.from_route << " -> " << l.to_route);
	}
	for(int i = 0; i < QueryRegistry().GetCount(); i++) {
		const FocusQueryDef& q = QueryRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusQuery[" << q.id << "] " << q.route_id << "." << q.param);
	}
	for(int i = 0; i < ComponentRegistry().GetCount(); i++) {
		const FocusComponentDef& c = ComponentRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusComponent[" << c.id << "] page=" << c.page_id);
	}
	for(int i = 0; i < FormRegistry().GetCount(); i++) {
		const FocusFormDef& f = FormRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusForm[" << f.id << "] action=" << f.action_id << " fields=" << f.fields.GetCount());
	}
	for(int i = 0; i < ActionRegistry().GetCount(); i++) {
		const FocusActionContractDef& a = ActionRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusActionContract[" << a.id << "] page=" << a.page_id);
	}
	for(int i = 0; i < StateRegistry().GetCount(); i++) {
		const FocusStateDef& s = StateRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusState[" << s.id << "] page=" << s.page_id);
		for(int j = 0; j < s.values.GetCount(); j++)
			EmitKV(av, String().Cat() << "state." << s.id << "." << s.values.GetKey(j), s.values[j]);
	}
	for(int i = 0; i < PermissionRegistry().GetCount(); i++) {
		const FocusPermissionDef& p = PermissionRegistry()[i];
		av.AccessLabel(String().Cat() << "FocusPermissions[" << p.action_id << "] enabled="
		                              << (p.enabled ? "true" : "false") << " reason=" << p.disabled_reason);
	}
	if(!HistoryLog().IsEmpty()) {
		const FocusHistoryEntry& h = HistoryLog().Top();
		av.AccessLabel(String().Cat() << "FocusHistory last: " << h.from_route << " -> " << h.to_route << " by " << h.action_id);
	}
	if(!DiffLog().IsEmpty()) {
		const FocusDiffOp& d = DiffLog().Top();
		av.AccessLabel(String().Cat() << "FocusDiff last: " << d.component_id << " " << d.op << " " << d.path);
	}
	if(!DevtoolsLog().IsEmpty()) {
		const FocusDevtoolsEntry& e = DevtoolsLog().Top();
		av.AccessLabel(String().Cat() << "FocusDevtools last: [" << e.level << "] " << e.message);
	}
}

}

INITBLOCK {
	using namespace MLUIRef;

	RegisterFocusPage("issue_board", "Issue Board", "Compact board view with triage actions")
		.Route("issues_list")
		.Component("board_table")
		.Action("select_issue")
		.Action("set_status");

	RegisterFocusPage("issue_editor", "Issue Editor", "Single issue editing view")
		.Route("issue_detail")
		.Component("editor_form")
		.Action("update_issue")
		.Action("close_issue");

	RegisterFocusRoute("issues_list", "/issues", "issue_board", "Open issue list", true);
	RegisterFocusRoute("issue_detail", "/issues/:key", "issue_editor", "Open issue detail");

	RegisterFocusSiteMap("main", "Issue Tracker Focus Map")
		.Route("issues_list")
		.Route("issue_detail");

	RegisterFocusLink("board_to_editor", "issues_list", "issue_detail", "Open selected issue", "selection != null");

	RegisterFocusQuery("q_owner", "issues_list", "owner", "all", "Owner filter");
	RegisterFocusQuery("q_status", "issues_list", "status", "all", "Status filter");
	RegisterFocusQuery("q_search", "issues_list", "search", "", "Text search");

	RegisterFocusComponent("board_table", "issue_board", "Board Table", "Rows with key/title/status/owner")
		.Key("rows")
		.Key("selected_key")
		.Key("filter");
	RegisterFocusComponent("editor_form", "issue_editor", "Issue Form", "Editable fields for selected issue")
		.Key("key")
		.Key("title")
		.Key("status")
		.Key("assignee")
		.Key("blocked")
		.Key("notes");

	RegisterFocusForm("update_issue_form", "issue_editor", "update_issue")
		.Field("status", "enum", true, "Workflow status")
		.Field("assignee", "string", true, "Responsible engineer")
		.Field("notes", "text", false, "Short engineering notes");

	RegisterFocusActionContract("update_issue", "issue_editor",
	                            "Update issue fields",
	                            "{status,assignee,notes}",
	                            "{ok,updated_fields}")
		.SideEffect("state.issue_editor_state.updated_at")
		.SideEffect("diff.editor_form");

	RegisterFocusActionContract("close_issue", "issue_editor",
	                            "Close selected issue",
	                            "{key}",
	                            "{ok,new_status}")
		.SideEffect("history.issue_detail->issues_list")
		.SideEffect("diff.board_table");

	RegisterFocusState("issue_board_state", "issue_board")
		.Set("selected_key", "")
		.Set("owner_filter", "all")
		.Set("status_filter", "all");
	RegisterFocusState("issue_editor_state", "issue_editor")
		.Set("selected_issue", "")
		.Set("dirty", false);

	SetFocusPermission("update_issue", true);
	SetFocusPermission("close_issue", false, "Select issue first");
	PushFocusDevtools("bootstrap", "info", "MLUI focus model initialized");

	MLUI::RegisterFocusPage("issue_board", "Issue Board", "Compact board view with triage actions")
		.Workflow(1, "Open issue list and select target issue")
		.Context("purpose", "Navigate and triage issues quickly")
		.Action("select_issue", "Select issue", "Set active issue from board row")
		.Action("set_status", "Set status", "Update selected issue workflow status");

	MLUI::RegisterFocusPage("issue_editor", "Issue Editor", "Single issue editing view")
		.Workflow(2, "Update selected issue metadata and close when done")
		.DependsOn("issue_board")
		.Context("purpose", "Edit single issue fields")
		.Action("update_issue", "Update issue", "Persist status/assignee/notes from args")
		.Action("close_issue", "Close issue", "Close selected issue when status is Done");
}

MLUIFocusReferenceApp::MLUIFocusReferenceApp()
{
	Title("MLUI Focus Reference");
	Sizeable().Zoomable();
	SetRect(0, 0, 1180, 760);

	board.LayoutId("board");
	board.AddColumn("Key", 90);
	board.AddColumn("Title", 370);
	board.AddColumn("Status", 120);
	board.AddColumn("Owner", 130);
	board.WhenSel = THISBACK(OnBoardSelection);

	route_label.SetLabel("Route");
	issue_label.SetLabel("Issue");
	title_label.SetLabel("Title");
	status_label.SetLabel("Status");
	assignee_label.SetLabel("Assignee");
	blocked.SetLabel("Blocked");

	status.LayoutId("status");
	status.Add("Backlog");
	status.Add("In Progress");
	status.Add("Review");
	status.Add("Done");
	status.WhenAction = THISBACK(OnStatusChange);
	assignee.LayoutId("assignee");
	assignee.WhenAction = THISBACK(OnAssigneeChange);

	editor.Add(route_label.LeftPos(10, 70).TopPos(10, 20));
	editor.Add(route_value.LeftPos(90, 280).TopPos(10, 20));
	editor.Add(issue_label.LeftPos(390, 50).TopPos(10, 20));
	editor.Add(issue_key.LayoutId("issue_key").LeftPos(445, 130).TopPos(10, 20));

	editor.Add(title_label.LeftPos(10, 50).TopPos(40, 20));
	editor.Add(issue_title.LayoutId("issue_title").HSizePos(90, 10).TopPos(40, 20));

	editor.Add(status_label.LeftPos(10, 60).TopPos(70, 20));
	editor.Add(status.LeftPos(90, 160).TopPos(70, 20));
	editor.Add(assignee_label.LeftPos(270, 70).TopPos(70, 20));
	editor.Add(assignee.LeftPos(345, 180).TopPos(70, 20));
	editor.Add(blocked.LeftPos(540, 120).TopPos(70, 20));

	editor.Add(notes.LayoutId("notes").HSizePos(10, 10).VSizePos(100, 10));

	split.Horz(board, editor);
	split.SetPos(3900);
	Add(split.SizePos());

	PopulateBoard();
	if(board.GetCount() > 0)
		board.SetCursor(0);
	RefreshEditorFromSelection();

	MLUI::FocusPage& board_focus = MLUI::GetFocusPage("issue_board");
	ValueMap ex_select_issue;
	ex_select_issue("key", "OVR-204");
	ValueMap ex_set_status;
	ex_set_status("status", "Done");
	board_focus
		.Add(board)
		.Add(route_value)
		.Add(issue_key)
		.ActionRequires("select_issue", "args.key or args.value must match issue key", "Provide existing issue key")
		.ActionRequires("set_status", "row selection must exist", "Select issue first")
		.ActionEffects("select_issue", "updates board_selected_issue; sets issue_editor context")
		.ActionEffects("set_status", "modifies issue status in board; may enable close_issue in issue_editor")
		.ActionArgs("set_status", []{
			ValueArray schema;
			ValueMap status_arg;
			status_arg.Add("name", "status");
			status_arg.Add("type", "string");
			ValueArray status_enum;
			status_enum.Add("Backlog");
			status_enum.Add("In Progress");
			status_enum.Add("Review");
			status_enum.Add("Done");
			status_arg.Add("enum", status_enum);
			schema.Add(status_arg);
			return schema;
		}())
		.ActionExample("select_issue", ex_select_issue, "Select issue by key")
		.ActionExample("set_status", ex_set_status, "Set selected issue status to Done");

	board_focus.ActionHandler("select_issue", [this](const ValueMap& args) -> Value {
		String key = args["key"];
		if(key.IsEmpty())
			key = args["value"];
		if(key.IsEmpty())
			return MLUIRef::ActionError("Missing key/value");

		int found = -1;
		for(int i = 0; i < board.GetCount(); i++) {
			if((String)board.Get(i, 0) == key) {
				found = i;
				break;
			}
		}
		if(found < 0)
			return MLUIRef::ActionError("Issue key not found: " + key);

		board.SetCursor(found);
		RefreshEditorFromSelection();

		ValueMap out;
		out.Add("ok", true);
		out.Add("selected_issue", (String)issue_key.GetData());
		out.Add("route", (String)route_value.GetData());
		return out;
	});

	board_focus.ActionHandler("set_status", [this](const ValueMap& args) -> Value {
		int row = board.GetCursor();
		if(row < 0)
			return MLUIRef::ActionError("No selected issue");

		String new_status = args["status"];
		if(new_status.IsEmpty())
			new_status = args["value"];
		if(new_status.IsEmpty())
			return MLUIRef::ActionError("Missing status/value");

		board.Set(row, 2, new_status);
		status.SetData(new_status);
		OnStatusChange();
		RefreshEditorFromSelection();

		ValueMap out;
		out.Add("ok", true);
		out.Add("status", new_status);
		out.Add("selected_issue", (String)issue_key.GetData());
		return out;
	});

	MLUI::FocusPage& editor_focus = MLUI::GetFocusPage("issue_editor");
	ValueMap ex_update_issue;
	ex_update_issue("status", "Review")("assignee", "ai-agent");
	ValueMap ex_close_issue;
	editor_focus
		.Add(issue_key)
		.Add(issue_title)
		.Add(status)
		.Add(assignee)
		.Add(notes)
		.ActionRequires("update_issue", "row selection must exist", "Select issue first")
		.ActionRequires("close_issue", "status must be Done", "Set status to Done first")
		.ActionEffects("update_issue", "persists assignee and status fields; clears state.dirty")
		.ActionEffects("close_issue", "removes issue from board; decrements board issue count")
		.ActionArgs("update_issue", []{
			ValueArray schema;
			ValueMap status_arg;
			status_arg.Add("name", "status");
			status_arg.Add("type", "string");
			ValueArray status_enum;
			status_enum.Add("Backlog");
			status_enum.Add("In Progress");
			status_enum.Add("Review");
			status_enum.Add("Done");
			status_arg.Add("enum", status_enum);
			schema.Add(status_arg);
			ValueMap assignee_arg;
			assignee_arg.Add("name", "assignee");
			assignee_arg.Add("type", "string");
			schema.Add(assignee_arg);
			ValueMap notes_arg;
			notes_arg.Add("name", "notes");
			notes_arg.Add("type", "text");
			notes_arg.Add("required", false);
			schema.Add(notes_arg);
			return schema;
		}())
		.ActionExample("update_issue", ex_update_issue, "Update issue fields")
		.ActionExample("close_issue", ex_close_issue, "Close selected issue");

	editor_focus.ActionHandler("update_issue", [this](const ValueMap& args) -> Value {
		int row = board.GetCursor();
		if(row < 0)
			return MLUIRef::ActionError("No selected issue");

		String new_status = args["status"];
		String new_assignee = args["assignee"];
		String new_notes = args["notes"];

		if(new_status.IsEmpty() && new_assignee.IsEmpty() && new_notes.IsEmpty())
			return MLUIRef::ActionError("No fields to update");

		if(!new_status.IsEmpty()) {
			board.Set(row, 2, new_status);
			status.SetData(new_status);
			OnStatusChange();
		}
		if(!new_assignee.IsEmpty()) {
			board.Set(row, 3, new_assignee);
			assignee <<= new_assignee;
			OnAssigneeChange();
		}
		if(!new_notes.IsEmpty())
			notes.Set(new_notes);

		ValueMap out;
		out.Add("ok", true);
		out.Add("selected_issue", (String)issue_key.GetData());
		out.Add("status", (String)status.GetData());
		out.Add("assignee", (String)assignee.GetData());
		return out;
	});

	editor_focus.ActionHandler("close_issue", [this](const ValueMap&) -> Value {
		int row = board.GetCursor();
		if(row < 0)
			return MLUIRef::ActionError("No selected issue");
		if((String)status.GetData() != "Done")
			return MLUIRef::ActionError("Issue status must be Done");

		String key = board.Get(row, 0);
		board.Remove(row);

		if(board.GetCount() > 0)
			board.SetCursor(min(row, board.GetCount() - 1));
		RefreshEditorFromSelection();
		MLUIRef::PushFocusHistory(String().Cat() << "/issues/" << key, "/issues", "close_issue");

		ValueMap out;
		out.Add("ok", true);
		out.Add("closed_issue", key);
		out.Add("remaining_rows", board.GetCount());
		return out;
	});

	MLUIRef::PushFocusHistory("", "issues_list", "open_app");
}

void MLUIFocusReferenceApp::PopulateBoard()
{
	board.Clear();
	board.Add("OVR-201", "FocusDiff stream too noisy for large trees", "In Progress", "sblo");
	board.Add("OVR-204", "FocusPermissions missing disabled reason", "Review", "ai-agent");
	board.Add("OVR-207", "Route parser mismatch for /issues/:key", "Backlog", "team");
	board.Add("OVR-210", "Close action should return to issue list", "In Progress", "sblo");
}

void MLUIFocusReferenceApp::OnBoardSelection()
{
	String prev_route;
	{
		MLUIRef::FocusStateDef& st = MLUIRef::GetFocusState("issue_editor_state");
		int i = st.values.Find("route");
		if(i >= 0)
			prev_route = st.values[i];
	}
	RefreshEditorFromSelection();

	MLUIRef::PushFocusHistory(prev_route, "issue_detail", "select_issue");
	MLUIRef::PushFocusDevtools("ui", "info", String().Cat() << "Selected issue " << issue_key.GetData());
}

void MLUIFocusReferenceApp::OnStatusChange()
{
	String st = status.GetData();
	MLUIRef::GetFocusState("issue_editor_state")
		.Set("status", st)
		.Set("dirty", true);
	MLUIRef::PushFocusDiff("editor_form", "replace", "/status", st);
	MLUIRef::PushFocusDevtools("action", "debug", String().Cat() << "Status changed to " << st);

	bool can_close = (st == "Done");
	MLUIRef::SetFocusPermission("close_issue", can_close, can_close ? String() : String("Issue status must be Done"));
}

void MLUIFocusReferenceApp::OnAssigneeChange()
{
	String own = assignee.GetData();
	MLUIRef::GetFocusState("issue_editor_state")
		.Set("assignee", own)
		.Set("dirty", true);
	MLUIRef::PushFocusDiff("editor_form", "replace", "/assignee", own);
	MLUIRef::PushFocusDevtools("action", "debug", String().Cat() << "Assignee changed to " << own);
}

void MLUIFocusReferenceApp::RefreshEditorFromSelection()
{
	int row = board.GetCursor();
	if(row < 0) {
		route_value <<= "/issues";
		issue_key <<= "";
		issue_title <<= "";
		status.SetData("Backlog");
		assignee <<= "";
		blocked = false;
		notes.Clear();

		MLUIRef::GetFocusState("issue_board_state").Set("selected_key", "");
		MLUIRef::GetFocusState("issue_editor_state")
			.Set("route", "/issues")
			.Set("selected_issue", "")
			.Set("dirty", false);
		MLUIRef::SetFocusPermission("close_issue", false, "Select issue first");
		return;
	}

	String key = board.Get(row, 0);
	String title = board.Get(row, 1);
	String st = board.Get(row, 2);
	String owner = board.Get(row, 3);

	route_value <<= String().Cat() << "/issues/" << key;
	issue_key <<= key;
	issue_title <<= title;
	status.SetData(st);
	assignee <<= owner;
	blocked = (st == "Backlog");
	notes.Set(String().Cat() << "Issue " << key << " currently " << st << ".\n"
	                         << "Owner: " << owner << "\n");

	MLUIRef::GetFocusState("issue_board_state").Set("selected_key", key);
	MLUIRef::GetFocusState("issue_editor_state")
		.Set("route", String().Cat() << "/issues/" << key)
		.Set("selected_issue", key)
		.Set("status", st)
		.Set("assignee", owner)
		.Set("dirty", false);

	bool can_close = (st == "Done");
	MLUIRef::SetFocusPermission("close_issue", can_close, can_close ? String() : String("Issue status must be Done"));
	MLUIRef::PushFocusDiff("board_table", "replace", "/selected_key", key);
}

bool MLUIFocusReferenceApp::Access(Visitor& v)
{
	bool base_handled = TopWindow::Access(v);

	if(auto *av = dynamic_cast<AutomationVisitor*>(&v)) {
		MLUIRef::FocusPageDef& board_page = MLUIRef::GetFocusPage("issue_board");
		board_page.ClearRuntime();
		String board_route = route_value.GetData();
		String board_selected_issue = issue_key.GetData();
		bool board_has_selection = board.GetCursor() >= 0;
		MLUI_USE_VAR(board_page, board_route, "Current board route");
		MLUI_USE_VAR(board_page, board_selected_issue, "Currently selected issue key");
		MLUI_USE_CTRL(board_page, board, "Main issue board list control");
		MLUI_USE_STATE(board_page, "selected_key", board_selected_issue, "State mirror of selected key");
		MLUI_USE_ACTION(board_page, "select_issue", board_has_selection, "Select row from board");
		MLUI_USE_ACTION(board_page, "set_status", board_has_selection, "Quick status update from board context");

		MLUIRef::FocusPageDef& editor_page = MLUIRef::GetFocusPage("issue_editor");
		editor_page.ClearRuntime();
		String editor_issue_key = issue_key.GetData();
		String editor_status = status.GetData();
		String editor_assignee = assignee.GetData();
		bool editor_dirty = false;
		{
			MLUIRef::FocusStateDef& st = MLUIRef::GetFocusState("issue_editor_state");
			int i = st.values.Find("dirty");
			if(i >= 0)
				editor_dirty = st.values[i];
		}
		bool editor_has_issue = !editor_issue_key.IsEmpty();
		bool editor_can_close = editor_status == "Done";
		MLUI_USE_VAR(editor_page, editor_issue_key, "Issue key currently open in editor");
		MLUI_USE_VAR(editor_page, editor_status, "Current workflow status");
		MLUI_USE_VAR(editor_page, editor_assignee, "Current assignee");
		MLUI_USE_CTRL(editor_page, status, "Status selector control");
		MLUI_USE_CTRL(editor_page, assignee, "Assignee editor control");
		MLUI_USE_CTRL(editor_page, issue_key, "Current issue key (read-only display)");
		MLUI_USE_CTRL(editor_page, issue_title, "Current issue title (read-only display)");
		MLUI_USE_CTRL(editor_page, notes, "Issue notes editor");
		MLUI_USE_STATE(editor_page, "dirty", editor_dirty, "Unsaved editor state");
		MLUI_USE_ACTION(editor_page, "update_issue", editor_has_issue, "Persist editor changes");
		MLUI_USE_ACTION(editor_page, "close_issue", editor_has_issue && editor_can_close, "Close issue when status is Done");
		MLUI::FocusPage& board_focus = MLUI::GetFocusPage("issue_board");
		board_focus.ClearRuntime();
		MLUI_USE_VAR(board_focus, board_route, "Current board route");
		MLUI_USE_VAR(board_focus, board_selected_issue, "Currently selected issue key");
		MLUI_USE_CTRL(board_focus, board, "Main issue board list control");
		MLUI_USE_STATE(board_focus, "selected_key", board_selected_issue, "State mirror of selected key");
		MLUI_USE_ACTION(board_focus, "select_issue", board_has_selection, "Select row from board");
		MLUI_USE_ACTION(board_focus, "set_status", board_has_selection, "Quick status update from board context");

		MLUI::FocusPage& editor_focus = MLUI::GetFocusPage("issue_editor");
		editor_focus.ClearRuntime();
		MLUI_USE_VAR(editor_focus, editor_issue_key, "Issue key currently open in editor");
		MLUI_USE_VAR(editor_focus, editor_status, "Current workflow status");
		MLUI_USE_VAR(editor_focus, editor_assignee, "Current assignee");
		MLUI_USE_CTRL(editor_focus, status, "Status selector control");
		MLUI_USE_CTRL(editor_focus, assignee, "Assignee editor control");
		MLUI_USE_CTRL(editor_focus, issue_key, "Current issue key (read-only display)");
		MLUI_USE_CTRL(editor_focus, issue_title, "Current issue title (read-only display)");
		MLUI_USE_CTRL(editor_focus, notes, "Issue notes editor");
		MLUI_USE_STATE(editor_focus, "dirty", editor_dirty, "Unsaved editor state");
		MLUI_USE_ACTION(editor_focus, "update_issue", editor_has_issue, "Persist editor changes");
		MLUI_USE_ACTION(editor_focus, "close_issue", editor_has_issue && editor_can_close, "Close issue when status is Done");
		MLUIRef::EmitAllFocusArtifacts(*av);
		return true;
	}

	return base_handled;
}

END_UPP_NAMESPACE

GUI_APP_MAIN
{
	Upp::MLUIFocusReferenceApp().Run();
}
