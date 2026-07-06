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
// "desktop" (one DisplayServer connection/window), so there is never a second
// instance to keep separate from the free-standing Draw/event-queue state.
class NetDpyServer : public VirtualGui {
public:
	NetDpyServer() {}

	// Connects to a DisplayServer listening at host:port and sends CMSG_HELLO with
	// the given title/canvas size. Must be called (successfully) before
	// RunNetDpyGui(). The canvas size is fixed for the lifetime of the connection --
	// live resize renegotiation is out of scope here, same known limitation
	// NetworkDisplay/0006 already documented for SMSG_WINDOW_RESIZED/CMSG_RESIZE.
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

private:
	struct QueuedEvent : Moveable<QueuedEvent> {
		enum Kind { MOUSE, KEY, CLOSE } kind;
		byte  mkind = 0;
		byte  kkind = 0;
		Point pos   = Point(0, 0);
		dword keyflags = 0;
		dword keycode = 0;
	};

	static void Pump(); // reads the socket, decodes frames into the event queue

public:
	// Analogous in spirit to TurtleServer::Draw: an SDraw whose PutRect/PutImage
	// (the two primitives every other SDraw *Op ends up rasterized down to) become
	// DisplayServer DrawCmd's instead of Turtle's own websocket binary protocol.
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
	static NetworkDisplayTransport transport;
	static FrameParser              parser;
	static Vector<QueuedEvent>      events;
	static Vector<DrawCmd>          pending; // accumulated since the last CommitDraw

	static Size    canvas_size;
	static dword   mousebuttons;
	static bool    mousein;
	static bool    got_close;
	static int     window_id;

	friend void RunNetDpyGui(NetDpyServer&, Event<>);
};

void RunNetDpyGui(NetDpyServer& gui, Event<> app_main);

}  // namespace Upp

#define GUIPLATFORM_INCLUDE_AFTER <VirtualGui/After.h>

#endif
