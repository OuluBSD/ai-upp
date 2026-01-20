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
