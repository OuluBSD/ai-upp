#ifdef flagUPP
#include <CtrlLib/CtrlLib.h>
using namespace Upp;
#elif defined flagMSC
#include <windows.h>
#endif

void Upp01(int);
void Qt01(int);
void Wx01(int);
void Juce01(int);
void Mfc01(int);
void Wtl01(int);
void WinRT01(int);
void Vst01(int);
void Gtkmm01(int);
void Wt01(int);
void Agar01(int);
void Gtk01(int);
void Cocoa01(int);

#ifdef flagUPP
GUI_APP_MAIN
#elif defined flagMSC
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int,const char**)
#endif
{
	int test = 0;
	
	#ifdef flagUPP
	if (CommandLine().GetCount())
		test = ScanInt(CommandLine()[0]);
	#endif
	
	#ifdef flagUPP
	Upp01(test);
	#endif
	
	#ifdef HAVE_QT
	Qt01(test);
	#endif
	
	#ifdef HAVE_WX
	Wx01(test);
	#endif
	
	#ifdef HAVE_JUCE
	Juce01(test);
	#endif
	
	#ifdef flagMFC
	Mfc01(test);
	#endif
	
	#ifdef flagWTL
	Wtl01(test);
	#endif
	
	#ifdef flagWINRT
	WinRT01(test);
	#endif

	#ifdef HAVE_VSTGUI
	Vst01(test);
	#endif

	#ifdef HAVE_GTKMM
	Gtkmm01(test);
	#endif

	#ifdef HAVE_WT
	Wt01(test);
	#endif

	#ifdef HAVE_AGAR
	Agar01(test);
	#endif

	#ifdef HAVE_GTK
	Gtk01(test);
	#endif

	#ifdef HAVE_COCOA
	Cocoa01(test);
	#endif
}
