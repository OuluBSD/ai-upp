#include "AIChat.h"

AIChat::AIChat() {
	Title("AI Chat");
	SetRect(0, 0, 800, 600);
	Sizeable().Zoomable();
	
	Add(chat.SizePos());
}

GUI_APP_MAIN {
	AIChat().Run();
}
