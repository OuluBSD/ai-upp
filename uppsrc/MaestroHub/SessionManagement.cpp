#include "SessionManagement.h"

NAMESPACE_UPP

SessionManagementPane::SessionManagementPane() {
	Add(split.SizePos());
	
	// Left Pane: Directories and Session Lists
	left_pane.Add(dirs.TopPos(0, 150).HSizePos());
	left_pane.Add(list_tabs.VSizePos(150, 0).HSizePos());
	
	dirs.AddColumn("Project Directory");
	dirs.WhenCursor = THISBACK(OnDirCursor);
	
	// Work Sessions Tab with Filter Bar
	filter_bar.Add(filter_type.LeftPos(0, 100).VSizePos(2, 2));
	filter_bar.Add(filter_status.LeftPos(105, 100).VSizePos(2, 2));
	filter_bar.Add(filter_search.HSizePos(210, 0).VSizePos(2, 2));
	
	filter_type.Add("All Types");
	filter_type.Add("work_track");
	filter_type.Add("work_phase");
	filter_type.Add("work_task");
	filter_type.Add("work_issue");
	filter_type.SetIndex(0);
	filter_type.WhenAction = THISBACK(UpdateWorkSessionList);
	
	filter_status.Add("All Status");
	filter_status.Add("running");
	filter_status.Add("paused");
	filter_status.Add("completed");
	filter_status.Add("failed");
	filter_status.SetIndex(0);
	filter_status.WhenAction = THISBACK(UpdateWorkSessionList);
	
	filter_search.NullText("Search purpose...");
	filter_search.WhenAction = THISBACK(UpdateWorkSessionList);
	
	work_pane.Add(filter_bar.TopPos(0, 30).HSizePos());
	work_pane.Add(work_sessions.VSizePos(30, 0).HSizePos());
	
	work_sessions.AddColumn("ID");
	work_sessions.AddColumn("Type");
	work_sessions.AddColumn("Status");
	work_sessions.AddColumn("Purpose");
	work_sessions.WhenCursor = THISBACK(OnWorkSessionCursor);
	work_sessions.WhenLeftDouble = [=] {
		if(work_sessions.IsCursor() && WhenSelect)
			WhenSelect("maestro", work_sessions.Get(0));
	};
	
	list_tabs.Add(work_pane.SizePos(), "Maestro Work Sessions");
	list_tabs.Add(sessions.SizePos(), "AI Backend Sessions");
	
	// Right Pane: Details
	detail_split.Vert(context_view, breadcrumbs);
	detail_tabs.Add(detail_split.SizePos(), "Session Details");
	
	breadcrumbs.AddColumn("Time");
	breadcrumbs.AddColumn("Model");
	breadcrumbs.AddColumn("Prompt / Response");
	
	split.Horz(left_pane, detail_tabs);
	split.SetPos(4000); // Give list more space initially
	
	sessions.WhenBar = THISBACK(OnSessionMenu);
	sessions.WhenLeftDouble = [=] {
		if(sessions.IsCursor() && WhenSelect)
			WhenSelect(engine.binary, sessions.Get(0));
	};
}

void SessionManagementPane::Load(const String& root) {
	current_root = root;
	
	dirs.Clear();
	dirs.Add(current_root);
	dirs.Add(AppendFileName(GetHomeDirectory(), ".gemini/tmp"));
	dirs.Add(AppendFileName(GetHomeDirectory(), ".qwen/projects"));
	
	if(dirs.GetCount() > 0) dirs.SetCursor(0);
}

void SessionManagementPane::OnDirCursor() {
	sessions.Clear();
	if(!dirs.IsCursor()) return;
	
	String dir = dirs.Get(0);
	
	UpdateWorkSessionList();

	String backend = "qwen";
	if(dir.Find(".gemini") >= 0) backend = "gemini";
	else if(dir.Find(".claude") >= 0) backend = "claude";
	else if(dir.Find(".codex") >= 0) backend = "codex";
	
	engine.binary = backend;
	engine.project_sessions.Clear();
	engine.ListSessions(dir, [this](const Array<SessionInfo>& s) {
		sessions.SetSessions(s);
	});
}

void SessionManagementPane::UpdateWorkSessionList() {
	work_sessions.Clear();
	if(!dirs.IsCursor()) return;
	String dir = dirs.Get(0);
	
	Array<WorkSession> ws_list = WorkSessionManager::ListSessions(dir);
	
	String type_filter = filter_type.GetData().ToString();
	String status_filter = filter_status.GetData().ToString();
	String search = ToLower(filter_search.GetData().ToString());
	
	for(const auto& s : ws_list) {
		if(type_filter != "All Types" && s.session_type != type_filter) continue;
		if(status_filter != "All Status" && StatusToString(s.status) != status_filter) continue;
		if(!search.IsEmpty() && ToLower(s.purpose).Find(search) < 0 && s.session_id.Find(search) < 0) continue;
		
		work_sessions.Add(s.session_id, s.session_type, StatusToString(s.status), s.purpose);
	}
}

void SessionManagementPane::OnWorkSessionCursor() {
	context_view.SetQTF("");
	breadcrumbs.Clear();
	
	if(!work_sessions.IsCursor()) return;
	
	String dir = dirs.Get(0);
	String sid = work_sessions.Get(0);
	String path = WorkSessionManager::FindSessionPath(dir, sid);
	
	WorkSession s;
	if(WorkSessionManager::LoadSession(path, s)) {
		String qtf;
		qtf << "[* ID:] " << s.session_id << "&";
		qtf << "[* Created:] " << s.created << "&";
		qtf << "[* Related:] " << StoreAsJson(s.related_entity) << "&";
		qtf << "[* Context:] " << StoreAsJson(s.context) << "&";
		context_view.SetQTF(qtf);
		
		Array<Breadcrumb> bc_list = BreadcrumbManager::ListBreadcrumbs(dir, sid);
		for(const auto& bc : bc_list) {
			String summary = bc.prompt.Left(50) + " -> " + bc.response.Left(50);
			breadcrumbs.Add(bc.timestamp_id, bc.model_used, summary);
		}
	}
}

void SessionManagementPane::OnSessionMenu(Bar& bar) {
	if(!sessions.IsCursor()) return;
	bar.Add("Delete", THISBACK(DeleteSession));
}

void SessionManagementPane::DeleteSession() {
	if(!sessions.IsCursor()) return;
	String id = sessions.Get(0);
	if(PromptYesNo("Delete session " + id + "?")) {
		sessions.Remove(sessions.GetCursor());
	}
}

END_UPP_NAMESPACE
