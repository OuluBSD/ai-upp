#ifndef _AIChat_AIChat_h_
#define _AIChat_AIChat_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

using namespace Upp;

class AIChat : public TopWindow {
	MenuBar    menu;
	TabCtrl    tabs;
	
	Array<AIChatCtrl> chat_tabs;
	RecentConfig      config;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void EditMenu(Bar& bar);
	
	void NewSession();
	void CloseSession();
	void CloseAllSessions();
	void SendCurrent();
	
	virtual bool Key(dword key, int count) override;

public:
	typedef AIChat CLASSNAME;
	AIChat();
};

#endif
