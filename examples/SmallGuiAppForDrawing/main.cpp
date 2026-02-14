#include <CtrlLib/CtrlLib.h>
#include <AI/LogicGui/LogicGui.h>

using namespace Upp;

struct Stroke : Moveable<Stroke> {
	Vector<Point> points;
	Color color;
	int width;
};

class DrawingCanvas : public Ctrl {
	Vector<Stroke> strokes;
	Stroke current_stroke;
	bool drawing = false;
	Color current_color = Black();

public:
	DrawingCanvas() {
		SetFrame(ViewFrame());
		BackPaint();
	}

	void SetColor(Color c) { current_color = c; }
	void Clear() { strokes.Clear(); Refresh(); }

	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, White());
		
		for(const auto& s : strokes) {
			if(s.points.GetCount() > 1) {
				for(int i = 0; i < s.points.GetCount() - 1; i++)
					w.DrawLine(s.points[i], s.points[i+1], s.width, s.color);
			}
		}
		
		if(drawing && current_stroke.points.GetCount() > 1) {
			for(int i = 0; i < current_stroke.points.GetCount() - 1; i++)
				w.DrawLine(current_stroke.points[i], current_stroke.points[i+1], 
				           current_stroke.width, current_stroke.color);
		}
	}

	virtual void LeftDown(Point p, dword) override {
		drawing = true;
		current_stroke.points.Clear();
		current_stroke.points.Add(p);
		current_stroke.color = current_color;
		current_stroke.width = 2;
		SetCapture();
	}

	virtual void MouseMove(Point p, dword) override {
		if(drawing) {
			current_stroke.points.Add(p);
			Refresh();
		}
	}

	virtual void LeftUp(Point p, dword) override {
		if(drawing) {
			current_stroke.points.Add(p);
			strokes.Add(pick(current_stroke));
			drawing = false;
			ReleaseCapture();
			Refresh();
		}
	}
	
	virtual bool Access(Visitor& v) override {
		v.AccessLabel("drawingCanvas");
		return true;
	}
};

class SmallGuiAppForDrawing : public TopWindow {
	ToolBar toolbar;
	DrawingCanvas canvas;
	ColorPusher color_pusher;
	TimeCallback tc;
	
	typedef SmallGuiAppForDrawing CLASSNAME;

public:
	SmallGuiAppForDrawing() {
		Title("DrawingApp").Sizeable().Zoomable();
		
		AddFrame(toolbar);
		Add(canvas.SizePos());
		
		toolbar.Set([this](Bar& bar) {
			bar.Add("pencilBtn", CtrlImg::save(), [this] { /* pencil tool active */ })
			   .Key(K_P).Help("Pencil tool");
			bar.Add("saveButton", CtrlImg::save(), [this] { SaveImage(); })
			   .Key(K_CTRL_S).Help("Save image");
			bar.Separator();
			bar.Add(color_pusher.SizePos(), 40);
		});
		
		color_pusher << [this] { canvas.SetColor(color_pusher.GetData()); };
		color_pusher.SetData(Black());
		
		tc.Set(1000, [this] {
			const Vector<String>& cmd = CommandLine();
			bool test = false;
			for(int i = 0; i < cmd.GetCount(); i++)
				if(cmd[i] == "--test") test = true;
			
			if(test) {
				RLOG("RUNNING AUTOMATED CONSTRAINT CHECK");
				Ctrl::CheckConstraints();
				Break(); 
			}
		});
	}
	
	void SaveImage() {
		Size sz = canvas.GetSize();
		ImageDraw iw(sz);
		canvas.Paint(iw);
		Image img = iw;
		
		FileSel fs;
		fs.Type("PNG Image", "*.png");
		if(fs.ExecuteSaveAs("Save Drawing")) {
			PNGEncoder().SaveFile(fs.Get(), img);
			PromptOK("Image saved to " + AsString(fs.Get()));
		}
	}
	
	virtual bool Access(Visitor& v) override {
		v.AccessLabel("mainWindow");
		v.AccessAction("colorBtn", [this]{ color_pusher.WhenAction(); });
		return true;
	}
};

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
		RLOG("Loading constraints from: " << constr_dir);
		FindFile ff(AppendFileName(constr_dir, "*.ugui"));
		while(ff) {
			RLOG("  Found file: " << ff.GetPath());
			String c = LoadFile(ff.GetPath());
			Vector<String> lines = Split(c, '\n');
			for(const String& l : lines) {
				String s = TrimLeft(TrimRight(l));
				if(s.GetCount() && !s.StartsWith("YOLO") && !s.StartsWith("Loaded") && !s.StartsWith("Hook")) {
					RLOG("    Adding constraint: " << s);
					Ctrl::constraints.Add(s);
				}
			}
			ff.Next();
		}
	}
	else {
		RLOG("Maestro project root NOT found!");
	}

	RLOG("Starting app with " << Ctrl::constraints.GetCount() << " constraints.");
	SmallGuiAppForDrawing().Run();
}
