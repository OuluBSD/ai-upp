#include "AIChat.h"

AIChat::AIChat() {
	Title("Maestro AI Chat");
	SetRect(0, 0, 1000, 700);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
tabs.Add(chat.SizePos(), "Chat Playground");
	
system_splitter.Horz() << repo_view << plan_view;
	tabs.Add(system_splitter.SizePos(), "System");
	
	// Initialize System Data
	String root = GetFileDirectory(GetExeFilePath());
	// Search upwards for .git or uppsrc to find repo root
	while(!root.IsEmpty() && !DirectoryExists(AppendFileName(root, "uppsrc"))) {
		String next = GetFileDirectory(root);
		if(next == root) break;
		root = next;
	}
	
	if(!root.IsEmpty()) {
		RepoScanner rs;
		rs.Scan(root);
		rs.DetectAssemblies();
		repo_view.Set(rs.assemblies, rs.packages);
		
		PlanParser pp;
		pp.Load(AppendFileName(root, "uppsrc/AI/plan"));
		plan_view.Set(pp.tracks);
	}
	
	// Automated testing support
	const Vector<String>& cmdline = CommandLine();
	String backend;
	String input;
	bool dump_sessions = false;
	bool test_session_window = false;
	
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			backend = cmdline[++i];
		else if(cmdline[i] == "--input" && i + 1 < cmdline.GetCount())
			input = cmdline[++i];
		else if(cmdline[i] == "--dump-sessions")
			dump_sessions = true;
		else if(cmdline[i] == "--test-session-window")
			test_session_window = true;
	}
	
	if(test_session_window && !backend.IsEmpty()) {
		chat.engine_select.SetData(backend);
		PostCallback([=] {
			chat.OnSelectSession();
		});
		SetTimeCallback(2000, [=] {
			PostCallback(callback(const_cast<AIChat*>(this), &TopWindow::Close));
		});
	}
	
	if(dump_sessions && !backend.IsEmpty()) {
		chat.engine_select.SetData(backend);
		if(backend == "qwen") ConfigureQwen(chat.engine);
		
		chat.engine.ListSessions(GetFileDirectory(GetExeFilePath()), [](const Array<SessionInfo>&) {});
		
		Cout() << "=== SESSION DUMP ===\n";
		for(int i = 0; i < chat.engine.project_sessions.GetCount(); i++) {
			Cout() << "Project: " << chat.engine.project_sessions.GetKey(i) << "\n";
			for(const auto& s : chat.engine.project_sessions[i]) {
				Cout() << "  Session: " << s.id << " (" << s.name << ")\n";
			}
		}
		Cout() << "=== END DUMP ===\n";
		Cout().Flush();
		Exit(0);
	}
	
	if(!backend.IsEmpty() && !input.IsEmpty()) {
		chat.engine_select.SetData(backend);
		
		static Vector<String> inputs;
		inputs = Split(input, ';');
		
		chat.WhenDone = [=] {
			static int idx = 0;
			idx++;
			if(idx < inputs.GetCount()) {
				chat.input.SetData(inputs[idx]);
				chat.OnSend();
			} else {
				Cout() << "=== TEST RESULT DEBUG DUMP ===\n";
				Cout() << chat.engine.debug_log << "\n";
				Cout() << "=== END DUMP ===\n";
				Cout().Flush();
				PostCallback(callback(const_cast<AIChat*>(this), &TopWindow::Close));
			}
		};
		
		chat.input.SetData(inputs[0]);
		PostCallback([=] { chat.OnSend(); });
		
		// Global test timeout (30 seconds)
		SetTimeCallback(30000, [=] {
			Cout() << "ERROR: Test timed out after 30s\n";
			Cout() << "=== PARTIAL DEBUG DUMP ===\n";
			Cout() << chat.engine.debug_log << "\n";
			Cout() << "=== END DUMP ===\n";
			Cout().Flush();
			PostCallback(callback(const_cast<AIChat*>(this), &TopWindow::Close));
		});
	}
}

void AIChat::MainMenu(Bar& bar) {
	bar.Sub("Edit", [=](Bar& bar) {
		bar.Add("Select Session", [=] { chat.OnSelectSession(); });
		bar.Add("Copy All Chat", [=] { chat.CopyAllChat(); });
		bar.Add("Copy Debug Data", [=] { chat.CopyDebugData(); });
	});
}

GUI_APP_MAIN {
	AIChat().Run();
}
