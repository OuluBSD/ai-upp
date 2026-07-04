#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Easing/Easing.h>
using namespace Upp;

GUI_APP_MAIN
{
	Cout() << "EaseOutCubic: " << EaseOutCubic(0.0) << " " << EaseOutCubic(0.25) << " "
	       << EaseOutCubic(0.5) << " " << EaseOutCubic(0.75) << " " << EaseOutCubic(1.0) << "\n";

	Tween tween;
	tween.Start(200,
		EaseOutCubic,
		[](double v) { Cout() << "step: " << v << "\n"; },
		[] { Cout() << "done\n"; });

	while(tween.IsRunning())
		Ctrl::ProcessEvents();

	Cout() << "exiting cleanly\n";
}
