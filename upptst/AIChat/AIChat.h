#ifndef _AIChat_AIChat_h_
#define _AIChat_AIChat_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

using namespace Upp;

class AIChat : public TopWindow {
public:
	TabCtrl    tabs;
	Array<AIChatCtrl> chat_tabs;
	
	void NewSession();
	void CreateSession(const String& backend, const String& model, const String& working_dir);
	void CloseSession();
	void CloseAllSessions();
	void SendCurrent();

private:
	MenuBar    menu;
	RecentConfig      config;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void EditMenu(Bar& bar);
	
	virtual bool Key(dword key, int count) override;

public:
	typedef AIChat CLASSNAME;
	AIChat();
};

#endif