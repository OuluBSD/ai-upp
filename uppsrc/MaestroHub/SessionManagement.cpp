#include "MaestroHub.h"

NAMESPACE_UPP

SessionManagementPane::SessionManagementPane() {
	Add(split.SizePos());
	split.Horz(dirs, list_tabs);
	
	list_tabs.Add(sessions.SizePos(), "AI Backend Sessions");
	list_tabs.Add(work_sessions.SizePos(), "Maestro Work Sessions");
	
	dirs.AddColumn("Project Directory");
	dirs.WhenCursor = THISBACK(OnDirCursor);
	
	work_sessions.AddColumn("ID");
	work_sessions.AddColumn("Type");
	work_sessions.AddColumn("Status");
	work_sessions.AddColumn("Purpose");
	work_sessions.WhenLeftDouble = [=] {
		if(work_sessions.IsCursor() && WhenSelect)
			WhenSelect("maestro", work_sessions.Get(0));
	};
	
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
	work_sessions.Clear();
	if(!dirs.IsCursor()) return;
	
	String dir = dirs.Get(0);
	
	// Load Maestro Work Sessions
	Array<WorkSession> ws_list = WorkSessionManager::ListSessions(dir);
	for(const auto& s : ws_list) {
		work_sessions.Add(s.session_id, s.session_type, StatusToString(s.status), s.purpose);
	}

	String backend = "qwen";
	if(dir.Find(".gemini") >= 0) backend = "gemini";
	else if(dir.Find(".claude") >= 0) backend = "claude";
	else if(dir.Find(".codex") >= 0) backend = "codex";
	
	engine.binary = backend;
	
	// Reset engine internal state before scanning
	engine.project_sessions.Clear();
	
	engine.ListSessions(dir, [this](const Array<SessionInfo>& s) {
		sessions.SetSessions(s);
	});
	
	if(sessions.GetCount() == 0 && engine.project_sessions.GetCount() > 0) {
		for(int i = 0; i < engine.project_sessions.GetCount(); i++) {
			sessions.SetSessions(engine.project_sessions[i]);
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