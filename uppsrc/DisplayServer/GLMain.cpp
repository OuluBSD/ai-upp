// DisplayServer, OpenGL-rendered backend.
//
// Investigation result (see plan doc / report for the full writeup): GLDraw *is*
// Draw-API-compatible (GLDraw : SDraw : Draw, GLDraw/GLDraw.h + GLDraw/SDraw.h), so
// it is perfectly usable wherever a plain `Draw&` is expected -- including calling
// FrameT<CtxUpp2D>'s existing, unmodified `Paint(Draw&)` directly. What does *not*
// work is wiring that up as automatic per-Ctrl dispatch: FrameT<Dim>::Paint(DrawT&)
// is declared `override` against Ctrl's fixed `virtual void Paint(Draw&)`, which
// requires DrawT == Draw exactly (parameter types aren't covariant in C++, so a
// hypothetical `CtxUppGL2D` with DrawT = GLDraw could not compile that override).
// GLCtrl itself already claims that exact override slot for its own purposes
// (uppsrc/GLCtrl/GLCtrl.h: `void Paint(Draw& w) override;` under PLATFORM_POSIX) and
// exposes a *different* hook, `GLPaint()`, as the actual user paint entry point
// (see reference/OpenGL, reference/GLDrawDemo) -- so a Ctrl-attached FrameT<Dim>
// would never even get its Paint() called through GLCtrl's pipeline.
//
// Given that, this backend keeps ScopeT<CtxUpp2D>/FrameT<CtxUpp2D> for exactly what
// the task doc suggested falling back to: move/focus/maximize/minimize/close STATE
// (position rects, maximized/shown flags, active-id bookkeeping) -- the ScopeT/
// FrameT instances here are real, just never attached to a visible Ctrl tree (their
// `desktop` StaticRect is never Added to this GLCtrl or its TopWindow, so normal
// Ctrl paint/input dispatch never touches them). Compositing and input hit-testing
// are done manually in GLPaint()/LeftDown()/MouseMove(), reusing FrameT::Paint(Draw&)
// for the chrome (passing a GLDraw as the Draw&, which is genuinely GPU-drawn) plus
// PaintSyntheticContent() (shared with the software backend) for window content.
#ifdef flagGL

#include "Common.h"
#include <GLCtrl/GLCtrl.h>
#include <GLDraw/GLDraw.h>

using namespace Upp;

static WindowManager::Frame *FindWindowFrame(WindowManager& scope, int id)
{
	for(int i = 0; i < scope.GetCount(); i++)
		if(scope.GetPosId(i) == id)
			return &scope[i];
	return NULL;
}

class GLDesktop : public GLCtrl {
public:
	typedef GLDesktop CLASSNAME;

	TopWindow             *host = NULL;
	WindowManager          scope; // ScopeT<CtxUpp2D>: state model only, see file header
	Vector<int>            zorder; // back = topmost (GL variant tracks its own z-order,
	                                // since it doesn't rely on Ctrl sibling order for paint)
	VectorMap<int, WindowSpec> specs;

	bool  dragging = false;
	int   drag_id = -1;
	Point drag_start;
	Rect  drag_start_rect;

	static constexpr int TASKBAR_H = 28;
	static constexpr int BTN = 20;

	GLDesktop()
	{
		// CloseHandle runs a tick after QueueCloseHandle (deferred via PostCallback,
		// see Ctrl/Display/ScopeT.h), so trigger a repaint once it actually happens --
		// otherwise the closed window would linger on screen until some unrelated
		// event (e.g. a later click) happened to trigger the next GLPaint().
		scope.WhenHandleClose << [this] { Refresh(); };
	}

	void Layout() override
	{
		Size sz = GetSize();
		scope.SetFrameBox(RectC(0, 0, sz.cx, sz.cy));
	}

	void AddWindow(const WindowSpec& spec)
	{
		ASSERT(host);
		scope.AddInterface(*host);
		int pos = scope.GetCount() - 1;
		WindowManager::Frame& h = scope[pos];
		int id = scope.GetPosId(pos);
		h.SetTitle(spec.title);
		specs.Add(id, spec);
		zorder.Add(id);
	}

	// Drops zorder/specs entries whose Frame no longer exists (closed windows are
	// removed from `scope` a tick after being queued, via ScopeT's PostCallback).
	void PruneClosed()
	{
		for(int i = zorder.GetCount() - 1; i >= 0; i--) {
			if(!FindWindowFrame(scope, zorder[i])) {
				specs.RemoveKey(zorder[i]);
				zorder.Remove(i);
			}
		}
	}

	void RaiseToTop(int id)
	{
		scope.FocusHandle(id);
		int i = FindIndex(zorder, id);
		if(i >= 0) {
			zorder.Remove(i);
			zorder.Add(id);
		}
	}

	Rect ButtonRect(WindowManager::Frame& f, int slot_from_right) const
	{
		Size sz = f.GetSize();
		Rect m = f.Margins();
		int x = sz.cx - m.right - slot_from_right * BTN;
		return Rect(x - BTN, m.top, x, m.top + BTN);
	}

	// Hit-test rect equals ButtonRect (full slot, no gap); the drawn rect is inset by
	// 1px so adjacent same-colored buttons remain visually distinct.
	static Rect Inset1(Rect r) { return Rect(r.left + 1, r.top + 1, r.right - 1, r.bottom - 1); }

