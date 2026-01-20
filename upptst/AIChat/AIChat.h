#ifndef _AIChat_AIChat_h_
#define _AIChat_AIChat_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>

using namespace Upp;

class AIChat : public TopWindow {
	TabCtrl    tabs;
	AIChatCtrl chat;
	StaticText system_view;
	
public:
	typedef AIChat CLASSNAME;
	AIChat();
};

#endif