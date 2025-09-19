#ifndef _GuboGLCtrlDemo_GuboGLCtrlDemo_h_
#define _GuboGLCtrlDemo_GuboGLCtrlDemo_h_

#include <CtrlLib/CtrlLib.h>
#include <GuboCtrl/GuboCtrl.h>

using namespace Upp;

class MainWin : public TopWindow {
    GuboGLCtrl glctrl;
    Button btn;
public:
    MainWin() {
        Title("GuboGLCtrl + CtrlLib");
        Add(glctrl.HSizePos(8, 8).VSizePos(8, 36));
        Add(btn.BottomPos(4, 24).LeftPos(8, 100));
        btn.SetLabel("Button");
    }
};

#endif

