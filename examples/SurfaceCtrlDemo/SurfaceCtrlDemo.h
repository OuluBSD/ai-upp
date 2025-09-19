#ifndef _SurfaceCtrlDemo_SurfaceCtrlDemo_h_
#define _SurfaceCtrlDemo_SurfaceCtrlDemo_h_

#include <CtrlLib/CtrlLib.h>
#include <GuboCore/GuboCore.h>

using namespace Upp;

class SurfaceCtrl : public Ctrl {
    TopSurface top;
public:
    SurfaceCtrl() {
        top.SetFrameRect(RectC(0,0,100,100));
        top.CreateGeom2DComponent();
    }
    void Layout() override {
        top.SetFrameRect(GetSize());
        top.SetPendingLayout();
    }
    void Paint(Draw& w) override {
        // Placeholder: show a pane; real rendering requires DrawCommand replay or image blit.
        w.DrawRect(GetSize(), LtCyan());
        w.DrawText(8, 8, "SurfaceCtrl placeholder", StdFont(), Black());
    }
    bool Key(dword key, int count) override {
        CtrlEvent e;
        e.type = EVENT_KEYDOWN;
        e.value = key;
        e.n = count;
        return top.Dispatch(e);
    }
    void LeftDown(Point p, dword keyflags) override { SendMouse(EVENT_MOUSE_EVENT, MOUSE_LEFTDOWN, p, 0, keyflags); }
    void LeftUp(Point p, dword keyflags) override { SendMouse(EVENT_MOUSE_EVENT, MOUSE_LEFTUP, p, 0, keyflags); }
    void RightDown(Point p, dword keyflags) override { SendMouse(EVENT_MOUSE_EVENT, MOUSE_RIGHTDOWN, p, 0, keyflags); }
    void RightUp(Point p, dword keyflags) override { SendMouse(EVENT_MOUSE_EVENT, MOUSE_RIGHTUP, p, 0, keyflags); }
    void MouseMove(Point p, dword keyflags) override { SendMouse(EVENT_MOUSEMOVE, 0, p, 0, keyflags); }
    void MouseWheel(Point p, int zdelta, dword keyflags) override { SendMouse(EVENT_MOUSEWHEEL, 0, p, zdelta, keyflags); }
private:
    void SendMouse(int type, int code, Point p, int z, dword kf) {
        CtrlEvent e;
        e.type = type;
        e.n = code;
        e.pt = p;
        e.value = kf;
        e.n = (type == EVENT_MOUSEWHEEL ? z : code);
        top.Dispatch(e);
        Refresh();
    }
};

class MainWin : public TopWindow {
    SurfaceCtrl surface;
    Button btn;
    EditString edit;
public:
    MainWin() {
        Title("SurfaceCtrl + CtrlLib widgets");
        Add(surface.HSizePos(8, 8).VSizePos(8, 36));
        Add(btn.BottomPos(4, 24).LeftPos(8, 100));
        Add(edit.BottomPos(4, 24).HSizePos(116, 8));
        btn.SetLabel("Click me");
    }
};

#endif

