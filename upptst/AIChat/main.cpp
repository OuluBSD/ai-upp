#include "AIChat.h"

AIChat::AIChat() {
	Title("Maestro AI Chat");
	SetRect(0, 0, 1000, 700);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
	config.Load();
}

void AIChat::CreateSession(const String& backend, const String& model, const String& working_dir) {
	AIChatCtrl& c = chat_tabs.Add();
	c.backend = backend;
	c.engine.model = model;
	c.engine.working_dir = working_dir;
	tabs.Add(c.SizePos(), backend + (model.IsEmpty() ? "" : " [" + model + "]"));
	tabs.Set(tabs.GetCount() - 1);
}

void AIChat::NewSession() {
	NewSessionWindow sw(config);
	if(sw.Run() == IDOK) {
		CreateSession(sw.selected_backend, "", sw.selected_dir);
		if(!sw.session_id.IsEmpty()) {
			chat_tabs.Top().engine.session_id = sw.session_id;
			chat_tabs.Top().AddItem("System", "Resumed session: " + sw.session_id);
		}
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
	AIChat chat;
	
	const Vector<String>& cmdline = CommandLine();
	String backend, model, input;
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			backend = cmdline[++i];
		else if(cmdline[i] == "--model" && i + 1 < cmdline.GetCount())
			model = cmdline[++i];
		else if(cmdline[i] == "--input" && i + 1 < cmdline.GetCount())
			input = cmdline[++i];
	}
	
	if(!backend.IsEmpty()) {
		chat.PostCallback([=, &chat] {
			chat.CreateSession(backend, model, GetCurrentDirectory());
			if(!input.IsEmpty()) {
				AIChatCtrl& c = chat.chat_tabs.Top();
				c.input.SetData(input);
				
				c.WhenDone = [&chat, &c] {
					Cout() << "=== TEST RESULT DEBUG DUMP ===\n";
					Cout() << c.engine.debug_log << "\n";
					Cout() << "=== END DUMP ===\n";
					Cout().Flush();
					chat.PostCallback([&chat] { chat.Close(); });
				};
				
				c.OnSend();
				
				chat.SetTimeCallback(60000, [&chat, &c] {
					Cout() << "ERROR: Test timed out after 60s\n";
					Cout() << "=== ENGINE LOG AT TIMEOUT ===\n";
					Cout() << c.engine.debug_log << "\n";
					Cout() << "=== END DUMP ===\n";
					Cout().Flush();
					chat.PostCallback([&chat] { chat.Close(); });
				});
			}
		});
	} else {
		chat.PostCallback([&chat] { chat.NewSession(); });
	}
	
	chat.Run();
}