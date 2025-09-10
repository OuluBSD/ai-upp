#ifdef flagWTL
#include <atlbase.h>
#include <atlapp.h>
CAppModule _Module;
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>

class CHelloFrame : public CFrameWindowImpl<CHelloFrame> {
public:
    DECLARE_FRAME_WND_CLASS_EX(L"WTL_HelloWorld_WindowClass", 0, CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

    BEGIN_MSG_MAP(CHelloFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
        RECT rc; GetClientRect(&rc);
        m_label.Create(m_hWnd, rc, L"Hello world!", WS_CHILD | WS_VISIBLE | SS_CENTER, 0);
        SetWindowText(L"Hello World program");
        CenterWindow();
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
        PostQuitMessage(0);
        return 0;
    }

private:
    CStatic m_label;
};

void Wtl01(int test_num) {
    if (test_num == 0 || test_num < 0) {
        _Module.Init(NULL, GetModuleHandle(NULL));
        CMessageLoop theLoop;
        _Module.AddMessageLoop(&theLoop);

        CHelloFrame wnd;
        if (wnd.CreateEx() == NULL) {
            _Module.RemoveMessageLoop();
            _Module.Term();
            return;
        }
        wnd.ShowWindow(SW_SHOW);
        theLoop.Run();
        _Module.RemoveMessageLoop();
        _Module.Term();
    }
}
#else
void Wtl01(int) {}
#endif

