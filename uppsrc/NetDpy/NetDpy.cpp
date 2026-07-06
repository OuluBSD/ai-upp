#include <CtrlLib/CtrlLib.h>
#include "NetDpy.h"

#define LLOG(x)     // LLOG(x)

namespace Upp {

ArrayMap<TopWindow *, NetDpyServer::WindowConn> NetDpyServer::conns;
NetDpyServer::WindowConn    *NetDpyServer::current = NULL;
NetDpyServer::WindowConn    *NetDpyServer::pending_conn = NULL;

String  NetDpyServer::host;
int     NetDpyServer::port = 0;

Vector<NetDpyServer::QueuedEvent> NetDpyServer::events;

Size    NetDpyServer::canvas_size = Size(900, 700);
dword   NetDpyServer::mousebuttons = 0;
bool    NetDpyServer::mousein = true;

static NetDpyServer::Draw sNetDpyDraw;

static void LogWindowConnect(const String& title, Size sz)
{
	// NetworkDisplay/0014: real stderr output (not an LLOG, those compile out by
	// default), so a NetDpy-hosted app's per-window connect/disconnect can be
	// correlated 1:1 against DisplayServer's own --verbose [net] client N
	// connected/disconnected log lines in the same test run.
	fprintf(stderr, "[netdpy] window connect: title=\"%s\" size=%dx%d\n",
	        title.Begin(), sz.cx, sz.cy);
	fflush(stderr);
}

static void LogWindowDisconnect(const String& title)
{
	fprintf(stderr, "[netdpy] window disconnect: title=\"%s\"\n", title.Begin());
	fflush(stderr);
}

static void LogWindowTitleChanged(const String& title)
{
	// NetworkDisplay/0015: same real-stderr-output rationale as LogWindowConnect.
	fprintf(stderr, "[netdpy] window title changed: \"%s\"\n", title.Begin());
	fflush(stderr);
}

static void LogWindowResized(const String& title, Size sz)
{
	fprintf(stderr, "[netdpy] window resized: title=\"%s\" size=%dx%d\n", title.Begin(), sz.cx,
	        sz.cy);
	fflush(stderr);
}

bool NetDpyServer::Connect(const String& h, int p, const String& title, Size sz)
{
	host = h;
	port = p;
	canvas_size = sz;
	sNetDpyDraw.Init(sz);

	pending_conn = new WindowConn;
	pending_conn->size = sz;
	pending_conn->title = title;

	String endpoint = host + ":" + AsString(port);
	if(!pending_conn->transport.Connect(endpoint))
		return false;
	pending_conn->transport.Send(EncodeFrame(CMSG_HELLO, EncodeHello(title, sz)));
	pending_conn->hello_sent = true;
	return true;
}

bool NetDpyServer::IsConnected() const
{
	if(pending_conn)
		return pending_conn->transport.IsOpen();
	for(int i = 0; i < conns.GetCount(); i++)
		if(conns[i].transport.IsOpen())
			return true;
	return false;
}

void NetDpyServer::Quit()
{
	for(int i = 0; i < conns.GetCount(); i++) {
		WindowConn& c = conns[i];
		if(c.transport.IsOpen()) {
			c.transport.Send(EncodeFrame(CMSG_BYE, String()));
			c.transport.Close();
		}
	}
	if(pending_conn && pending_conn->transport.IsOpen()) {
		pending_conn->transport.Send(EncodeFrame(CMSG_BYE, String()));
		pending_conn->transport.Close();
	}
}

void NetDpyServer::WindowOpened(TopWindow *w)
{
	if(!w || conns.Find(w) >= 0)
		return;

	// NetworkDisplay/0017: resolve which already-open window (if any) this one is a
	// popup/dialog of, so its CMSG_HELLO can tell DisplayServer which existing window
	// to center over instead of falling back to AddInterface()'s generic staggered
	// default position. w->GetOwner() (VirtualGui/Wnd.cpp) is already correctly set by
	// the time Ctrl::PopUp() calls WindowOpened() here -- see TopWindow::Open()'s
	// frame->PopUp(owner, ...) / PopUp(frame, ...) pair, which resolves all the way
	// through the (invisible, chrome-suppressed for NetDpy) TopWindowFrame's own
	// owner_window to the real ancestor TopWindow. Only usable once that owner already
	// has its own DisplayServer-assigned window_id (from a previously-received
	// SMSG_WELCOME on the owner's own connection, recorded in conns[oi].window_id) --
	// in practice always true here, since an owner window is always opened (and its
	// connection pumped at least once) well before any dialog/popup of its own opens.
	// If the owner's WELCOME genuinely hasn't arrived yet, owner_window_id is left at
	// -1 and the popup just falls back to the generic default position, same as if it
	// had no owner.
	int owner_window_id = -1;
	if(TopWindow *owner_win = dynamic_cast<TopWindow *>(w->GetOwner())) {
		int oi = conns.Find(owner_win);
		if(oi >= 0 && conns[oi].window_id >= 0)
			owner_window_id = conns[oi].window_id;
	}

	WindowConn *c;
	if(pending_conn) {
		// Adopt the connection Connect() already opened before any TopWindow
		// existed, instead of wastefully opening a second one for the very first
		// window that ever opens (always the app's real main window -- apps open
		// their main window before any dialog/popup can exist). See
		// NetworkDisplay/0014's plan doc, design proposal point 2. Always ownerless
		// (it's the main window), so its already-sent CMSG_HELLO (from Connect(),
		// before any TopWindow existed to resolve an owner from anyway) correctly
		// carried EncodeHello's default owner_window_id of -1.
		c = pending_conn;
		pending_conn = NULL;
		conns.Add(w, c);
	}
	else {
		c = &conns.Add(w);
		c->size = w->GetRect().GetSize();
		c->title = w->GetTitle().ToString();
		String endpoint = host + ":" + AsString(port);
		if(c->transport.Connect(endpoint)) {
			c->transport.Send(EncodeFrame(CMSG_HELLO, EncodeHello(c->title, c->size, owner_window_id)));
			c->hello_sent = true;
		}
	}
	c->win = w;
	LogWindowConnect(c->title, c->size);
}

void NetDpyServer::WindowClosed(TopWindow *w)
{
	if(!w)
		return;
	int i = conns.Find(w);
	if(i < 0)
		return;
	WindowConn& c = conns[i];
	LogWindowDisconnect(c.title);
	if(c.transport.IsOpen()) {
		c.transport.Send(EncodeFrame(CMSG_BYE, String()));
		c.transport.Close();
	}
	if(current == &c)
		current = NULL;
	conns.Remove(i);
}

void NetDpyServer::WindowTitleChanged(TopWindow *w)
{
	if(!w)
		return;
	int i = conns.Find(w);
	if(i < 0)
		return;
	WindowConn& c = conns[i];
	c.title = w->GetTitle().ToString();
	if(c.transport.IsOpen())
		c.transport.Send(EncodeFrame(CMSG_TITLE, EncodeTitle(c.title)));
	LogWindowTitleChanged(c.title);
}

void NetDpyServer::SelectWindow(TopWindow *w)
{
	int i = w ? conns.Find(w) : -1;
	current = i >= 0 ? &conns[i] : NULL;
}

void NetDpyServer::PumpOne(WindowConn& c)
{
	if(!c.transport.IsOpen())
		return;

	byte buf[16384];
	for(;;) {
		int n = c.transport.Receive(buf, sizeof(buf));
		if(n < 0) { // connection lost -- treat exactly like an explicit server close
			if(!c.got_close) {
				c.got_close = true;
				QueuedEvent& e = events.Add();
				e.kind = QueuedEvent::CLOSE;
				e.win = c.win;
			}
			break;
		}
		if(n == 0)
			break;
		c.parser.Feed(buf, n);
	}

	byte type;
	String payload;
	while(c.parser.Next(type, payload)) {
		switch(type) {
		case SMSG_WELCOME:
			DecodeWelcome(payload, c.window_id);
			break;
		case SMSG_INPUT_MOUSE: {
			QueuedEvent& e = events.Add();
			e.kind = QueuedEvent::MOUSE;
			e.win = c.win;
			DecodeInputMouse(payload, e.mkind, e.pos, e.keyflags);
			break;
		}
		case SMSG_INPUT_KEY: {
			QueuedEvent& e = events.Add();
			e.kind = QueuedEvent::KEY;
			e.win = c.win;
			DecodeInputKey(payload, e.kkind, e.keycode);
			break;
		}
		case SMSG_WINDOW_RESIZED: {
			// NetworkDisplay/0015: DisplayServer's real Ctrl/Display frame around
			// this window was resized (maximize/restore/manual drag-resize) --
			// update this window's own size to match and let it actually
			// relayout/repaint at the new size, instead of the previous no-op
			// (which left the client rendering at its stale original size while
			// DisplayServer just filled the enlarged frame with black).
			Size sz;
			if(DecodeSize(payload, sz) && c.win) {
				c.size = sz;
				Rect r = c.win->GetRect();
				r.right = r.left + sz.cx;
				r.bottom = r.top + sz.cy;
				c.win->SetRect(r); // triggers Ctrl's normal resize/relayout/repaint path
				// The shared virtual-desktop bound (canvas_size/sNetDpyDraw) is what
				// Ctrl::PaintPerWindowScene() (VirtualGui/Wnd.cpp) seeds its per-pass
				// paintable region from (VirtualGuiPtr->GetSize()) -- grow it if this
				// window's new rect no longer fits inside it, or painting beyond the
				// old bound would silently clip. Mirrors the same sNetDpyDraw.Init()
				// call Connect() already does once at startup; unrelated to any one
				// window's own per-connection canvas size (see canvas_size's own doc
				// comment above).
				Size needed(max(canvas_size.cx, r.right), max(canvas_size.cy, r.bottom));
				if(needed != canvas_size) {
					canvas_size = needed;
					sNetDpyDraw.Init(needed);
				}
				LogWindowResized(c.title, sz);
			}
			break;
		}
		case SMSG_CLOSE:
			if(!c.got_close) {
				c.got_close = true;
				QueuedEvent& e = events.Add();
				e.kind = QueuedEvent::CLOSE;
				e.win = c.win;
			}
			break;
		default:
			break;
		}
	}
}

void NetDpyServer::Pump()
{
	for(int i = 0; i < conns.GetCount(); i++)
		PumpOne(conns[i]);
	// pending_conn (not yet adopted by any TopWindow) never carries real window
	// input -- DisplayServer only starts forwarding SMSG_INPUT_*/SMSG_CLOSE after
	// SMSG_WELCOME, which nothing consumes until WindowOpened() adopts it anyway.
}

bool NetDpyServer::IsWaitingEvent()
{
	Pump();
	return events.GetCount() > 0;
}

void NetDpyServer::WaitEvent(int ms)
{
	Sleep(min(ms, 20));
	Pump();
}

bool NetDpyServer::ProcessEvent(bool *quit)
{
	if(events.IsEmpty())
		Pump();
	if(events.IsEmpty())
		return false;

	QueuedEvent e = events[0];
	events.Remove(0);

	// NetworkDisplay/0014: input coordinates arrive window-local (from whichever
	// window's own DisplayServer connection produced them); translate back into
	// NetDpy's shared internal virtual-desktop coordinate space before dispatching
	// through the unmodified Ctrl::DoMouseFB/DoKeyFB -- topctrl's existing
	// rect-hit-testing keeps working exactly as before, since every window's
	// GetRect() is still a real, distinct rect in that shared space.
	Point off = e.win ? e.win->GetRect().TopLeft() : Point(0, 0);

	switch(e.kind) {
	case QueuedEvent::MOUSE: {
		int ev = Ctrl::MOUSEMOVE;
		switch(e.mkind) {
		case MOUSE_MOVE:       ev = Ctrl::MOUSEMOVE;          break;
		case MOUSE_LEFT_DOWN:  ev = Ctrl::LEFT | Ctrl::DOWN;  mousebuttons |= 1; break;
		case MOUSE_LEFT_UP:    ev = Ctrl::LEFT | Ctrl::UP;    mousebuttons &= ~1; break;
		case MOUSE_RIGHT_DOWN: ev = Ctrl::RIGHT | Ctrl::DOWN; mousebuttons |= 2; break;
		case MOUSE_RIGHT_UP:   ev = Ctrl::RIGHT | Ctrl::UP;   mousebuttons &= ~2; break;
		default: break;
		}
		mousein = true;
		Ctrl::DoMouseFB(ev, e.pos + off, 0);
		return true;
	}
	case QueuedEvent::KEY:
		// e.keycode is already a raw Upp K_ code (incl. modifier bits), forwarded
		// verbatim by DisplayServer from its own Ctrl::Key() -- no translation
		// needed, unlike Turtle's browser-keycode table.
		Ctrl::DoKeyFB(e.keycode, 1);
		return true;
	case QueuedEvent::CLOSE:
		// NetworkDisplay/0014: the main window's connection closing still ends the
		// whole app (Ctrl::EndSession(), as before 0014); any other (popup/dialog)
		// window's connection closing just closes that one window -- exactly like
		// clicking its own close button would -- leaving the app and every other
		// open window running.
		if(e.win && e.win->GetMainWindow() != e.win)
			e.win->Close();
		else
			Ctrl::EndSession();
		if(quit)
			*quit = true;
		return true;
	}
	return true;
}

SystemDraw& NetDpyServer::BeginDraw()
{
	return sNetDpyDraw.sysdraw;
}

void NetDpyServer::CommitDraw()
{
	if(current && current->pending.GetCount() && current->transport.IsOpen())
		current->transport.Send(EncodeFrame(CMSG_DRAW_BATCH, EncodeDrawBatch(current->pending)));
	if(current)
		current->pending.Clear();
}

NetDpyServer::Draw::Draw()
{
	pos = Point(0, 0);
	PaintOnly();
	sysdraw.SetTarget(this);
	SDraw::Init(NetDpyServer::canvas_size);
}

void NetDpyServer::Draw::Init(const Size& sz)
{
	SDraw::Init(sz);
}

void NetDpyServer::Draw::PutRect(const Rect& r, Color color)
{
	if(color == InvertColor()) // DisplayServer's protocol has no invert-rect primitive
		return;                 // (documented limitation -- caret/selection XOR effects are skipped)

	NetDpyServer::WindowConn *c = NetDpyServer::current;
	if(!c)
		return; // nothing selected (e.g. PaintCaretCursor called outside any cluster) -- drop, not crash

	Point off = c->win ? c->win->GetRect().TopLeft() : Point(0, 0);
	DrawCmd& d = c->pending.Add();
	d.op = DOP_RECT;
	Size sz = r.GetSize();
	d.x = r.left - off.x; d.y = r.top - off.y; d.w = sz.cx; d.h = sz.cy;
	d.r = color.GetR(); d.g = color.GetG(); d.b = color.GetB();
}

void NetDpyServer::Draw::PutImage(Point p, const Image& img, const Rect& src)
{
	Size sz = src.GetSize();
	if(sz.cx <= 0 || sz.cy <= 0)
		return;

	NetDpyServer::WindowConn *c = NetDpyServer::current;
	if(!c)
		return;

	// DisplayServer's baseline NetworkDisplay/0006 protocol only had flat-color
	// primitives (RECT/LINE/ELLIPSE/TEXT); PutImage is the seam every non-trivial
	// Ctrl paint (glyphs, icons, gradients, ...) actually rasterizes through, so a
	// generic backend needs a real raster blit. Extended the wire protocol here with
	// DOP_IMAGE (see Net/Protocol.h) -- a real RGBA blit.
	//
	// NetworkDisplay/0011: `img`'s own RGBA pixels are already alpha-premultiplied
	// (U++'s standard internal Image storage convention for IMAGE_ALPHA content --
	// see Premultiply()/AlphaBlend() in uppsrc/Draw/Image.h), which is exactly the
	// representation the server side needs to do a correct alpha-over composite. So
	// just send each pixel's real, un-discarded r,g,b,a verbatim -- no unmultiply,
	// no dropping alpha into a hardcoded opaque/black fallback (the previous 0007
	// RGB-only shape had to do that, which is what produced solid black boxes
	// wherever the source was transparent instead of blending against the real
	// window background).
	Point off = c->win ? c->win->GetRect().TopLeft() : Point(0, 0);
	DrawCmd& d = c->pending.Add();
	d.op = DOP_IMAGE;
	d.x = p.x - off.x; d.y = p.y - off.y; d.w = sz.cx; d.h = sz.cy;

	StringBuffer sb(sz.cx * sz.cy * 4);
	byte *out = sb;
	for(int y = 0; y < sz.cy; y++) {
		const RGBA *row = img[src.top + y] + src.left;
		for(int x = 0; x < sz.cx; x++) {
			RGBA px = row[x];
			*out++ = px.r; *out++ = px.g; *out++ = px.b; *out++ = px.a;
		}
	}
	d.image_rgba = String(sb);
}

void RunNetDpyGui(NetDpyServer& gui, Event<> app_main)
{
	RunVirtualGui((VirtualGui&)gui, app_main);
}

}  // namespace Upp

#ifdef flagMAIN // For automated builds.
CONSOLE_APP_MAIN {}
#endif
