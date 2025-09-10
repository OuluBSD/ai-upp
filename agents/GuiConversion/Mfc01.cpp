#ifdef flagMFC
#include <afxwin.h>

class CHelloFrame : public CFrameWnd {
public:
    CHelloFrame() {
        Create(NULL, _T("Hello World program"),
               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
               CRect(100, 100, 420, 340));
        CRect rc; GetClientRect(&rc);
        m_label.Create(_T("Hello world!"), WS_CHILD | WS_VISIBLE | SS_CENTER, rc, this, 1);
    }
protected:
    afx_msg void OnDestroy() { CFrameWnd::OnDestroy(); PostQuitMessage(0); }
    DECLARE_MESSAGE_MAP()
private:
    CStatic m_label;
};

BEGIN_MESSAGE_MAP(CHelloFrame, CFrameWnd)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

void Mfc01(int test_num) {
    if (test_num == 0 || test_num < 0) {
        HINSTANCE hInst = (HINSTANCE)::GetModuleHandle(NULL);
        if(!AfxWinInit(hInst, NULL, ::GetCommandLine(), 0))
            return;

        CHelloFrame* pFrame = new CHelloFrame();
        pFrame->ShowWindow(SW_SHOW);
        pFrame->UpdateWindow();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        delete pFrame;
    }
}
#else
void Mfc01(int) {}
#endif
