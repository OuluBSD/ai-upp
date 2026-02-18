#include <CtrlLib/CtrlLib.h>
#include <AI/LogicGui/LogicGui.h>

using namespace Upp;

namespace {

static bool LooksLikeFormula(const String& s)
{
	int l = s.Find('(');
	int r = s.ReverseFind(')');
	if(l <= 0 || r <= l)
		return false;
	for(int i = 0; i < l; i++) {
		byte c = (byte)s[i];
		if(!IsAlpha(c) && !IsDigit(c) && c != '_')
			return false;
	}
	return true;
}

static void AddConstraintFormula(const String& raw)
{
	String s = TrimBoth(raw);
	if(s.IsEmpty() || s.StartsWith("#"))
		return;
	if(s.StartsWith("-")) {
		int q0 = s.Find('"');
		int q1 = s.ReverseFind('"');
		if(q0 >= 0 && q1 > q0) {
			String quoted = s.Mid(q0 + 1, q1 - q0 - 1);
			if(LooksLikeFormula(quoted))
				Ctrl::constraints.Add(quoted);
			return;
		}
	}
	if(LooksLikeFormula(s))
		Ctrl::constraints.Add(s);
}

static void LoadMaestroConstraints()
{
	String root = GetCurrentDirectory();
	while(!root.IsEmpty()) {
		if(DirectoryExists(AppendFileName(root, "docs/maestro")))
			break;
		String next = GetFileFolder(root);
		if(next == root) {
			root.Clear();
			break;
		}
		root = next;
	}
	if(root.IsEmpty())
		return;

	String constr_dir = AppendFileName(root, "docs/maestro/plans/constraints");
	if(!DirectoryExists(constr_dir))
		return;

	FindFile ff(AppendFileName(constr_dir, "*.ugui"));
	while(ff) {
		String content = LoadFile(ff.GetPath());
		if(!content.IsEmpty()) {
			Vector<String> lines = Split(content, '\n');
			for(const String& l : lines)
				AddConstraintFormula(l);
		}
		ff.Next();
	}
}

static bool HasArg(const char* key)
{
	const Vector<String>& cmd = CommandLine();
	for(int i = 0; i < cmd.GetCount(); i++)
		if(cmd[i] == key)
			return true;
	return false;
}

}

struct FontRow : ParentCtrl {
	DropList font;
	Button   bold, italic, underline;

	FontRow() {
		font.Add("Arial");
		font.Add("Times New Roman");
		font.Add("Consolas");
		font.SetIndex(0);
		bold.SetLabel("B");
		italic.SetLabel("I");
		underline.SetLabel("U");
		Add(font);
		Add(bold);
		Add(italic);
		Add(underline);
	}

	void Layout() override {
		int h = GetSize().cy;
		font.LeftPos(0, 140).VCenterPos(h);
		bold.LeftPos(150, 28).VCenterPos(h);
		italic.LeftPos(182, 28).VCenterPos(h);
		underline.LeftPos(214, 28).VCenterPos(h);
	}
};

struct NameRow : ParentCtrl {
	EditString name;
	Label      lbl;

	NameRow() {
		lbl.SetText("Label");
		name <<= "Title";
		Add(lbl);
		Add(name);
	}

	void Layout() override {
		int h = GetSize().cy;
		lbl.LeftPos(0, 50).VCenterPos(h);
		name.LeftPos(60, 160).VCenterPos(h);
	}
};

struct PreviewRow : ParentCtrl {
	RichTextView preview;

	PreviewRow() {
		preview.SetQTF("[* Bold] / [@I Italic] / [@U Underline]");
		Add(preview);
	}

	void Layout() override {
		preview.HSizePos(0, 0).VSizePos(0, 0);
	}
};

struct ColumnItemDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const override
	{
		PaintBackground(w, r, q, ink, paper, style);
		String s = q;
		int tcy = GetTextSize(s, StdFont()).cy;
		int c = min(r.Deflated(4).GetSize().cy, r.Deflated(4).GetSize().cx);
		w.DrawImage(r.left + 4, r.top + 4, c, c, CtrlImg::File());
		w.DrawText(r.left + c + 8, r.top + (r.GetHeight() - tcy) / 2, s, StdFont(), ink);
	}
};

struct StylesPanel : ParentCtrl {
	FontRow    fontrow;
	NameRow    namerow;
	PreviewRow previewrow;
	Label      lbl_font;
	Label      lbl_name;
	Label      lbl_prev;

