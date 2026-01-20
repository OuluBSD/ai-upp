#ifndef _Maestro_AIChatCtrl_h_
#define _Maestro_AIChatCtrl_h_

#include <CtrlLib/CtrlLib.h>
#include "Maestro.h"

namespace Upp {

class MaestroItem : public Ctrl {
public:
	String role;
	String text;
	bool   is_error = false;
	bool   is_tool = false;
	
	virtual void Paint(Draw& d) override;
	int GetHeight(int width) const;
	
	MaestroItem() {}
};

class AIChatCtrl : public Ctrl {
public:
	CliMaestroEngine engine;
	
	Array<MaestroItem> items;
	ScrollBar          vscroll;
	
	DocEdit            input;
	Button             send;
	DropList           engine_select;
	
	String             current_response;
	
	Event<>            WhenDone;
	
	void OnSend();
	void OnEvent(const MaestroEvent& e);
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
};

}

#endif
