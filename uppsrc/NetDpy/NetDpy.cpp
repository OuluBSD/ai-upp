#include <CtrlLib/CtrlLib.h>
#include "NetDpy.h"

#define LLOG(x)     // LLOG(x)

namespace Upp {

NetworkDisplayTransport      NetDpyServer::transport;
FrameParser                  NetDpyServer::parser;
Vector<NetDpyServer::QueuedEvent> NetDpyServer::events;
Vector<DrawCmd>               NetDpyServer::pending;

Size    NetDpyServer::canvas_size = Size(900, 700);
dword   NetDpyServer::mousebuttons = 0;
bool    NetDpyServer::mousein = true;
bool    NetDpyServer::got_close = false;
int     NetDpyServer::window_id = -1;

static NetDpyServer::Draw sNetDpyDraw;

bool NetDpyServer::Connect(const String& host, int port, const String& title, Size sz)
{
	canvas_size = sz;
	sNetDpyDraw.Init(sz);
	String endpoint = host + ":" + AsString(port);
	if(!transport.Connect(endpoint))
		return false;
	transport.Send(EncodeFrame(CMSG_HELLO, EncodeHello(title, sz)));
	return true;
}

bool NetDpyServer::IsConnected() const
{
	return transport.IsOpen();
}

void NetDpyServer::Quit()
{
	if(transport.IsOpen()) {
		transport.Send(EncodeFrame(CMSG_BYE, String()));
		transport.Close();
	}
}

void NetDpyServer::Pump()
{
	if(!transport.IsOpen())
		return;

	byte buf[16384];
	for(;;) {
		int n = transport.Receive(buf, sizeof(buf));
		if(n < 0) { // connection lost -- treat exactly like an explicit server close
			if(!got_close) {
				got_close = true;
				QueuedEvent& e = events.Add();
				e.kind = QueuedEvent::CLOSE;
			}
			break;
		}
		if(n == 0)
			break;
		parser.Feed(buf, n);
	}

	byte type;
	String payload;
	while(parser.Next(type, payload)) {
		switch(type) {
		case SMSG_WELCOME:
			DecodeWelcome(payload, window_id);
			break;
		case SMSG_INPUT_MOUSE: {
			QueuedEvent& e = events.Add();
			e.kind = QueuedEvent::MOUSE;
			DecodeInputMouse(payload, e.mkind, e.pos, e.keyflags);
			break;
		}
		case SMSG_INPUT_KEY: {
			QueuedEvent& e = events.Add();
			e.kind = QueuedEvent::KEY;
			DecodeInputKey(payload, e.kkind, e.keycode);
			break;
		}
		case SMSG_WINDOW_RESIZED:
			// Not currently emitted by DisplayServer (documented NetworkDisplay/0006
			// known limitation: no live host-resize renegotiation yet); ignored.
			break;
		case SMSG_CLOSE:
			if(!got_close) {
				got_close = true;
				QueuedEvent& e = events.Add();
				e.kind = QueuedEvent::CLOSE;
			}
			break;
		default:
			break;
		}
	}
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
		Ctrl::DoMouseFB(ev, e.pos, 0);
		return true;
	}
	case QueuedEvent::KEY:
		// e.keycode is already a raw Upp K_ code (incl. modifier bits), forwarded
		// verbatim by DisplayServer from its own Ctrl::Key() -- no translation
		// needed, unlike Turtle's browser-keycode table.
		Ctrl::DoKeyFB(e.keycode, 1);
		return true;
	case QueuedEvent::CLOSE:
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
	if(pending.GetCount() && transport.IsOpen())
		transport.Send(EncodeFrame(CMSG_DRAW_BATCH, EncodeDrawBatch(pending)));
	pending.Clear();
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

	DrawCmd& c = NetDpyServer::pending.Add();
	c.op = DOP_RECT;
	Size sz = r.GetSize();
	c.x = r.left; c.y = r.top; c.w = sz.cx; c.h = sz.cy;
	c.r = color.GetR(); c.g = color.GetG(); c.b = color.GetB();
}

void NetDpyServer::Draw::PutImage(Point p, const Image& img, const Rect& src)
{
	Size sz = src.GetSize();
	if(sz.cx <= 0 || sz.cy <= 0)
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
	DrawCmd& c = NetDpyServer::pending.Add();
	c.op = DOP_IMAGE;
	c.x = p.x; c.y = p.y; c.w = sz.cx; c.h = sz.cy;

	StringBuffer sb(sz.cx * sz.cy * 4);
	byte *out = sb;
	for(int y = 0; y < sz.cy; y++) {
		const RGBA *row = img[src.top + y] + src.left;
		for(int x = 0; x < sz.cx; x++) {
			RGBA px = row[x];
			*out++ = px.r; *out++ = px.g; *out++ = px.b; *out++ = px.a;
		}
	}
	c.image_rgba = String(sb);
}

void RunNetDpyGui(NetDpyServer& gui, Event<> app_main)
{
	RunVirtualGui((VirtualGui&)gui, app_main);
}

}  // namespace Upp

#ifdef flagMAIN // For automated builds.
CONSOLE_APP_MAIN {}
#endif
