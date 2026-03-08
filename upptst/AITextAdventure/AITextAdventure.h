#ifndef _AITextAdventure_AITextAdventure_h_
#define _AITextAdventure_AITextAdventure_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>
#include "AdventureEngine.h"

using namespace Upp;

NAMESPACE_UPP
void RegisterAdventureTools(MaestroToolRegistry& reg, AdventureEngine& engine);
END_UPP_NAMESPACE

class AITextAdventure : public TopWindow {
	MenuBar    menu;
	TabCtrl    tabs;
	
	// Game Tab
	AIChatCtrl game_chat;
	
	// Raw Chat Tab (Debug)
	AIChatCtrl raw_chat;
	
	AdventureEngine  engine;
	RecentConfig     config;
	bool             human_mode = false;
	String           last_processed_text;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void EditMenu(Bar& bar);
	
	void NewGame();
	void OnGameEvent(const MaestroEvent& e);
	void OnGameTurnDone();
	void SendCommand();

public:
	typedef AITextAdventure CLASSNAME;
	AITextAdventure();
};

#endif
