#ifndef _VirtualGui_VirtualGui_h_
#define _VirtualGui_VirtualGui_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

#ifdef PLATFORM_POSIX
#include <CtrlCore/stdids.h>
#endif

namespace Upp {

class SystemDraw : public DrawProxy {
public:
	bool    CanSetSurface()                         { return false; }
	static void Flush()                             {}
};

enum KM {
	KM_NONE  = 0x00,

	KM_LSHIFT= 0x01,
	KM_RSHIFT= 0x02,
	KM_LCTRL = 0x04,
	KM_RCTRL = 0x08,
	KM_LALT  = 0x10,
	KM_RALT  = 0x20,

	KM_CAPS  = 0x40,
	KM_NUM   = 0x80,
	
	KM_CTRL = KM_LCTRL | KM_RCTRL,
	KM_SHIFT = KM_LSHIFT | KM_RSHIFT,
	KM_ALT = KM_LALT | KM_RALT,
};

enum GUI_OPTIONS {
	GUI_SETMOUSECURSOR = 0x01,
	GUI_SETCARET       = 0x02,
};

class TopWindow;

struct VirtualGui {
	virtual dword       GetOptions();
	// Whether VirtualGui's own TopWindowFrame (Top.h/TopFrame.cpp) should draw its
	// generic title bar + border chrome around every hosted TopWindow. True by
	// default (Turtle's browser canvas has no window manager of its own, so it
	// genuinely needs this chrome to fake a window). NetDpy overrides this to false
	// since DisplayServer already draws a real, interactive outer frame around the
	// hosted window (see NetworkDisplay/0010) -- drawing both would double-frame it.
	virtual bool        WantsOwnWindowFrame();
	virtual Size        GetSize() = 0;
	virtual dword       GetMouseButtons() = 0;
	virtual dword       GetModKeys() = 0;
	virtual bool        IsMouseIn() = 0;
	virtual bool        ProcessEvent(bool *quit) = 0;
	virtual void        WaitEvent(int ms) = 0;
	virtual void        WakeUpGuiThread() = 0;
	virtual void        SetMouseCursor(const Image& image);
	virtual void        SetCaret(const Rect& caret);
	virtual void        Quit() = 0;
	virtual bool        IsWaitingEvent() = 0;
	virtual SystemDraw& BeginDraw() = 0;
	virtual void        CommitDraw() = 0;

	// NetworkDisplay/0014: whether Wnd.cpp's Ctrl::PopUp()/Ctrl::DestroyWnd()/
	// Ctrl::PaintScene() should route each genuine top-level TopWindow (the main
	// window, plus any popup/dialog that is itself a TopWindow -- NOT transient
	// plain-Ctrl popups such as dropdowns/tooltips/menus, which stay glued to
	// whichever real TopWindow owns them via the existing Ctrl::GetTopWindow())
	// to its own isolated paint batch/connection via WindowOpened()/WindowClosed()/
	// SelectWindow(), instead of compositing everything into one shared
	// StaticRect-backed virtual desktop canvas (Ctrl::PaintScene()'s original,
	// single-canvas loop). False by default: Turtle's entire virtual desktop
	// genuinely *is* one browser <canvas> element, so the existing shared-desktop
	// compositing is correct there and must stay exactly as-is. NetDpy overrides
	// this to true -- every TopWindow becomes its own DisplayServer connection and
	// window (see NetDpy/NetDpy.h/.cpp).
	virtual bool        WantsPerWindowRouting();

	// Called once, only when WantsPerWindowRouting() is true, right after a genuine
	// TopWindow (never a TopWindowFrame, never a transient plain-Ctrl popup) becomes
	// open -- from Ctrl::PopUp() (Wnd.cpp), guarded by dynamic_cast<TopWindow*>(this).
	// w->GetRect() already holds the window's final, resolved rect at this point.
	// Default no-op.
	virtual void        WindowOpened(TopWindow *w);

	// Mirror of WindowOpened(), called once from Ctrl::DestroyWnd() (Wnd.cpp) right
	// before the matching topctrl entry is removed, guarded the same way. Default
	// no-op.
	virtual void        WindowClosed(TopWindow *w);

