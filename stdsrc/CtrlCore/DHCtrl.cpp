// STL-backed DHCtrl (Direct Handle Control) implementation for stdsrc/CtrlCore

#include "DHCtrl.h"

#ifdef PLATFORM_WIN32

#include <windows.h>

namespace Upp {

// Static member definitions
Vector<DHCtrl *> DHCtrl::all_active;

// Constructor
DHCtrl::DHCtrl() : hwnd(NULL), current_visible(false) {
    BackPaint(EXCLUDEPAINT);
}

// Destructor
DHCtrl::~DHCtrl() {
    CloseHWND();
    RemoveActive();
}

// State management
void DHCtrl::State(int reason) {
    switch(reason) {
    case OPEN:
        current_pos = Null;
        current_visible = Null;
        OpenHWND();
        [[fallthrough]];
    default:
        SyncHWND();
        break;
    case CLOSE:
        CloseHWND();
        break;
    }
}

// Synchronize HWND with control state
void DHCtrl::SyncHWND() {
    if(hwnd) {
        Rect r = GetScreenRect() - GetScreenClient(hwnd).TopLeft();
        if(r != current_pos || IsVisible() != current_visible) {
            SetWindowPos(hwnd, NULL, r.left, r.top, r.Width(), r.Height(), 
                        SWP_NOACTIVATE|SWP_NOZORDER);
            ShowWindow(hwnd, IsVisible() ? SW_SHOW : SW_HIDE);
            Refresh();
            current_pos = r;
            current_visible = IsVisible();
        }
    }
}

// Open HWND
void DHCtrl::OpenHWND() {
    CloseHWND();
    HWND phwnd = GetTopCtrl()->GetHWND();
    if(phwnd)
        CreateWindowEx(0, "UPP-CLASS-DH", "",
                       WS_CHILD|WS_DISABLED|WS_VISIBLE,
                       0, 0, 20, 20,
                       phwnd, NULL, hInstance, this);
}

// Close HWND
void DHCtrl::CloseHWND() {
    if(hwnd) {
        DestroyWindow(hwnd);
        hwnd = NULL;
        RemoveActive();
    }
}

// Window procedure
LRESULT DHCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    if(message == WM_ERASEBKGND)
        return 1L;
    return DefWindowProc(hwnd, message, wParam, lParam);
}

// Preprocess message
bool DHCtrl::PreprocessMessage(MSG& msg) {
    return false;
}

// Static preprocess message for all DHCtrl instances
bool DHCtrl::PreprocessMessageAll(MSG& msg) {
    for(auto q : all_active)
        if(q->PreprocessMessage(msg))
            return true;
    return false;
}

// Non-client create
void DHCtrl::NcCreate(HWND _hwnd) {
    hwnd = _hwnd;
    all_active.Add(this);
}

// Non-client destroy
void DHCtrl::NcDestroy() {
    hwnd = NULL;
    RemoveActive();
}

// Remove from active list
void DHCtrl::RemoveActive() {
    for(;;) {
        int q = FindIndex(all_active, this);
        if(q < 0)
            return;
        all_active.Remove(q);
    }
}

// Update DHCtrl children
void Ctrl::UpdateDHCtrls() {
    // We call this in WM_PAINT to force updating in single WM_PAINT,
    // this makes things smoother e.g. with OpenGL in splitter
    for(Ctrl *q = GetFirstChild(); q; q = q->GetNext()) {
        DHCtrl *dh = dynamic_cast<DHCtrl *>(q);
        if(dh)
            UpdateWindow(dh->GetHWND());
        q->UpdateDHCtrls();
    }
}

// Platform-specific implementations
void DHCtrl::Sync() {
    if(hwnd) {
        Rect r = GetRect().Size();
        SetWindowPos(hwnd, NULL, r.left, r.top, r.Width(), r.Height(), 
                    SWP_NOACTIVATE|SWP_NOZORDER);
    }
    Ctrl::Sync();
}

void DHCtrl::Paint(Draw& w) {
    // DHCtrl typically doesn't paint itself - it's handled by the native control
}

// Platform-specific initialization
void DHCtrl::GuiPlatformConstruct() {
    // Platform-specific construction
}

void DHCtrl::GuiPlatformDestruct() {
    // Platform-specific destruction
}

// Platform-specific methods
void DHCtrl::OpenPlatform() {
    // Platform-specific open
}

void DHCtrl::ClosePlatform() {
    // Platform-specific close
}

// Platform-specific refresh
void DHCtrl::RefreshPlatform() {
    // Platform-specific refresh
}

// Platform-specific visibility
void DHCtrl::ShowPlatform(bool show) {
    if(hwnd) {
        ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
    }
}

// Platform-specific positioning
void DHCtrl::SetPosPlatform(int x, int y) {
    if(hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        SetWindowPos(hwnd, NULL, x, y, 
                    rect.right - rect.left, rect.bottom - rect.top,
                    SWP_NOACTIVATE|SWP_NOZORDER);
    }
}

// Platform-specific sizing
void DHCtrl::SetSizePlatform(int cx, int cy) {
    if(hwnd) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        SetWindowPos(hwnd, NULL, rect.left, rect.top, cx, cy,
                    SWP_NOACTIVATE|SWP_NOZORDER);
    }
}

// Platform-specific rectangle setting
void DHCtrl::SetRectPlatform(const Rect& r) {
    if(hwnd) {
        SetWindowPos(hwnd, NULL, r.left, r.top, r.Width(), r.Height(),
                    SWP_NOACTIVATE|SWP_NOZORDER);
    }
}

