#include "Maestro.h"

#ifdef flagGUI

NAMESPACE_UPP

SessionSelectWindow::SessionSelectWindow(CliMaestroEngine& e) {
	engine = &e;
	Title("Select AI Session");
	SetRect(0, 0, 800, 500);
	
	Add(split.SizePos());
	split.Horz(dirs, sessions);
	
	dirs.AddColumn("Project Directory");
	sessions.AddColumn("Session ID");
	sessions.AddColumn("Title");
	sessions.AddColumn("Time");
	
	dirs.WhenCursor = THISBACK(OnDirCursor);
	sessions.WhenLeftDouble = THISBACK(OnSessionDouble);
}

void SessionSelectWindow::DataDirectories() {
	dirs.Clear();
	for(int i = 0; i < engine->project_sessions.GetCount(); i++)
		dirs.Add(engine->project_sessions.GetKey(i));
	if(dirs.GetCount() > 0) dirs.SetCursor(0);
}

void SessionSelectWindow::OnDirCursor() {
	sessions.Clear();
	if(!dirs.IsCursor()) return;
	
	String dir = dirs.Get(0);
	int q = engine->project_sessions.Find(dir);
	if(q >= 0) {
		for(const auto& s : engine->project_sessions[q])
			sessions.Add(s.id, s.name, s.timestamp);
	}
}

void SessionSelectWindow::OnSessionDouble() {
	if(!sessions.IsCursor()) return;
	selected_id = sessions.Get(0);
	Break(IDOK);
}

NewSessionWindow::NewSessionWindow(RecentConfig& cfg) : config(cfg) {
	Title("New AI Session");
	SetRect(0, 0, 500, 200);
	
	Add(dir.TopPos(10, 20).HSizePos(10, 100));
	Add(recent_btn.SetLabel("...").TopPos(10, 20).RightPos(10, 80));
	Add(backend.TopPos(40, 20).HSizePos(10, 10));
	
	Add(btn_new.SetLabel("New Session").BottomPos(10, 30).LeftPos(10, 150));
	Add(btn_resume.SetLabel("Resume Existing...").BottomPos(10, 30).LeftPos(170, 150));
	Add(btn_cancel.SetLabel("Cancel").BottomPos(10, 30).RightPos(10, 100));
	
	backend.Add("gemini", "Gemini");
	backend.Add("qwen", "Qwen");
	backend.Add("claude", "Claude");
	backend.Add("codex", "Codex");
	backend.SetIndex(0);
	
	dir.SetData(GetCurrentDirectory());
	
	recent_btn << THISBACK(OnRecent);
	btn_new << THISBACK(OnNew);
	btn_resume << THISBACK(OnResume);
	btn_cancel << [=] { Break(IDCANCEL); };
}

void NewSessionWindow::OnRecent() {
	String d = SelectDirectory();
	if(!d.IsEmpty()) dir.SetData(d);
}

void NewSessionWindow::OnNew() {
	selected_backend = backend.GetData();
	selected_dir = dir.GetData();
	config.AddDir(selected_dir);
	Break(IDOK);
}

void NewSessionWindow::OnResume() {
	selected_backend = backend.GetData();
	selected_dir = dir.GetData();
	config.AddDir(selected_dir);
	
	CliMaestroEngine e;
	e.binary = selected_backend;
	// We need to list sessions first
	e.ListSessions(selected_dir, [this, &e](const Array<SessionInfo>& sessions) {
		SessionSelectWindow sw(e);
		sw.DataDirectories();
		if(sw.Run() == IDOK) {
			session_id = sw.selected_id;
			Break(IDOK);
		}
	});
}

END_UPP_NAMESPACE

#endif
