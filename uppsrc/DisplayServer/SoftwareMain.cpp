// DisplayServer, software-rendered backend: composites N synthetic framed windows
// via plain Ctrl/Draw/Painter, using Ctrl/Display's ScopeT<CtxUpp2D>/FrameT<CtxUpp2D>
// as-is (no changes to the Dim traits struct needed for this backend).
#ifndef flagGL

#include "Common.h"

using namespace Upp;

class DesktopWindow : public TopWindow {
public:
	typedef DesktopWindow CLASSNAME;

	WindowManager           scope; // ScopeT<CtxUpp2D>
	Array<SyntheticContent> contents;
	Array<Button>           taskbar;
	static constexpr int    TASKBAR_H = 28;

	// NetworkDisplay/0006: real client connections, each with its own framed window
	// created through the exact same scope.AddInterface() path as the synthetic
	// windows above (see AddNetworkWindow()). Network windows don't get a taskbar
	// button (only the fixed startup demo windows do) -- they are otherwise fully
	// real, framed, movable/focusable/closable windows.
	NetServer            net;
	Array<NetworkContent> net_contents;

	DesktopWindow()
	{
		Title("DisplayServer -- software backend");
		Sizeable().Zoomable();
		SetRect(0, 0, 1000, 700);
		Add(scope.GetDesktop());

		net.WhenConnect = [this](int client_index) {
			NetClientSession *c = net.Find(client_index);
			if(c)
				AddNetworkWindow(client_index, c->title, c->size);
		};
		net.WhenDrawBatch = [this](int client_index) {
			for(NetworkContent& c : net_contents)
				if(c.client_index == client_index) {
					c.Refresh();
					break;
				}
		};
		net.WhenDisconnect = [this](int client_index) {
			for(NetworkContent& c : net_contents)
				if(c.client_index == client_index) {
					scope.QueueCloseHandle(c.id);
					break;
				}
		};
		// NetworkDisplay/0015: the client's hosted TopWindow's title changed after
		// connect -- update this window's live DisplayServer frame title to match
		// (previously only ever set once, from CMSG_HELLO, at connect time).
		net.WhenTitleChanged = [this](int client_index) {
			NetClientSession *c = net.Find(client_index);
			if(!c)
				return;
			for(NetworkContent& nc : net_contents)
				if(nc.client_index == client_index) {
					WindowManager::Frame *f = FindFrame(nc.id);
					if(f) {
						f->SetTitle(c->title);
						f->Refresh();
					}
					nc.title = c->title;
					break;
				}
		};
		// Closing is always deferred (QueueCloseHandle -> next tick), whether it was
		// triggered by the frame's own close button (user click) or by us above (a
		// client disconnecting) -- either way, once the handle is actually gone from
		// `scope`, tear down the matching net_contents entry + NetServer session.
		scope.WhenHandleClose << [this] { PruneClosedNetworkWindows(); };
	}

	// Finds the live ScopeT<CtxUpp2D> Frame for a given handle id, or NULL if it has
	// since been closed. Needed here (not just by GLMain.cpp's own copy) to reach a
	// network window's Frame again after connect time, e.g. to update its title live
	// (NetworkDisplay/0015).
	WindowManager::Frame *FindFrame(int id)
	{
		for(int p = 0; p < scope.GetCount(); p++)
			if(scope.GetPosId(p) == id)
				return &scope[p];
		return NULL;
	}

