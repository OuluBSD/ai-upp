// This is Upp conversion of Qt example, see
// http://doc.trolltech.com/3.0/hello-example.html

#include <CtrlLib/CtrlLib.h>
#include <AI/LogicGui/LogicGui.h>

using namespace Upp;

class HelloWorld : public TopWindow {
public:
	virtual void LeftDown(Point, dword) override;
	virtual void Paint(Draw& w) override;
	
	virtual bool Access(Visitor& v) override {
		v.AccessLabel("mainWindow");
		v.AccessLabel("helloText");
		return true;
	}

private:
	String text;
	
public:
	HelloWorld& Text(const String& t)     { text = t; Refresh(); return *this; }

	HelloWorld();
};

HelloWorld::HelloWorld()
{
	LayoutId("mainWindow");
	SetTimeCallback(-40, [=] { Refresh(); });
	BackPaint();
	Zoomable().Sizeable();
	SetRect(0, 0, 1000, 300);
}

void HelloWorld::LeftDown(Point, dword)
{
	EditText(text, "Text to display", "Text");
}

void HelloWorld::Paint(Draw& w)
{
	Font fnt = Roman(64);
	Size sz = GetSize();
	w.DrawRect(sz, White);
	Size tsz = GetTextSize(text, fnt);
	Point pos = (sz - tsz) / 2;
	for(int i = 0; i < text.GetLength(); i++) {
		int q = (i + GetTickCount() / 40) & 15;
		w.DrawText(pos.x, pos.y + sin(GetTickCount() / 100.0 + i / 2.0) * (sz.cy - fnt.GetCy()) / 4,
		           ~text + i, fnt, HsvColorf(q / 15.0, 1, 0.5), 1);
		pos.x += fnt[text[i]];
	}
}

GUI_APP_MAIN
{
	LinkLogicGui();
	
	String root = GetCurrentDirectory();
	bool found = false;
	while(!root.IsEmpty()) {
		if(DirectoryExists(AppendFileName(root, "docs/maestro"))) {
			found = true;
			break;
		}
		String next = GetFileFolder(root);
		if(next == root) break;
		root = next;
	}
	
	if(found) {
		String constr_dir = AppendFileName(root, "docs/maestro/plans/constraints");
		FindFile ff(AppendFileName(constr_dir, "*.ugui"));
		while(ff) {
			String c = LoadFile(ff.GetPath());
			Vector<String> lines = Split(c, '\n');
			for(const String& l : lines) {
				String s = TrimLeft(TrimRight(l));
				if(s.GetCount() && !s.StartsWith("YOLO") && !s.StartsWith("Loaded") && !s.StartsWith("Hook")) {
					Ctrl::constraints.Add(s);
				}
			}
			ff.Next();
		}
	}

	HelloWorld hw;
	hw.Title("Hello world example");
	hw.Text(Nvl(Join(CommandLine(), " "), "Hello world !"));
	
	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++) {
		if(cmd[i] == "--test") {
			Ctrl::CheckConstraints();
			return;
		}
	}

	hw.Run();
}
