#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>

using namespace Upp;

class DockingTemplate1Window : public DockWindow {
public:
	typedef DockingTemplate1Window CLASSNAME;

	DockingTemplate1Window() {
		Title("DockingTemplate1");
		Sizeable().Zoomable();

		AddFrame(menu);
		menu.Set(THISBACK(MainMenu));

		tab1_area.Add(tab1_label.HSizePos(8, 8).VCenterPos(24));
		tab2_area.Add(tab2_label.HSizePos(8, 8).VCenterPos(24));
		tab3_area.Add(tab3_label.HSizePos(8, 8).VCenterPos(24));
		tab1_label.SetLabel("Theme Editor tab content placeholder");
		tab2_label.SetLabel("Tab 2 content placeholder");
		tab3_label.SetLabel("Tab 3 content placeholder");

		main_tabs.Add(tab1_area.SizePos(), "Tab 1");
		main_tabs.Add(tab2_area.SizePos(), "Tab 2");
		main_tabs.Add(tab3_area.SizePos(), "Tab 3");
		Add(main_tabs.SizePos());

		dock_a_view.Add(dock_a_label.HSizePos(8, 8).VCenterPos(24));
		dock_b_view.Add(dock_b_label.HSizePos(8, 8).VCenterPos(24));
		dock_c_view.Add(dock_c_label.HSizePos(8, 8).VCenterPos(24));
		dock_a_label.SetLabel("Dock A");
		dock_b_label.SetLabel("Dock B");
		dock_c_label.SetLabel("Dock C");

		main_tabs.WhenSet = [=] { OnMainTabChanged(); };
	}

	virtual void DockInit() override {
		dock_a = &Dockable(dock_a_view, "Dock A").SizeHint(Size(280, 220));
		dock_b = &Dockable(dock_b_view, "Dock B").SizeHint(Size(280, 220));
		dock_c = &Dockable(dock_c_view, "Dock C").SizeHint(Size(360, 220));
		Register(*dock_a);
		Register(*dock_b);
		Register(*dock_c);

		int tab = main_tabs.Get();
		if (!LoadTabLayout(tab))
			ApplyDefaultDockSetForTab(tab);
		loaded = true;
	}

	virtual void Close() override {
		if (loaded)
			SaveTabLayout(main_tabs.Get());
		TopWindow::Close();
	}

private:
	MenuBar menu;
	TabCtrl main_tabs;

	ParentCtrl tab1_area;
	ParentCtrl tab2_area;
	ParentCtrl tab3_area;
	Label tab1_label;
	Label tab2_label;
	Label tab3_label;

	ParentCtrl dock_a_view;
	ParentCtrl dock_b_view;
	ParentCtrl dock_c_view;
	Label dock_a_label;
	Label dock_b_label;
	Label dock_c_label;

	DockableCtrl* dock_a = nullptr;
	DockableCtrl* dock_b = nullptr;
	DockableCtrl* dock_c = nullptr;

	bool loaded = false;
	int current_tab = 0;

	void MainMenu(Bar& bar) {
		bar.Add("File", THISBACK(MenuFile));
		bar.Sub("Windows", [=](Bar& sub) { DockWindowMenu(sub); });
		bar.Add("View", THISBACK(MenuView));
		bar.Add("Help", THISBACK(MenuHelp));
	}

	void MenuFile(Bar& bar) {
		bar.Add("New", [=] { PromptOK("New not implemented"); });
		bar.Add("Open", [=] { PromptOK("Open not implemented"); });
		bar.Add("Save", [=] { PromptOK("Save not implemented"); });
		bar.Add("Save As", [=] { PromptOK("Save As not implemented"); });
		bar.Sub("Recent", [=](Bar& sub) { sub.Add("No recent projects", [] {}); });
		bar.Separator();
		bar.Add("Preferences", [=] { PromptOK("Preferences not implemented"); });
		bar.Separator();
		bar.Add("Exit", [=] { Break(); });
	}

	void MenuView(Bar& bar) {
		bar.Add("Reset Layout For Current Tab", [=] {
			ApplyDefaultDockSetForTab(main_tabs.Get());
		});
	}

	void MenuHelp(Bar& bar) {
		bar.Add("About", [=] { PromptOK("DockingTemplate1 minimal frame"); });
	}

	String LayoutFileForTab(int tab) const {
		switch (tab) {
		case 1: return "dockingtemplate1_layout_tab2.dat";
		case 2: return "dockingtemplate1_layout_tab3.dat";
		default: return "dockingtemplate1_layout_tab1.dat";
		}
	}

	void SaveTabLayout(int tab) {
		if (!IsOpen())
			return;
		FileOut out(GetDataFile(LayoutFileForTab(tab)));
		if (out.IsOpen())
			SerializeWindow(out);
	}

	bool LoadTabLayout(int tab) {
		if (!IsOpen())
			return false;
		FileIn in(GetDataFile(LayoutFileForTab(tab)));
		if (!in.IsOpen() || in.IsError())
			return false;
		SerializeWindow(in);
		return !in.IsError();
	}

	void SetDockVisible(DockableCtrl& dc, bool visible) {
		if (!IsOpen()) {
			dc.Show(visible);
			return;
		}
		if (visible) {
			if (dc.IsHidden())
				RestoreDockerPos(dc, false);
		}
		else if (!dc.IsHidden()) {
			DockWindow::Close(dc);
		}
	}

	void ApplyDefaultDockSetForTab(int tab) {
		DockLeft(*dock_a);
		DockRight(*dock_b);
		DockBottom(*dock_c);
		if (tab == 0) {
			SetDockVisible(*dock_a, true);
			SetDockVisible(*dock_b, true);
			SetDockVisible(*dock_c, true);
		}
		else if (tab == 1) {
			SetDockVisible(*dock_a, true);
			SetDockVisible(*dock_b, false);
			SetDockVisible(*dock_c, true);
		}
		else {
			SetDockVisible(*dock_a, false);
			SetDockVisible(*dock_b, true);
			SetDockVisible(*dock_c, false);
		}
	}

	void OnMainTabChanged() {
		int next = main_tabs.Get();
		if (!loaded || next == current_tab)
			return;
		SaveTabLayout(current_tab);
		if (!LoadTabLayout(next))
			ApplyDefaultDockSetForTab(next);
		current_tab = next;
	}
};

GUI_APP_MAIN {
	DockingTemplate1Window().Run();
}

