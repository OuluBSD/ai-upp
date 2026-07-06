#ifndef _NetDpy_NetDpy_h_
#define _NetDpy_NetDpy_h_

// NetDpy backend (NetworkDisplay/0007).
//
// This header is selected as CtrlCore.h's GUIPLATFORM_INCLUDE when flagNETDPY is
// defined. Unlike NetworkDisplay/0003's now-superseded meaning (a plain alias for
// flagTURTLE, i.e. "embed a TurtleServer"), flagNETDPY now means: this process is a
// network *client* of a separate, already-running DisplayServer process
// (uppsrc/DisplayServer, NetworkDisplay/0005/0006) -- its Ctrl/Draw output is encoded
// as DisplayServer's wire-protocol draw commands and shipped over a TCP socket instead
// of going to a local X11/GTK/Win32 window, and input events DisplayServer forwards
// back over that same socket are dispatched into Ctrl's normal input pipeline.
//
// Structurally this follows the exact same seam Turtle.h uses (a VirtualGui subclass,
// selected via GUIPLATFORM_INCLUDE, reusing VirtualGui's own Ctrl.h/Top.h/Keys.h decls
// since NetDpy needs nothing platform-special there -- unlike Turtle, whose key codes
// arrive from a web browser and need translating, NetDpy's key codes are already raw
// Upp K_ codes forwarded verbatim by DisplayServer, see Net/Protocol.h's SMSG_INPUT_KEY
// doc comment).
//
// NetworkDisplay/0014: NetDpy no longer funnels the whole app (main window + any
// popup/dialog) through VirtualGui's single shared virtual-desktop canvas. Instead it
// overrides VirtualGui::WantsPerWindowRouting() (true) and gives every genuine
// TopWindow (dynamic_cast<TopWindow*> succeeds -- the app's main window, and any
// dialog derived from TopWindow such as PromptOK's PromptDlgWnd__) its own
// DisplayServer connection/window via a per-TopWindow WindowConn, keyed in `conns`.
// Transient plain-Ctrl popups (dropdowns/tooltips/menus/IME) are NOT given their own
// connection -- they keep compositing into whichever real TopWindow's connection
// currently owns them (Ctrl::GetTopWindow()), exactly as before, just scoped to that
// one window's canvas instead of the whole process's. See the plan doc
// (CompositeEasingNetwork/NetworkDisplay/0014) for the full design rationale.

#define GUIPLATFORM_CTRL_TOP_DECLS   Ctrl *owner_window;
#define GUIPLATFORM_CTRL_DECLS_INCLUDE <VirtualGui/Ctrl.h>
#define GUIPLATFORM_PASTECLIP_DECLS \
	bool dnd; \
	friend struct DnDLoop; \

#define GUIPLATFORM_TOPWINDOW_DECLS_INCLUDE <VirtualGui/Top.h>

#include <VirtualGui/VirtualGui.h>
#include <DisplayServer/Net/Protocol.h>
#include <DisplayServer/Net/Transport.h>

#ifdef PLATFORM_POSIX
#include <CtrlCore/stdids.h>
#endif

namespace Upp {

// NetDpyServer is deliberately single-instance/static-state, exactly like
// TurtleServer -- a process using this backend represents exactly one virtual
// "desktop" (one shared internal Ctrl coordinate space, still used for input
// dispatch, see NetworkDisplay/0014's WindowConn/Pump() below), so there is never a
// second instance to keep separate from the free-standing Draw/event-queue state.
class NetDpyServer : public VirtualGui {
public:
	NetDpyServer() {}

	// Connects to a DisplayServer listening at host:port and sends CMSG_HELLO with
	// the given title/canvas size -- this becomes the app's *main* window's
	// connection once the app's first TopWindow opens (WindowOpened() adopts it;
	// see NetworkDisplay/0014). Must be called (successfully) before
	// RunNetDpyGui(). host/port are also remembered so any further TopWindow
	// (popup/dialog) opened later gets its own additional connection to the same
	// endpoint.
	bool Connect(const String& host, int port, const String& title, Size canvas_size);

	bool IsConnected() const;

private:
	virtual dword       GetOptions()                       { return 0; }
	// DisplayServer already draws a real, interactive outer window frame around
	// the hosted window (NetworkDisplay/0004/0005); suppress VirtualGui's own
	// TopWindowFrame chrome so the app's content isn't double-framed
	// (NetworkDisplay/0010).
	virtual bool        WantsOwnWindowFrame()              { return false; }
	virtual Size        GetSize()                          { return canvas_size; }
	virtual dword       GetMouseButtons()                  { return mousebuttons; }
	virtual dword       GetModKeys()                       { return 0; } // see Known limitations in the plan doc
	virtual bool        IsMouseIn()                        { return mousein; }
	virtual bool        ProcessEvent(bool *quit);
	virtual void        WaitEvent(int ms);
	virtual bool        IsWaitingEvent();
	virtual void        SetMouseCursor(const Image& image) {}
	virtual void        SetCaret(const Rect& caret)        {}
	virtual SystemDraw& BeginDraw();
	virtual void        CommitDraw();
	virtual void        WakeUpGuiThread()                  {}
	virtual void        Quit();

