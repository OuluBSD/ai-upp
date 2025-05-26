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
	auto esys = b->val.FindOwner<Ecs::EventSystem>();
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



void DesktopMain(Upp::Machine& mach, bool _3d) {
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
	Machine::WhenInitialize << callback(DefaultSerialInitializer);
	Machine::WhenPreFirstUpdate << callback(DefaultStartup);
	//Machine::WhenPreFirstUpdate << callback(BindEcsToSerial);
}
END_UPP_NAMESPACE

GUI_APP_MAIN {
	Machine& mach = MetaEnv().root.Add<Machine>("mach");
	mach.Run(true, "Shell");
}

#else


CONSOLE_APP_MAIN {
	using namespace Upp;
	Machine& mach = MetaEnv().root.Add<Machine>("mach");
	mach.WhenBoot << callback(DefaultSerialInitializer);
	mach.WhenInitialize << callback(::Upp::MachineEcsInit);
	mach.WhenPreFirstUpdate << callback(DefaultStartup);
	
	Ecs::Engine& eng = MetaEnv().root.Add<Ecs::Engine>("eng");
	eng.Start();

	AgentInteractionSystem::Setup();
	
	bool gubo = false;
	if (gubo) {
		mach.WhenUserProgram << callback1(DesktopMain, true);
	}
	
	mach.Run(true, "Shell");
	
	AgentInteractionSystem::Uninstall();
	
	eng.Stop();
	mach.Stop();
	
	eng.GetRootPool().Clear();
	mach.GetRootSpace().Clear();
	mach.GetRootLoop().Clear();
	mach.Clear();
	MetaEnv().root.ClearExtDeep();
	MetaEnv().root.sub.Clear();
}

#endif


#endif
