#include "Shell.h"

#ifdef flagUWP
#include <ppl.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Graphics.Holographic.h>
#include <winrt/Windows.UI.Core.h>

using namespace concurrency;
using namespace std::placeholders;

namespace winrt_app = winrt::Windows::ApplicationModel;
namespace winrt_core = winrt::Windows::ApplicationModel::Core;
namespace winrt_ui_core = winrt::Windows::UI::Core;
namespace winrt_activation = winrt::Windows::ApplicationModel::Activation;
namespace winrt_foundation = winrt::Windows::Foundation;
namespace winrt_holo = winrt::Windows::Graphics::Holographic;

NAMESPACE_UPP


void UwpDesktopMain() {
    winrt_core::CoreApplication::Run(AppViewSource());
}


END_UPP_NAMESPACE

#endif


using namespace Upp;

template <class Gfx>
void BindGfxBuffer(String id, BufferT<Gfx>* b) {
}

void BindEcsToSerial() {
}


void VguiMain(bool _3d) {
}



void DesktopMain(Upp::Engine& mach, bool _3d) {
}


#ifdef flagMAIN
#ifdef flagGUI
GUI_APP_MAIN {
	ShellMain(false);
}
#else
CONSOLE_APP_MAIN {
	ShellMain(false);
}
#endif
#endif

Upp::Engine& ShellMainEngine()
{
	using namespace Upp;
	Engine& eng = MetaEnv().root.GetAdd<Engine>("eng");
	return eng;
}

void ShellMain(bool skip_eon)
{
	using namespace Upp;
	
	extern int ForceLinkDraw_Dummy;
	ForceLinkDraw_Dummy++;
	ForceLinkDraw();

	Engine& eng = ShellMainEngine();
	eng.WhenInitialize << callback(::Upp::MachineEcsInit);
	eng.WhenInitialize << callback(::Upp::EngineEcsInit);
	if (skip_eon)
		eng.WhenBoot << callback(DefaultSerialInitializerInternalEon);
	else
		eng.WhenBoot << callback(DefaultSerialInitializer);
	bool gubo = false;
	if (gubo) {
		eng.WhenUserProgram << callback1(DesktopMain, true);
	}
	
	Engine::Setup("Shell", &eng);
	
	eng.MainLoop();
	
	Engine::Uninstall(true, &eng);
}