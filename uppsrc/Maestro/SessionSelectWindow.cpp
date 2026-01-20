#include "Maestro.h"
#include "SessionSelectWindow.h"

namespace Upp {

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

}