	void AddNetworkWindow(int client_index, const String& title, Size csize)
	{
		scope.AddInterface(*this);
		int pos = scope.GetCount() - 1;
		WindowManager::Frame& h = scope[pos];
		int id = scope.GetPosId(pos);
		h.SetTitle(title.GetCount() ? title : Format("Client %d", client_index));

		// NetworkDisplay/0015: size the frame to the network client's own real
		// declared canvas size (from CMSG_HELLO), instead of leaving
		// ScopeT<Dim>::AddInterface()'s generic default handle dimensions in place
		// -- scoped to network windows only, via SetClient() (already-existing
		// FrameT API: computes the frame box that yields this exact client rect,
		// same margins/titlebar math GetClient() uses in reverse). Synthetic
		// AddWindow() windows below are untouched, so --demo-windows keeps its
		// existing default sizing.
		if(csize.cx > 0 && csize.cy > 0) {
			Rect box = h.GetFrameBox();
			h.SetClient(RectC(box.left, box.top, csize.cx, csize.cy));
		}

		NetworkContent& content = net_contents.Add();
		content.id = id;
		content.client_index = client_index;
		content.title = h.GetTitle();
		content.scope = &scope;
		content.net = &net;
		Rect m = h.Margins();
		int top_h = m.top + GetStdFontCy() + 4;
		h.Add(content.HSizePos(m.left, m.right).VSizePos(top_h, m.bottom));

		net.SendWelcome(client_index, id);
		Layout();
	}

	// Mirrors the OpenGL backend's PruneClosed() idiom: scope.WhenHandleClose fires
	// with no id (see ScopeT<Dim>::CloseHandle), so the only reliable way to learn
	// *which* handle just disappeared is to compare our own bookkeeping against
	// what's still actually in `scope`.
	void PruneClosedNetworkWindows()
	{
		for(int i = net_contents.GetCount() - 1; i >= 0; i--) {
			int id = net_contents[i].id;
			bool still_present = false;
			for(int p = 0; p < scope.GetCount(); p++)
				if(scope.GetPosId(p) == id) {
					still_present = true;
					break;
				}
			if(!still_present) {
				net.CloseAndRemove(net_contents[i].client_index);
				net_contents.Remove(i);
			}
		}
	}

	void Layout() override
	{
		Size sz = GetSize();
		Rect desktop_rect(0, 0, sz.cx, max(0, sz.cy - TASKBAR_H));
		scope.SetFrameBox(desktop_rect); // also sizes scope.GetDesktop()

		int x = 4, y = desktop_rect.bottom + 2;
		for(Button& b : taskbar) {
			b.SetRect(x, y, 120, TASKBAR_H - 4);
			x += 124;
		}
	}

	void AddWindow(const WindowSpec& spec)
	{
		scope.AddInterface(*this);
		int pos = scope.GetCount() - 1;
		WindowManager::Frame& h = scope[pos];
		int id = scope.GetPosId(pos);
		h.SetTitle(spec.title);

		SyntheticContent& content = contents.Add();
		content.id = id;
		content.spec = spec;
		content.scope = &scope;
		Rect m = h.Margins();
		int top_h = m.top + GetStdFontCy() + 4;
		h.Add(content.HSizePos(m.left, m.right).VSizePos(top_h, m.bottom));

		Button& b = taskbar.Add();
		b.SetLabel(spec.title);
		b.WhenPush = [this, id] { scope.FocusHandle(id); };
		Add(b);

		Layout();
	}

	void Paint(Draw& w) override
	{
		LogComposite("software", scope.GetCount());
		TopWindow::Paint(w);
		Size sz = GetSize();
		w.DrawRect(RectC(0, max(0, sz.cy - TASKBAR_H), sz.cx, TASKBAR_H), SColorFace());
	}
};

GUI_APP_MAIN
{
	ParseVerboseFlag();
	ParseDemoWindowsFlag();
	if(g_verbose)
		fprintf(stderr, "[software] DisplayServer starting, verbose logging enabled\n");

	Ctrl::GlobalBackPaint();

	DesktopWindow win;
	for(const WindowSpec& spec : DefaultWindowSpecs())
		win.AddWindow(spec);

	int port = GetDisplayServerPort();
	win.net.Start(port);

	win.Open();

	// Poll the (non-blocking) network layer from inside the same GUI event loop that
	// already drives the synthetic windows, instead of a separate thread -- keeps
	// everything single-threaded (no locking) while still never blocking the UI,
	// since NetServer::Poll() never waits on the network.
	SetTimeCallback(-20, [&win] { win.net.Poll(); });

	win.Run();
}

#endif
