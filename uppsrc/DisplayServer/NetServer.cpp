#include "NetServer.h"
#include "Common.h"

NAMESPACE_UPP

#ifdef flagGUI

void NetClientSession::EnsureCanvas()
{
	Size sz = size;
	if(sz.cx <= 0 || sz.cy <= 0)
		sz = Size(640, 480);
	if(canvas.IsEmpty() || canvas_alloc_size != sz) {
		canvas.Create(sz);
		canvas_alloc_size = sz;
		canvas->DrawRect(sz, Black());
		snapshot = *canvas;
	}
}

void NetClientSession::ApplyDrawBatch(const Vector<DrawCmd>& cmds)
{
	EnsureCanvas();
	ImageDraw& w = *canvas;
	for(const DrawCmd& c : cmds) {
		Color col(c.r, c.g, c.b);
		switch(c.op) {
		case DOP_CLEAR:
			w.DrawRect(canvas_alloc_size, col);
			break;
		case DOP_RECT:
			w.DrawRect(c.x, c.y, c.w, c.h, col);
			break;
		case DOP_LINE:
			w.DrawLine(c.x, c.y, c.x2, c.y2, c.width, col);
			break;
		case DOP_ELLIPSE:
			w.DrawEllipse(c.x, c.y, c.w, c.h, col);
			break;
		case DOP_TEXT:
			w.DrawText(c.x, c.y, c.text, StdFont(), col);
			break;
		case DOP_IMAGE: {
			// Raw opaque RGB blit (NetworkDisplay/0007 protocol extension -- see
			// Net/Protocol.h's DOP_IMAGE comment): rebuild an Image from the raw
			// row-major r,g,b bytes and blit it in one go.
			if(c.w > 0 && c.h > 0 && c.image_rgb.GetCount() == c.w * c.h * 3) {
				ImageBuffer ib(c.w, c.h);
				const byte *src = (const byte *)~c.image_rgb;
				RGBA *dst = ~ib;
				for(int i = 0; i < c.w * c.h; i++) {
					dst[i].r = src[3 * i + 0];
					dst[i].g = src[3 * i + 1];
					dst[i].b = src[3 * i + 2];
					dst[i].a = 255;
				}
				w.DrawImage(c.x, c.y, Image(ib));
			}
			break;
		}
		default:
			break;
		}
	}
	snapshot = *canvas;
	draw_cmd_total += cmds.GetCount();
}

bool NetServer::Start(int port)
{
	bool ok = listener.Listen(port);
	if(g_verbose)
		fprintf(stderr, "[net] %s on port %d\n", ok ? "listening" : "FAILED to listen", port);
	return ok;
}

void NetServer::AcceptNew()
{
	One<NetworkDisplayTransport> t = listener.Accept();
	if(t.IsEmpty())
		return;
	int idx = next_index++;
	String peer = t->Describe();
	NetClientSession& c = clients.Add(idx);
	c.index = idx;
	c.transport = pick(t);
	LogNetConnect(idx, peer);
}

void NetServer::PumpClient(NetClientSession& c)
{
	if(c.transport.IsEmpty())
		return;

	byte buf[8192];
	bool closed = false;
	for(;;) {
		int n = c.transport->Receive(buf, sizeof(buf));
		if(n < 0) {
			closed = true;
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
		case CMSG_HELLO: {
			String title;
			Size sz;
			if(!c.hello_received && DecodeHello(payload, title, sz)) {
				c.hello_received = true;
				c.title = title;
				c.size = sz;
				WhenConnect(c.index);
			}
			break;
		}
		case CMSG_DRAW_BATCH: {
			Vector<DrawCmd> cmds;
			if(c.hello_received && DecodeDrawBatch(payload, cmds)) {
				c.ApplyDrawBatch(cmds);
				LogNetDraw(c.index, cmds.GetCount(), c.draw_cmd_total);
				WhenDrawBatch(c.index);
			}
			break;
		}
		case CMSG_RESIZE: {
			Size sz;
			if(c.hello_received && DecodeSize(payload, sz))
				c.size = sz;
			break;
		}
		case CMSG_BYE:
			closed = true;
			break;
		default:
			break;
		}
	}

	if(closed && !c.disconnect_notified) {
		c.disconnect_notified = true;
		LogNetDisconnect(c.index);
		if(c.hello_received) {
			WhenDisconnect(c.index); // backend owns tearing down the window + calling CloseAndRemove
			return;
		}
		clients.RemoveKey(c.index); // never got far enough to have a window; nothing to tear down
	}
}

void NetServer::Poll()
{
	AcceptNew();
	for(int i = clients.GetCount() - 1; i >= 0; i--)
		PumpClient(clients[i]);
}

NetClientSession *NetServer::Find(int client_index)
{
	return clients.FindPtr(client_index);
}

Image NetServer::GetSnapshot(int client_index)
{
	NetClientSession *c = Find(client_index);
	return c ? c->snapshot : Image();
}

void NetServer::SendWelcome(int client_index, int window_id)
{
	NetClientSession *c = Find(client_index);
	if(!c || c->transport.IsEmpty() || !c->transport->IsOpen())
		return;
	c->transport->Send(EncodeFrame(SMSG_WELCOME, EncodeWelcome(window_id)));
}

void NetServer::SendMouse(int client_index, byte kind, Point p, dword keyflags)
{
	NetClientSession *c = Find(client_index);
	if(!c || c->transport.IsEmpty() || !c->transport->IsOpen())
		return;
	c->transport->Send(EncodeFrame(SMSG_INPUT_MOUSE, EncodeInputMouse(kind, p, keyflags)));
}

void NetServer::SendKey(int client_index, byte kind, dword keycode)
{
	NetClientSession *c = Find(client_index);
	if(!c || c->transport.IsEmpty() || !c->transport->IsOpen())
		return;
	c->transport->Send(EncodeFrame(SMSG_INPUT_KEY, EncodeInputKey(kind, keycode)));
}

void NetServer::CloseAndRemove(int client_index)
{
	int pos = clients.Find(client_index);
	if(pos < 0)
		return;
	NetClientSession& c = clients[pos];
	if(!c.transport.IsEmpty() && c.transport->IsOpen())
		c.transport->Send(EncodeFrame(SMSG_CLOSE, String()));
	if(!c.transport.IsEmpty())
		c.transport->Close();
	clients.Remove(pos);
}

#endif

END_UPP_NAMESPACE
