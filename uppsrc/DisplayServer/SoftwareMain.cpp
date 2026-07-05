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

	DesktopWindow()
	{
		Title("DisplayServer -- software backend");
		Sizeable().Zoomable();
		SetRect(0, 0, 1000, 700);
		Add(scope.GetDesktop());
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
	if(g_verbose)
		fprintf(stderr, "[software] DisplayServer starting, verbose logging enabled\n");

	Ctrl::GlobalBackPaint();

	DesktopWindow win;
	for(const WindowSpec& spec : DefaultWindowSpecs())
		win.AddWindow(spec);

	win.Open();
	win.Run();
}

#endif
