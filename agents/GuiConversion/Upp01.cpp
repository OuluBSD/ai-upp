#ifdef flagUPP
#include <CtrlLib/CtrlLib.h>
using namespace Upp;


void Upp01(int test_num) {
	
	// 1. Hello World GUI
	if (test_num == 0 || test_num < 0) {
		TopWindow tw;
		tw.Title("Hello World program");
		Label lbl;
		tw.SetRect(0,0,320,240);
		tw.Add(lbl.SizePos());
		lbl.SetLabel("Hello world!");
		lbl.SetAlign(ALIGN_CENTER);
		tw.Run();
	}
	
	// 2. Simple events (Button & Callbacks)
	{
		
	}
	
}
#endif
