#include "SessionManagement.h"

NAMESPACE_UPP

SessionManagementPane::SessionManagementPane() {
	Add(split.SizePos());
	split.Horz(dirs, sessions);
	
	dirs.AddColumn("Project Directory");
	dirs.WhenCursor = THISBACK(OnDirCursor);
	
	sessions.WhenBar = THISBACK(OnSessionMenu);
}

void SessionManagementPane::Load(const String& root) {
	current_root = root;
	
	dirs.Clear();
	dirs.Add(AppendFileName(GetHomeDirectory(), ".gemini/tmp"));
	dirs.Add(AppendFileName(GetHomeDirectory(), ".qwen/projects"));
	dirs.Add(AppendFileName(GetHomeDirectory(), ".claude/projects"));
	
	if(dirs.GetCount() > 0) dirs.SetCursor(0);
}

void SessionManagementPane::OnDirCursor() {
	sessions.Clear();
	if(!dirs.IsCursor()) return;
	
	String dir = dirs.Get(0);
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