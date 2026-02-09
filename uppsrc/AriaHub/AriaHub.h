#ifndef _AriaHub_AriaHub_h_
#define _AriaHub_AriaHub_h_

#include <Aria/Aria.h>
#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Automation/Automation.h>
#include <ByteVM/ByteVM.h>
#include "ThreadsCtrl.h"
#include "WhatsAppCtrl.h"

NAMESPACE_UPP

class AriaMainWindow : public TopWindow {
public:
	typedef AriaMainWindow CLASSNAME;
	AriaMainWindow();

	void MainMenu(Bar& bar);
	void ServiceMenu(Bar& bar);

	void RefreshActiveSubTab();
	void RefreshActiveService();
	void RefreshAllServices();

	void RefreshAll();
	void OpenSettings();
	void StopScrapers();

	virtual bool Key(dword key, int count) override;

private:
	MenuBar   menu;
	TabCtrl   tabs;
	ToolBar   toolbar;
	StatusBar statusbar;
	
	Aria      aria; // Backend integration
	
	ThreadsCtrl  threads;
	WhatsAppCtrl whatsapp;
};

void AriaAlert(const String& msg);
bool IsAutomation();

END_UPP_NAMESPACE

#endif
