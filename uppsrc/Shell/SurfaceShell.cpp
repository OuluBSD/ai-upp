#include "EcsShell.h"


NAMESPACE_UPP


void VguiMain() {
	#if IS_UPP_CORE
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	
	ChClassicSkin();
	#endif
	
	Gu::GuiTesterApp app;
	app.OpenMain();
	Surface::EventLoop();
}

void DesktopMain() {
	
	
	Surface::SetDebugDraw(0);
	Gubo::SetDebugDraw(1);
	
	AtomVirtualGui3D gui;
	gui.Create(RectC(100, 100, 1024, 768), "SurfaceShell");

	RunVirtualGui3D(gui, callback(VguiMain));
	
}


END_UPP_NAMESPACE



CONSOLE_APP_MAIN {
	using namespace TS;
	Serial::Machine::WhenInitialize << callback(DefaultSerialInitializer);
	Serial::Machine::WhenPreFirstUpdate << callback(DefaultStartup);
	Serial::Machine::WhenUserProgram << callback(DesktopMain);
	
	SingleMachine mach;
	if (mach.Start()) {
		DefaultRunner(0, "Gui App", SURFACE_EON);
		
		mach.Stop();
	}
}
