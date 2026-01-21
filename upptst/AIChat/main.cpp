#include "AIChat.h"

AIChat::AIChat() {
	Title("Maestro AI Chat");
	SetRect(0, 0, 1000, 700);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
	config.Load();
	
	// Automated testing support
	const Vector<String>& cmdline = CommandLine();
	String backend;
	String input;
	bool dump_sessions = false;
	bool test_session_window = false;
	bool test_new_session = false;
	bool test_new_tab_dialog = false;
	
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			backend = cmdline[++i];
		else if(cmdline[i] == "--input" && i + 1 < cmdline.GetCount())
			input = cmdline[++i];
		else if(cmdline[i] == "--dump-sessions")
			dump_sessions = true;
		else if(cmdline[i] == "--test-session-window")
			test_session_window = true;
		else if(cmdline[i] == "--test-new-session")
			test_new_session = true;
		else if(cmdline[i] == "--test-new-tab-dialog")
			test_new_tab_dialog = true;
	}
	
	if(test_new_tab_dialog && !backend.IsEmpty()) {
		PostCallback([=] {
			NewSessionWindow sw(config);
			sw.backend.SetData(backend);
			sw.dir.SetData(GetFileDirectory(GetExeFilePath()));
			PostCallback([&sw] { sw.OnNew(); }); // Simulate clicking "New Session"
			
			if(sw.Run() == IDOK) {
				String b = sw.selected_backend;
				String d = sw.selected_dir;
				
				AIChatCtrl& chat = chat_tabs.Add();
				chat.backend = b;
				chat.engine.working_dir = d;
				tabs.Add(chat.SizePos(), b);
				tabs.Set(tabs.GetCount() - 1);
				
				chat.AddItem("System", "Started via Dialog Test");
				chat.input.SetData("test via dialog");
				chat.OnSend();
				
				SetTimeCallback(10000, [=] {
					if(chat_tabs.GetCount() > 0) {
						Cout() << "=== ENGINE LOG ===\n";
						Cout() << chat_tabs.Top().engine.debug_log << "\n";
						Cout() << "=== END DUMP ===\n";
						Cout().Flush();
					}
					PostCallback([=] { Close(); });
				});
			} else {
				Cout() << "ERROR: Dialog did not return IDOK\n";
				Cout().Flush();
				Close();
			}
		});
	}
	
	if(dump_sessions && !backend.IsEmpty()) {
		CliMaestroEngine engine;
		if(backend == "qwen") ConfigureQwen(engine);
		else if(backend == "gemini") ConfigureGemini(engine);
		
		engine.ListSessions(GetFileDirectory(GetExeFilePath()), [](const Array<SessionInfo>&) {});
		
		Cout() << "=== SESSION DUMP ===\n";
		for(int i = 0; i < engine.project_sessions.GetCount(); i++) {
			Cout() << "Project: " << engine.project_sessions.GetKey(i) << "\n";
			for(const auto& s : engine.project_sessions[i]) {
				Cout() << "  Session: " << s.id << " (" << s.name << ")\n";
			}
		}
		Cout() << "=== END DUMP ===\n";
		Cout().Flush();
		Exit(0);
	}
	
	if(test_session_window && !backend.IsEmpty()) {
		PostCallback([=] {
			NewSession();
		});
		SetTimeCallback(2000, [=] {
			PostCallback([=] { Close(); });
		});
	}
	
	if(test_new_session && !backend.IsEmpty()) {
		PostCallback([=] {
			Cout() << "DEBUG: Starting test-new-session for backend: " << backend << "\n";
			AIChatCtrl& chat = chat_tabs.Add();
			chat.backend = backend;
			chat.engine.working_dir = GetFileDirectory(GetExeFilePath());
			tabs.Add(chat.SizePos(), backend);
			tabs.Set(tabs.GetCount() - 1);
			Cout() << "DEBUG: Tab added, count: " << tabs.GetCount() << "\n";
			Cout().Flush();
			
			chat.AddItem("System", "Starting automated test...");
			chat.input.SetData("test message for " + backend);
			chat.OnSend();
			
			SetTimeCallback(15000, [=] {
				if(chat_tabs.GetCount() > 0) {
					Cout() << "=== ENGINE LOG ===\n";
					Cout() << chat_tabs.Top().engine.debug_log << "\n";
					Cout() << "=== END DUMP ===\n";
					Cout().Flush();
				}
				PostCallback([=] { Close(); });
			});
		});
	}
	
	if(!backend.IsEmpty() && !input.IsEmpty() && !test_new_session) {
		PostCallback([=] {
			AIChatCtrl& chat = chat_tabs.Add();
			chat.backend = backend;
			chat.engine.working_dir = GetFileDirectory(GetExeFilePath());
			tabs.Add(chat.SizePos(), backend);
			tabs.Set(tabs.GetCount() - 1);
			
			static Vector<String> inputs;
			inputs = Split(input, ';');
			
			chat.WhenDone = [=] {
				static int idx = 0;
				idx++;
				if(idx < inputs.GetCount()) {
					chat_tabs.Top().input.SetData(inputs[idx]);
					chat_tabs.Top().OnSend();
				} else {
					Cout() << "=== TEST RESULT DEBUG DUMP ===\n";
					Cout() << chat_tabs.Top().engine.debug_log << "\n";
					Cout() << "=== END DUMP ===\n";
					Cout().Flush();
					PostCallback([=] { Close(); });
				}
			};
			
			chat.input.SetData(inputs[0]);
			chat.OnSend();
		});
		
		SetTimeCallback(30000, [=] {
			Cout() << "ERROR: Test timed out after 30s\n";
			Cout().Flush();
			PostCallback([=] { Close(); });
		});
	}

	if(cmdline.GetCount() == 0) {
		// Create initial session
		PostCallback([=] { NewSession(); });
	}
}