	StylesPanel()
	{
		lbl_font.SetText("Font");
		lbl_name.SetText("Name");
		lbl_prev.SetText("Preview");
		Add(lbl_font);
		Add(lbl_name);
		Add(lbl_prev);
		Add(fontrow);
		Add(namerow);
		Add(previewrow);
	}

	void Layout() override
	{
		int y = 0;
		lbl_font.LeftPos(0, 60).TopPos(y, 22);
		fontrow.LeftPos(70, 260).TopPos(y, 22);
		y += 26;
		lbl_name.LeftPos(0, 60).TopPos(y, 22);
		namerow.LeftPos(70, 260).TopPos(y, 22);
		y += 26;
		lbl_prev.LeftPos(0, 60).TopPos(y, 22);
		previewrow.LeftPos(70, 260).TopPos(y, 48);
	}
};

struct ItemsPanel : ParentCtrl {
	DropList view;
	ColumnList list;
	ColumnItemDisplay display;

	enum ViewMode {
		VIEW_DETAILS,
		VIEW_SMALL,
		VIEW_LARGE,
		VIEW_THUMB,
	};

	void SetMode(int m)
	{
		switch(m) {
		case VIEW_DETAILS:
			list.ListMode().Columns(1).ItemHeight(28).ItemWidth(240);
			break;
		case VIEW_SMALL:
			list.ColumnMode().Columns(3).ItemWidth(80).ItemHeight(48);
			break;
		case VIEW_LARGE:
			list.ColumnMode().Columns(2).ItemWidth(120).ItemHeight(80);
			break;
		case VIEW_THUMB:
			list.ColumnMode().Columns(2).ItemWidth(140).ItemHeight(96);
			break;
		}
		list.RefreshLayout();
		list.Refresh();
	}

	ItemsPanel()
	{
		view.Add(VIEW_DETAILS, "Details");
		view.Add(VIEW_SMALL, "Small icons");
		view.Add(VIEW_LARGE, "Large icons");
		view.Add(VIEW_THUMB, "Thumbnails");
		view.SetIndex(VIEW_SMALL);
		view.WhenAction = [=] { SetMode((int)view.GetData()); };

		list.SetDisplay(display);
		list.NoRoundSize();
		list.Add("Photo");
		list.Add("Vector");
		list.Add("Archive");
		list.Add("Document");
		list.Add("Slides");
		list.Add("Audio");
		list.Add("Project");
		list.Add("Scene");

		Add(view);
		Add(list);
		SetMode(VIEW_SMALL);
	}

	void Layout() override
	{
		view.LeftPos(0, 180).TopPos(0, 22);
		list.HSizePos(0, 0).VSizePos(26, 0);
	}
};

struct RibbonBarDemo : TopWindow {
	RibbonBar  ribbon;
	ParentCtrl content;
	Option     show_picture;
	Option     show_list_labels;
	DropList   display;
	Label      info;
	StylesPanel styles_panel;
	ItemsPanel  items_panel;

	void OnDisplay()
	{
		Value v = display.GetData();
		if(IsNumber(v))
			ribbon.SetDisplayMode((RibbonBar::DisplayMode)(int)v);
	}

	void ToggleContext()
	{
		ribbon.ShowContext("picture", show_picture);
	}