	void GLPaint() override
	{
		PruneClosed();

		Size sz = GetSize();
		GLDraw w;
		w.Init(sz);

		w.DrawRect(sz, Color(40, 44, 52));

		LogComposite("opengl", zorder.GetCount());

		for(int id : zorder) {
			WindowManager::Frame *f = FindWindowFrame(scope, id);
			if(!f || !f->IsShown())
				continue;

			Rect r = f->GetFrameBox();
			if(!w.Clipoff(r))
				continue;

			f->Paint(w); // reuse the existing FrameT<CtxUpp2D> chrome painter as-is

			// FrameT's close/maximize/minimize Buttons are never part of a live Ctrl
			// tree in this backend (see file header), so draw simple stand-ins for
			// them at the same slots LeftDown() hit-tests against.
			Rect close_r = ButtonRect(*f, 1);
			Rect max_r = ButtonRect(*f, 2);
			Rect min_r = ButtonRect(*f, 3);
			w.DrawRect(Inset1(close_r), Color(176, 60, 60));
			w.DrawText(close_r.left + 6, close_r.top + 3, "x", StdFont(), White());
			w.DrawRect(Inset1(max_r), Color(90, 90, 100));
			w.DrawText(max_r.left + 5, max_r.top + 3, f->IsMaximized() ? "o" : "+", StdFont(),
			           White());
			w.DrawRect(Inset1(min_r), Color(90, 90, 100));
			w.DrawText(min_r.left + 5, min_r.top + 8, "_", StdFont(), White());

			Rect client = FrameClientLocal(*f);
			if(w.Clipoff(client)) {
				bool active = scope.IsActiveHandle(id);
				const WindowSpec& spec = specs.Get(id);
				PaintSyntheticContent(w, client.GetSize(), spec, active);
				LogPaint("opengl", id, f->GetTitle(), client);
				w.End();
			}

			w.End();
		}

		// taskbar (also serves to un-minimize windows, mirroring the software backend)
		Rect bar(0, sz.cy - TASKBAR_H, sz.cx, sz.cy);
		w.DrawRect(bar, Color(30, 32, 38));
		int x = 4;
		for(int i = 0; i < specs.GetCount(); i++) {
			int id = specs.GetKey(i);
			bool active = scope.IsActiveHandle(id);
			w.DrawRect(RectC(x, bar.top + 2, 120, TASKBAR_H - 4),
			           active ? Color(90, 110, 140) : Color(60, 62, 70));
			w.DrawText(x + 6, bar.top + 6, specs[i].title, StdFont(), White());
			x += 124;
		}

		w.End();
	}

	void LeftDown(Point p, dword keyflags) override
	{
		Size sz = GetSize();
		if(p.y >= sz.cy - TASKBAR_H) {
			int x = 4;
			for(int i = 0; i < specs.GetCount(); i++) {
				Rect br(x, sz.cy - TASKBAR_H + 2, x + 120, sz.cy - 2);
				if(br.Contains(p)) {
					RaiseToTop(specs.GetKey(i));
					Refresh();
					return;
				}
				x += 124;
			}
			return;
		}

		for(int idx = zorder.GetCount() - 1; idx >= 0; idx--) {
			int id = zorder[idx];
			WindowManager::Frame *f = FindWindowFrame(scope, id);
			if(!f || !f->IsShown())
				continue;
			Rect r = f->GetFrameBox();
			if(!r.Contains(p))
				continue;

			RaiseToTop(id);

			Point local = p - r.TopLeft();
			Rect m = f->Margins();
			int titlebar_bottom = m.top + GetStdFontCy() + 4;

			if(!f->IsMaximized() && ButtonRect(*f, 1).Contains(local))
				scope.QueueCloseHandle(id);
			else if(ButtonRect(*f, 2).Contains(local)) {
				if(f->IsMaximized())
					scope.RestoreHandle(id);
				else
					scope.MaximizeHandle(id);
			}
			else if(!f->IsMaximized() && ButtonRect(*f, 3).Contains(local))
				scope.MinimizeHandle(id);
			else if(!f->IsMaximized() && local.y < titlebar_bottom) {
				dragging = true;
				drag_id = id;
				drag_start = p;
				drag_start_rect = r;
				SetCapture();
			}
			Refresh();
			return;
		}
	}

	void MouseMove(Point p, dword keyflags) override
	{
		if(dragging && HasCapture()) {
			WindowManager::Frame *f = FindWindowFrame(scope, drag_id);
			if(f) {
				Rect r = drag_start_rect;
				r.Offset(p - drag_start);
				f->SetFrameBox(r);
				Refresh();
			}
		}
	}

	void LeftUp(Point p, dword keyflags) override
	{
		if(dragging) {
			dragging = false;
			ReleaseCapture();
		}
	}
};

GUI_APP_MAIN
{
	ParseVerboseFlag();
	if(g_verbose)
		fprintf(stderr, "[opengl] DisplayServer starting, verbose logging enabled\n");

	Ctrl::GlobalBackPaint();

	TopWindow win;
	win.Title("DisplayServer -- OpenGL backend");
	win.Sizeable().Zoomable();
	win.SetRect(0, 0, 1000, 700);

	GLDesktop gl;
	gl.host = &win;
	gl.scope.SetFrameBox(RectC(0, 0, 1000, 700));
	win.Add(gl.SizePos());

	for(const WindowSpec& spec : DefaultWindowSpecs())
		gl.AddWindow(spec);

	win.Open();
	win.Run();
}

#endif
