#ifndef _Maestro_AIChatCtrl_h_
#define _Maestro_AIChatCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include "CliEngine.h"

NAMESPACE_UPP

class MaestroItem : public Ctrl {
public:
	String role;
	String text;
	bool   is_error = false;
	bool   is_tool = false;
	
	virtual void Paint(Draw& d) override;
	int GetHeight(int width) const;
	
	typedef MaestroItem CLASSNAME;
	MaestroItem() {}
};

class AIChatCtrl : public Ctrl {
	Array<MaestroItem> items;
	String             current_response;
	ScrollBar          vscroll;
	
public:
	DocEdit            input;
	Button             send;
	Option             send_continue;
	Ctrl               chat;
	String             backend;
	CliMaestroEngine   engine;
	
	virtual void OnSend();
	virtual void OnEvent(const MaestroEvent& e);
	virtual void OnDone();
	void Poll();
	
	void AddItem(const String& role, const String& text, bool is_error = false);
	void AddToolItem(const String& role, const String& text);
	void CopyAllChat();
	void CopyDebugData();
	void OnSelectSession();

	virtual void Layout() override;
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) override;

	typedef AIChatCtrl CLASSNAME;
	AIChatCtrl();
	
	Function<void()> WhenDone;
};

END_UPP_NAMESPACE

#endif