	RibbonBarDemo()
	{
		Title("RibbonBar demo");
		Sizeable().Zoomable();
		LayoutId("mainWindow");

		AddFrame(ribbon);
		Add(content.SizePos());
		ribbon.LayoutId("ribbonBar");
		content.LayoutId("contentArea");

		ribbon.SetQuickAccess([=](Bar& bar) {
			bar.Add(CtrlImg::new_doc(), [] {});
			bar.Add(CtrlImg::open(), [] {});
			bar.Add(CtrlImg::save(), [] {});
		});

		RibbonPage& home = ribbon.AddTab("Home");
		RibbonGroup& clip = home.AddGroup("Clipboard");
		clip.SetLarge([=](Bar& bar) {
			bar.Add("Paste", CtrlImg::paste(), [] {});
			bar.Add("Copy", CtrlImg::copy(), [] {});
			bar.Add("Cut", CtrlImg::cut(), [] {});
		});
		RibbonGroup& edit = home.AddGroup("Editing");
		edit.SetList([=](Bar& bar) {
			bar.Add("Undo", CtrlImg::undo(), [] {});
			bar.Add("Redo", CtrlImg::redo(), [] {});
			bar.Add("Select All", CtrlImg::select_all(), [] {});
		});

		RibbonPage& insert = ribbon.AddTab("Insert");
		RibbonGroup& ill = insert.AddGroup("Illustrations");
		ill.SetLarge([=](Bar& bar) {
			bar.Add("Add Shape", CtrlImg::Plus(), [] {});
			bar.Add("From File", CtrlImg::File(), [] {});
			bar.Add("From Folder", CtrlImg::Dir(), [] {});
		});

		RibbonPage& view = ribbon.AddTab("View");
		RibbonGroup& win = view.AddGroup("Window");
		win.SetList([=](Bar& bar) {
			bar.Add("Window", CtrlImg::menu_window(), [] {});
			bar.Add("Up", CtrlImg::DirUp(), [] {});
			bar.Add("Home", CtrlImg::Home(), [] {});
		});
		RibbonGroup& view_ops = view.AddGroup("Layout");
		view_ops.SetLarge([=](Bar& bar) {
			bar.Add("Tile", CtrlImg::File(), [] {});
		});

		RibbonGroup& styles = view.AddGroup("Styles");
		styles.SetContentMinSize(Size(340, 0));
		styles.SetListCtrl(styles_panel);

		RibbonGroup& icons = insert.AddGroup("Items");
		icons.SetContentMinSize(Size(300, 0));
		icons.SetListCtrl(items_panel);

		RibbonPage& picture = ribbon.AddContextTab("picture", "Picture Tools");
		RibbonGroup& adj = picture.AddGroup("Adjust");
		adj.SetLarge([=](Bar& bar) {
			bar.Add("Share", CtrlImg::Share(), [] {});
			bar.Add("Transparent", CtrlImg::set_transparent(), [] {});
		});

		show_picture.SetLabel("Picture selected (context tab)");
		show_picture.LayoutId("showPicture");
		show_picture.WhenAction = [=] { ToggleContext(); };

		show_list_labels.SetLabel("Show list labels");
		show_list_labels.LayoutId("showListLabels");
		show_list_labels = true;
		show_list_labels.WhenAction = [=, &edit] { edit.ShowListText(show_list_labels); };

		display.Add(RibbonBar::RIBBON_ALWAYS, "Always show ribbon");
		display.Add(RibbonBar::RIBBON_TABS, "Show tabs");
		display.Add(RibbonBar::RIBBON_AUTOHIDE, "Auto-hide");
		display.SetIndex(0);
		display.LayoutId("displayMode");
		display.WhenAction = [=] { OnDisplay(); };

		info.SetText("Ctrl+F1 toggles ribbon, Alt toggles key tips.");
		info.LayoutId("infoLabel");

		content.Add(show_picture.LeftPosZ(10, 240).TopPosZ(10, 20));
		content.Add(show_list_labels.LeftPosZ(10, 240).TopPosZ(40, 20));
		content.Add(display.LeftPosZ(10, 240).TopPosZ(70, 20));
		content.Add(info.LeftPosZ(10, 400).TopPosZ(100, 20));

		ribbon.SetDisplayMode(RibbonBar::RIBBON_ALWAYS);
		ribbon.ShowContext("picture", false);
	}

	bool Access(Visitor& v) override
	{
		v.AccessLabel("mainWindow");
		v.AccessLabel("ribbonBar");
		v.AccessAction("showPicture", [=] { ToggleContext(); });
		v.AccessAction("showListLabels", [=] {});
		v.AccessAction("displayMode", [=] {});
		v.AccessLabel("infoLabel");
		return TopWindow::Access(v);
	}
};

GUI_APP_MAIN
{
	LinkLogicGui();
	LoadMaestroConstraints();

	RibbonBarDemo app;
	if(HasArg("--test")) {
		app.Open();
		for(int i = 0; i < 3; i++) {
			Ctrl::ProcessEvents();
			Ctrl::GuiSleep(20);
		}

		Vector<String> tabs;
		tabs.Add("Home");
		tabs.Add("Insert");
		tabs.Add("View");
		int checks = 0;
		for(const String& tab : tabs) {
			if(app.ribbon.SelectTab(tab)) {
				for(int i = 0; i < 3; i++) {
					Ctrl::ProcessEvents();
					Ctrl::GuiSleep(20);
				}
				Ctrl::CheckConstraints();
				checks++;
			}
		}
		if(checks == 0)
			Ctrl::CheckConstraints();
		return;
	}
	app.Run();
}
