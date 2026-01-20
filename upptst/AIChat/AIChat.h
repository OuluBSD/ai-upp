#ifndef _AIChat_AIChat_h_
#define _AIChat_AIChat_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

using namespace Upp;

class AIChat : public TopWindow {
	MenuBar    menu;
	TabCtrl    tabs;
	AIChatCtrl chat;
	
	Splitter   system_splitter;
	RepoView   repo_view;
	PlanView   plan_view;
	
	void MainMenu(Bar& bar);
	
public:
	typedef AIChat CLASSNAME;
	AIChat();
};

#endif