	// NetworkDisplay/0014: every genuine TopWindow gets its own DisplayServer
	// connection/window -- see VirtualGui.h's doc comments for the general hook
	// contract, and the .cpp for what each override actually does here.
	virtual bool        WantsPerWindowRouting()            { return true; }
	virtual void        WindowOpened(TopWindow *w);
	virtual void        WindowClosed(TopWindow *w);
	virtual void        SelectWindow(TopWindow *w);

	// NetworkDisplay/0015: fired by VirtualGui/Top.cpp's TopWindow::SyncTitle()
	// (only when VirtualGuiPtr->WantsPerWindowRouting() is true, i.e. only for
	// NetDpy -- Turtle's own per-window chrome is unaffected) whenever a hosted
	// TopWindow's title actually changes after connect (Ctrl::Title() called
	// again, e.g. examples/UWord's Save As/new-document titling). Sends CMSG_TITLE
	// on that window's own connection so DisplayServer's frame title stays live
	// instead of frozen at whatever CMSG_HELLO carried at connect time.
	virtual void        WindowTitleChanged(TopWindow *w);

private:
	struct QueuedEvent : Moveable<QueuedEvent> {
		enum Kind { MOUSE, KEY, CLOSE } kind;
		byte       mkind = 0;
		byte       kkind = 0;
		Point      pos   = Point(0, 0);
		dword      keyflags = 0;
		dword      keycode = 0;
		// Which window's connection this event arrived on (NetworkDisplay/0014) --
		// used to translate MOUSE positions back into NetDpy's shared internal
		// virtual-desktop coordinate space, and to decide whether a CLOSE ends just
		// that one window (win->Close()) or the whole app (Ctrl::EndSession(), main
		// window only). Never null once a WindowConn exists.
		TopWindow *win = NULL;
	};

	// One DisplayServer connection per genuine TopWindow (NetworkDisplay/0014).
	// Analogous to (and, on the DisplayServer side, indistinguishable from) any
	// other independent NetClientSession -- see DisplayServer/NetServer.h.
	struct WindowConn {
		TopWindow              *win = NULL;   // back-reference; NULL only for `pending_conn` before it's adopted
		NetworkDisplayTransport transport;
		FrameParser             parser;
		Vector<DrawCmd>         pending;       // accumulated since this window's last CommitDraw
		int                     window_id = -1;
		Size                    size = Size(0, 0);
		String                  title;
		bool                    hello_sent = false;
		bool                    got_close = false; // this connection's own close-notice de-dup flag
	};

	static void PumpOne(WindowConn& c); // reads one connection's socket, decodes frames into the event queue
	static void Pump();                 // pumps every open WindowConn (+ the not-yet-adopted pending_conn)

public:
	// Analogous in spirit to TurtleServer::Draw: an SDraw whose PutRect/PutImage
	// (the two primitives every other SDraw *Op ends up rasterized down to) become
	// DisplayServer DrawCmd's instead of Turtle's own websocket binary protocol.
	// NetworkDisplay/0014: both now route to whichever WindowConn `current` points
	// at (set by SelectWindow(), from Ctrl::PaintPerWindowScene()) instead of one
	// shared global batch, and subtract that window's own top-left so coordinates
	// recorded here are window-local, not shared-virtual-desktop-absolute.
	struct Draw : SDraw {
		Draw();
		void            Init(const Size& sz);
		void            PutImage(Point p, const Image& img, const Rect& src) final;
		void            PutRect(const Rect& r, Color color) final;
		Point           pos;
		SystemDraw      sysdraw;
	};

	friend struct NetDpyServer::Draw;

private:
	static ArrayMap<TopWindow *, WindowConn> conns;   // keyed by the TopWindow* itself
	static WindowConn              *current;          // set by SelectWindow(), consumed by Draw::Put*
	// The connection Connect() opened before any TopWindow existed yet (there is no
	// TopWindow to key it by at that point). Adopted wholesale -- not duplicated --
	// by the first-ever WindowOpened() call, which in practice is always the app's
	// real main window (apps always open their main window before any dialog).
	static WindowConn               *pending_conn;

	static String  host; // recorded from Connect() so later per-window Connect() calls reuse the same endpoint
	static int     port;

	static Vector<QueuedEvent>      events;

	static Size    canvas_size; // NetDpy's own shared internal "virtual desktop" size (Ctrl::SetDesktopSize) -- unrelated to any one window's own DisplayServer canvas size
	static dword   mousebuttons;
	static bool    mousein;

	friend void RunNetDpyGui(NetDpyServer&, Event<>);
};

void RunNetDpyGui(NetDpyServer& gui, Event<> app_main);

}  // namespace Upp

#define GUIPLATFORM_INCLUDE_AFTER <VirtualGui/After.h>

#endif
