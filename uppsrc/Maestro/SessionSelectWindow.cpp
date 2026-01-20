#include "Maestro.h"
#include "SessionSelectWindow.h"

NAMESPACE_UPP

SessionSelectWindow::SessionSelectWindow(CliMaestroEngine& e) : engine(&e) {
	Title("Select AI Session");
	SetRect(0, 0, 800, 500);
	Sizeable().Zoomable();
	
	Add(split.SizePos());
	split.Horz(dirs, sessions);
	
	dirs.AddColumn("Directory");
	dirs.AddIndex("IDX"); // For internal mapping
	
	sessions.AddIndex("ID");
	sessions.AddColumn("Date");
	sessions.AddColumn("Session Name");
	
	dirs.WhenCursor = [=] { OnDirCursor(); };
	sessions.WhenLeftDouble = [=] { OnSessionDouble(); };
}

void SessionSelectWindow::DataDirectories() {
	dirs.Clear();
	sessions.Clear();
	if(!engine) return;
	
	// Ensure we have scanned projects
	engine->ListSessions("", [](const Array<SessionInfo>&) {}); // Trigger scan
	
	for(int i = 0; i < engine->project_sessions.GetCount(); i++) {
		dirs.Add(engine->project_sessions.GetKey(i), i);
	}
	
	if(dirs.GetCount() > 0) {
		dirs.SetCursor(0);
	}
}

void SessionSelectWindow::Load(CliMaestroEngine& e) {
	engine = &e;
	DataDirectories();
}

void SessionSelectWindow::OnDirCursor() {
	sessions.Clear();
	if(!dirs.IsCursor() || !engine) return;
	
	int idx = dirs.Get("IDX");
	if(idx < 0 || idx >= engine->project_sessions.GetCount()) return;
	
	const Array<SessionInfo>& s_list = engine->project_sessions[idx];
	
	for(const auto& s : s_list) {
		sessions.Add(s.id, Format(s.timestamp), s.name);
	}
}

void SessionSelectWindow::OnSessionDouble() {
	if(!sessions.IsCursor()) return;
	selected_id = sessions.Get("ID");
	Break(IDOK);
}

// NewSessionWindow implementation

NewSessionWindow::NewSessionWindow(RecentConfig& cfg) : config(cfg) {
	Title("New AI Session");
	SetRect(0, 0, 500, 180);
	
	dir.Tip("Working directory");
	recent_btn.SetLabel("Recent").Tip("Show recent directories");
	backend.Tip("AI Backend");
	
	backend.Add("gemini", "Gemini");
	backend.Add("qwen", "Qwen");
	backend.Add("claude", "Claude");
	backend.Add("codex", "Codex");
	backend.SetIndex(0);
	
	btn_new.SetLabel("New Session");
	btn_resume.SetLabel("Resume Session");
	btn_cancel.SetLabel("Cancel");
	
	Add(dir.HSizePos(10, 100).TopPos(10, 25));
	Add(recent_btn.RightPos(10, 80).TopPos(10, 25));
	Add(backend.HSizePos(10, 10).TopPos(45, 25));
	
	Add(btn_cancel.LeftPos(10, 100).BottomPos(10, 25));
	Add(btn_resume.HCenterPos(120).BottomPos(10, 25));
	Add(btn_new.RightPos(10, 100).BottomPos(10, 25));
	
	recent_btn << [=] { OnRecent(); };
	btn_new << [=] { OnNew(); };
	btn_resume << [=] { OnResume(); };
	btn_cancel << [=] { Break(IDCANCEL); };
	
	dir.SetData(GetCurrentDirectory());
}

void NewSessionWindow::OnRecent() {
	MenuBar bar;
	for(const String& d : config.recent_dirs) {
		bar.Add(d, [=] { dir.SetData(d); });
	}
	bar.Execute();
}

void NewSessionWindow::OnNew() {
	if(dir.GetData().ToString().IsEmpty()) return;
	selected_dir = dir.GetData().ToString();
	selected_backend = backend.GetValue().ToString();
	config.AddDir(selected_dir);
	Break(IDOK);
}

void NewSessionWindow::OnResume() {
	if(dir.GetData().ToString().IsEmpty()) return;
	selected_dir = dir.GetData().ToString();
	selected_backend = backend.GetValue().ToString();
	config.AddDir(selected_dir);
	
	CliMaestroEngine engine;
	String key = selected_backend;
	if(key == "gemini") ConfigureGemini(engine);
	else if(key == "qwen") ConfigureQwen(engine);
	else if(key == "claude") ConfigureClaude(engine);
	else if(key == "codex") ConfigureCodex(engine);
	
	SessionSelectWindow sw(engine);
	sw.DataDirectories();
	
	// Pre-select the directory if it exists in scanned projects
	int q = sw.dirs.Find(dir.GetData().ToString());
	if(q >= 0) {
		sw.dirs.SetCursor(q);
	}
	
	if(sw.Run() == IDOK) {
		session_id = sw.selected_id;
		Break(IDOK);
	}
}

END_UPP_NAMESPACE