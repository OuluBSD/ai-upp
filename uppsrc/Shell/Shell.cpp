#include "Shell.h"


// UWP solution is different
#ifdef flagUWP

using namespace concurrency;
using namespace std::placeholders;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Holographic;
using namespace winrt::Windows::UI::Core;

NAMESPACE_UPP


void DesktopMain() {
	
	
    CoreApplication::Run(AppViewSource());
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
		DefaultRunner(0, "Shell");
		
		mach.Stop();
	}
}


#else



#if 0
void BindEcsEventsBase(EcsEventsBase* b) {
	auto esys = b->val.FindOwner<EventSystem>();
	ASSERT(esys);
	esys->Attach(b);
}
#endif

using namespace Upp;

template <class Gfx>
void BindGfxBuffer(String id, BufferT<Gfx>* b) {
	TODO
}

void BindEcsToSerial() {
	//Serial::EcsEventsBase::WhenInitialize << callback(BindEcsEventsBase);
	
	#if 0
	
	#ifdef flagSDL2
	BufferT<SdlSwGfx>::WhenLinkInit << callback(BindGfxBuffer<SdlSwGfx>);
	#ifdef flagOGL
	BufferT<SdlOglGfx>::WhenLinkInit << callback(BindGfxBuffer<SdlOglGfx>);
	#endif
	#endif
	
	#endif
}


void VguiMain(bool _3d) {
	TODO
	#if 0
	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);
	ChClassicSkin();
	
	if (_3d) {
		Gu::GuiTesterApp app;
		app.OpenMain();
		Surface::EventLoop();
	}
	else {
		Upp::EventsTester app;
		//GuiTesterApp app;
		//CtrlTesterApp app;
		app.OpenMain();
		//app.SetRect(RectC(10,10,640,480));
		Ctrl::EventLoop();
	}
	#endif
}



void DesktopMain(Upp::Engine& mach, bool _3d) {
	TODO
	#if 0
	Surface::SetDebugDraw(0);
	Gubo::SetDebugDraw(1);
	
	if (_3d) {
		AtomVirtualGui3D gui;
		gui.Create(RectC(100, 100, 1024, 768), "SurfaceShell");
		RunVirtualGui3D(gui, callback1(VguiMain, true));
	}
	else {
		AtomVirtualGui gui;
		gui.Create(RectC(100, 100, 1024, 768), "Libtopside Virtual Gui Test");
		RunVirtualGui(gui, callback1(VguiMain, false));
	}
	#endif
}


#ifdef flagGUI

NAMESPACE_UPP
INITBLOCK {
	Engine::WhenInitialize << callback(DefaultSerialInitializer);
	Engine::WhenPreFirstUpdate << callback(DefaultStartup);
	//Machine::WhenPreFirstUpdate << callback(BindEcsToSerial);
}
END_UPP_NAMESPACE

GUI_APP_MAIN {
	Engine& mach = MetaEnv().root.Add<Engine>("mach");
	mach.Run(true, "Shell");
}

#else


CONSOLE_APP_MAIN {
	using namespace Upp;
	auto& root = MetaEnv().root;
	
	Engine& eng = root.Add<Engine>("eng");
	eng.WhenBoot << callback(DefaultSerialInitializer);
	eng.WhenInitialize << callback(::Upp::MachineEcsInit);
	eng.WhenPreFirstUpdate << callback(DefaultStartup);
	eng.Start("Shell");

	bool gubo = false;
	if (gubo) {
		eng.WhenUserProgram << callback1(DesktopMain, true);
	}
	
	eng.MainLoop();
	
	eng.Stop();
	eng.Clear();
	
	root.StopDeep();
	root.ClearDependenciesDeep();
	root.UninitializeDeep();
	root.ClearExtDeep();
	root.sub.Clear();
}

#endif


#endif
