#include "EcsShell.h"


NAMESPACE_UPP


void VguiMain() {
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	
	ChClassicSkin();
	
	Upp::EventsTester app;
	//GuiTesterApp app;
	//CtrlTesterApp app;
	app.OpenMain();
	//app.SetRect(RectC(10,10,640,480));
	Ctrl::EventLoop();
}

void DesktopMain() {
	
	
	Surface::SetDebugDraw(true);
	Gubo::SetDebugDraw(true);
	
	UPP::AtomVirtualGui gui;
	gui.Create(RectC(100, 100, 1024, 768), "Libtopside Virtual Gui Test");

	RunVirtualGui(gui, callback(VguiMain));
	
}


END_UPP_NAMESPACE


GUI_APP_MAIN
{
	using namespace Upp;
	Serial::Machine::WhenInitialize << callback(DefaultSerialInitializer);
	Serial::Machine::WhenPreFirstUpdate << callback(DefaultStartup);
	Serial::Machine::WhenUserProgram << callback(DesktopMain);
	
	SingleMachine mach;
	if (mach.Start()) {
		DefaultRunner(0, "Gui App", GUI_EON);
		
		mach.Stop();
	}
}