void AIChat::NewSession() {
	NewSessionWindow sw(config);
	if(sw.Run() == IDOK) {
		String backend = sw.selected_backend;
		String dir = sw.selected_dir;
		String sid = sw.session_id;
		
		if (dir.Right(1) == DIR_SEPS) dir = dir.Left(dir.GetCount()-1);
		
		AIChatCtrl& chat = chat_tabs.Add();
		chat.backend = backend;
		chat.engine.working_dir = dir;
		if(!sid.IsEmpty()) {
			chat.engine.session_id = sid;
			chat.AddItem("System", "Resumed session: " + sid);
		}
		
		tabs.Add(chat.SizePos(), backend + " (" + GetFileName(dir) + ")");
		tabs.Set(tabs.GetCount() - 1);
	}
}

void AIChat::CloseSession() {
	int idx = tabs.Get();
	if(idx >= 0) {
		tabs.Remove(idx);
		chat_tabs.Remove(idx);
	}
}

void AIChat::CloseAllSessions() {
	while(tabs.GetCount() > 0) {
		tabs.Remove(0);
		chat_tabs.Remove(0);
	}
}

void AIChat::SendCurrent() {
	int idx = tabs.Get();
	if(idx >= 0) {
		chat_tabs[idx].OnSend();
	}
}

bool AIChat::Key(dword key, int count) {
	if(key == K_CTRL_T) {
		NewSession();
		return true;
	}
	if(key == K_CTRL_W) {
		CloseSession();
		return true;
	}
	if(key == K_F5) {
		SendCurrent();
		return true;
	}
	return TopWindow::Key(key, count);
}

void AIChat::AppMenu(Bar& bar) {
	bar.Add("New Session", THISBACK(NewSession)).Key(K_CTRL_T);
	bar.Add("Close Session", THISBACK(CloseSession)).Key(K_CTRL_W);
	bar.Add("Close All", THISBACK(CloseAllSessions));
	bar.Separator();
	bar.Add("Exit", THISBACK(Close));
}

void AIChat::EditMenu(Bar& bar) {
	int idx = tabs.Get();
	if(idx >= 0) {
		AIChatCtrl& chat = chat_tabs[idx];
		bar.Add("Send", [=, &chat] { chat.OnSend(); }).Key(K_F5);
		bar.Separator();
		bar.Add("Copy All Chat", [=, &chat] { chat.CopyAllChat(); });
		bar.Add("Copy Debug Data", [=, &chat] { chat.CopyDebugData(); });
	}
}

void AIChat::MainMenu(Bar& bar) {
	bar.Sub("App", THISBACK(AppMenu));
	bar.Sub("Edit", THISBACK(EditMenu));
}

GUI_APP_MAIN {
	AIChat().Run();
}