// Platform-specific focus
void DHCtrl::SetFocusPlatform() {
    if(hwnd) {
        SetFocus(hwnd);
    }
}

// Platform-specific capture
bool DHCtrl::SetCapturePlatform() {
    if(hwnd) {
        ::SetCapture(hwnd);
        return true;
    }
    return false;
}

bool DHCtrl::ReleaseCapturePlatform() {
    if(hwnd && GetCapture() == hwnd) {
        ::ReleaseCapture();
        return true;
    }
    return false;
}

bool DHCtrl::HasCapturePlatform() const {
    return hwnd && GetCapture() == hwnd;
}

// Platform-specific cursor
void DHCtrl::SetCursorPlatform(const Image& cursor) {
    // In a real implementation, this would set the cursor for the HWND
}

// Platform-specific painting
void DHCtrl::BeginPaintPlatform() {
    // In a real implementation, this would begin platform-specific painting
}

void DHCtrl::EndPaintPlatform() {
    // In a real implementation, this would end platform-specific painting
}

// Platform-specific event handling
void DHCtrl::HandleEventPlatform(UINT message, WPARAM wParam, LPARAM lParam) {
    // In a real implementation, this would handle platform-specific events
}

// Platform-specific timer
void DHCtrl::SetTimerPlatform(int id, int interval) {
    if(hwnd) {
        ::SetTimer(hwnd, id, interval, NULL);
    }
}

void DHCtrl::KillTimerPlatform(int id) {
    if(hwnd) {
        ::KillTimer(hwnd, id);
    }
}

// Platform-specific scrolling
void DHCtrl::ScrollPlatform(int dx, int dy) {
    if(hwnd) {
        ::ScrollWindow(hwnd, dx, dy, NULL, NULL);
    }
}

// Platform-specific clipboard
bool DHCtrl::SetClipboardPlatform(const String& text) {
    // In a real implementation, this would set clipboard through HWND
    return false;
}

String DHCtrl::GetClipboardPlatform() {
    // In a real implementation, this would get clipboard through HWND
    return String();
}

// Platform-specific drag and drop
int DHCtrl::DoDragAndDropPlatform(const VectorMap<String, ClipData>& data, 
                                  const Image& sample, dword actions) {
    // In a real implementation, this would perform platform-specific drag and drop
    return 0;
}

// Platform-specific keyboard
void DHCtrl::SetKeyboardFocusPlatform() {
    if(hwnd) {
        SetFocus(hwnd);
    }
}

bool DHCtrl::HasKeyboardFocusPlatform() const {
    return hwnd && GetFocus() == hwnd;
}

// Platform-specific mouse
Point DHCtrl::GetMousePosPlatform() const {
    POINT pt;
    if(hwnd && GetCursorPos(&pt)) {
        ScreenToClient(hwnd, &pt);
        return Point(pt.x, pt.y);
    }
    return Point(0, 0);
}

// Platform-specific window management
void DHCtrl::BringToFrontPlatform() {
    if(hwnd) {
        ::BringWindowToTop(hwnd);
    }
}

void DHCtrl::SendToBackPlatform() {
    if(hwnd) {
        ::SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, 
                      SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    }
}

// Platform-specific z-order
void DHCtrl::SetZOrderPlatform(HWND insert_after, bool top_most) {
    if(hwnd) {
        UINT flags = SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE;
        if(top_most) {
            flags |= SWP_NOACTIVATE;
        }
        ::SetWindowPos(hwnd, insert_after, 0, 0, 0, 0, flags);
    }
}

// Platform-specific window styles
void DHCtrl::SetWindowStylePlatform(dword style, dword ex_style) {
    if(hwnd) {
        ::SetWindowLong(hwnd, GWL_STYLE, style);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
        ::SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
                      SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
    }
}

// Platform-specific window text
void DHCtrl::SetWindowTextPlatform(const String& text) {
    if(hwnd) {
        ::SetWindowText(hwnd, text);
    }
}

String DHCtrl::GetWindowTextPlatform() const {
    if(hwnd) {
        int len = ::GetWindowTextLength(hwnd);
        if(len > 0) {
            Buffer<wchar> buffer(len + 1);
            ::GetWindowText(hwnd, buffer, len + 1);
            return FromWString(buffer);
        }
    }
    return String();
}

// Platform-specific window placement
void DHCtrl::SaveWindowPlacementPlatform(Stream& s) {
    if(hwnd) {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if(::GetWindowPlacement(hwnd, &wp)) {
            // Serialize the window placement
            s % wp.flags % wp.showCmd % wp.ptMinPosition.x % wp.ptMinPosition.y
              % wp.ptMaxPosition.x % wp.ptMaxPosition.y % wp.rcNormalPosition.left
              % wp.rcNormalPosition.top % wp.rcNormalPosition.right % wp.rcNormalPosition.bottom;
        }
    }
}

void DHCtrl::RestoreWindowPlacementPlatform(Stream& s) {
    if(hwnd) {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        // Deserialize the window placement
        s % wp.flags % wp.showCmd % wp.ptMinPosition.x % wp.ptMinPosition.y
          % wp.ptMaxPosition.x % wp.ptMaxPosition.y % wp.rcNormalPosition.left
          % wp.rcNormalPosition.top % wp.rcNormalPosition.right % wp.rcNormalPosition.bottom;
        ::SetWindowPlacement(hwnd, &wp);
    }
}

}

#endif // PLATFORM_WIN32