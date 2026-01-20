#include "AIChat.h"

AIChat::AIChat() {
	Title("Maestro AI Chat");
	SetRect(0, 0, 800, 600);
	Sizeable().Zoomable();
	
	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));
	
	Add(tabs.SizePos());
	
	tabs.Add(chat.SizePos(), "Chat Playground");
	
	system_view.SetText("System Internals Visualization (Placeholder)");
	system_view.SetAlign(ALIGN_CENTER);
	tabs.Add(system_view.SizePos(), "System");
	
	// Automated testing support
	const Vector<String>& cmdline = CommandLine();
	String backend;
	String input;
	
	for(int i = 0; i < cmdline.GetCount(); i++) {
		if(cmdline[i] == "--backend" && i + 1 < cmdline.GetCount())
			backend = cmdline[++i];
		else if(cmdline[i] == "--input" && i + 1 < cmdline.GetCount())
			input = cmdline[++i];
	}
	
	if(!backend.IsEmpty() && !input.IsEmpty()) {
		chat.engine_select.SetData(backend);
		chat.input.SetData(input);
		
		chat.WhenDone = [=] {
			Cout() << "=== TEST RESULT DEBUG DUMP ===\n";
			Cout() << chat.engine.debug_log << "\n";
			Cout() << "=== END DUMP ===\n";
			PostCallback(callback(this, &TopWindow::Close));
		};
		
		PostCallback([=] { chat.OnSend(); });
	}
}

void AIChat::MainMenu(Bar& bar) {
	bar.Sub("Edit", [=](Bar& bar) {
		bar.Add("Copy All Chat", [=] { chat.CopyAllChat(); });
		bar.Add("Copy Debug Data", [=] { chat.CopyDebugData(); });
	});
}

GUI_APP_MAIN {
	AIChat().Run();
}