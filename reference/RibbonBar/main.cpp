#include <CtrlLib/CtrlLib.h>

using namespace Upp;

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

		AddFrame(ribbon);
		Add(content.SizePos());

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
		show_picture.WhenAction = [=] { ToggleContext(); };

		show_list_labels.SetLabel("Show list labels");
		show_list_labels = true;
		show_list_labels.WhenAction = [=, &edit] { edit.ShowListText(show_list_labels); };

		display.Add(RibbonBar::RIBBON_ALWAYS, "Always show ribbon");
		display.Add(RibbonBar::RIBBON_TABS, "Show tabs");
		display.Add(RibbonBar::RIBBON_AUTOHIDE, "Auto-hide");
		display.SetIndex(0);
		display.WhenAction = [=] { OnDisplay(); };

		info.SetText("Ctrl+F1 toggles ribbon, Alt toggles key tips.");

		content.Add(show_picture.LeftPosZ(10, 240).TopPosZ(10, 20));
		content.Add(show_list_labels.LeftPosZ(10, 240).TopPosZ(40, 20));
		content.Add(display.LeftPosZ(10, 240).TopPosZ(70, 20));
		content.Add(info.LeftPosZ(10, 400).TopPosZ(100, 20));

		ribbon.SetDisplayMode(RibbonBar::RIBBON_ALWAYS);
		ribbon.ShowContext("picture", false);
	}
};

GUI_APP_MAIN
{
	RibbonBarDemo().Run();
}