	// Called once per genuine-TopWindow "cluster" (that window plus every transient
	// popup anchored to it via Ctrl::GetTopWindow()), immediately before painting
	// that cluster, only when WantsPerWindowRouting() is true (from the new
	// Ctrl::PaintPerWindowScene(), Wnd.cpp) -- lets the backend point subsequent
	// PutRect()/PutImage() calls (and the CommitDraw() that follows) at that
	// window's own per-connection state. Default no-op (irrelevant when
	// WantsPerWindowRouting() is false).
	virtual void        SelectWindow(TopWindow *w);

	// NetworkDisplay/0015: called from TopWindow::SyncTitle() (VirtualGui/Top.cpp),
	// only when WantsPerWindowRouting() is true (NetDpy only -- Turtle never
	// overrides WantsPerWindowRouting(), so this is unreachable for it, same as
	// WindowOpened()/WindowClosed() above), right after `w`'s title actually
	// changed (TopWindow::Title() already guards on title != previous title, so
	// this never fires spuriously). There is no other existing signal for this --
	// SyncTitle()/SyncCaption() just assign a plain member on VirtualGui's own
	// internal TopWindowFrame and repaint it locally, with nothing observable from
	// outside. Default no-op.
	virtual void        WindowTitleChanged(TopWindow *w);
};

void RunVirtualGui(VirtualGui& gui, Event<> app_main);

struct BackDraw__ : public SystemDraw {
	BackDraw__() : SystemDraw() {}
};

class BackDraw : public BackDraw__ { // Dummy only, as we are running in GlobalBackBuffer mode
	Size        size;
	Draw       *painting;
	Point       painting_offset;
	ImageBuffer ib;
	
public:
	virtual bool  IsPaintingOp(const Rect& r) const;

public:
	void  Put(SystemDraw& w, int x, int y)             {}
	void  Put(SystemDraw& w, Point p)                  { Put(w, p.x, p.y); }

	void Create(SystemDraw& w, int cx, int cy)         {}
	void Create(SystemDraw& w, Size sz)                { Create(w, sz.cx, sz.cy); }
	void Destroy()                                     {}

	void SetPaintingDraw(Draw& w, Point off)           { painting = &w; painting_offset = off; }

	Point GetOffset() const                            { return Point(0, 0); }

	BackDraw();
	~BackDraw();
};

class ImageDraw : public SImageDraw { // using software renderer
public:
	ImageDraw(Size sz) : SImageDraw(sz) {}
	ImageDraw(int cx, int cy) : SImageDraw(cx, cy) {}
};

void DrawDragRect(SystemDraw& w, const Rect& rect1, const Rect& rect2, const Rect& clip, int n,
                  Color color, uint64 pattern);

class TopWindowFrame;

#define GUIPLATFORM_CTRL_TOP_DECLS   Ctrl *owner_window;

#define GUIPLATFORM_CTRL_DECLS_INCLUDE <VirtualGui/Ctrl.h>

#define GUIPLATFORM_PASTECLIP_DECLS \
	bool dnd; \
	friend struct DnDLoop; \

#define GUIPLATFORM_TOPWINDOW_DECLS_INCLUDE <VirtualGui/Top.h>

class PrinterJob { // Dummy only...
	NilDraw             nil_;
	Vector<int>         pages;

public:
	Draw&               GetDraw()                       { return nil_; }
	operator            Draw&()                         { return GetDraw(); }
	const Vector<int>&  GetPages() const                { return pages; }
	int                 operator[](int i) const         { return 0; }
	int                 GetPageCount() const            { return 0; }

	bool                Execute()                       { return false; }

	PrinterJob& Landscape(bool b = true)                { return *this; }
	PrinterJob& MinMaxPage(int minpage, int maxpage)    { return *this; }
	PrinterJob& PageCount(int n)                        { return *this; }
	PrinterJob& CurrentPage(int currentpage)            { return *this; }
	PrinterJob& Name(const char *_name)                 { return *this; }

	PrinterJob(const char *name = NULL)                 {}
	~PrinterJob()                                       {}
};

}

#define GUIPLATFORM_INCLUDE_AFTER <VirtualGui/After.h>

#endif
