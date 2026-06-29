#include "MainWindow.h"

static const char* kTabName1   = "Tab 1";
static const char* kTabName2   = "Tab 2";
static const char* kTabNameDbg = "Debug";

// ---------------------------------------------------------------------------
// Constructor

MainWindow::MainWindow()
{
	Title("DockingTemplate2").Sizeable().Zoomable();

	AddFrame(menu_);
	menu_.Set(THISBACK(MainMenu));

	AddFrame(toolbar_);
	toolbar_.Set(THISBACK(UpdateToolBar));

	tab1_label_.SetLabel("Tab 1 content — place your editor or view here");
	tab2_label_.SetLabel("Tab 2 content — place your editor or view here");
	tab1_area_.Add(tab1_label_.HSizePos(8, 8).VCenterPos(24));
	tab2_area_.Add(tab2_label_.HSizePos(8, 8).VCenterPos(24));

	main_tabs_.Add(tab1_area_.SizePos(), kTabName1);
	main_tabs_.Add(tab2_area_.SizePos(), kTabName2);
	main_tabs_.Add(debug_tab_.SizePos(), kTabNameDbg);
	main_tabs_.WhenSet = [=] { OnMainTabChanged(); };

	Add(main_tabs_.SizePos());

	mode_.WhenModeChanged << [=](int m) {
		toolbar_.Set(THISBACK(UpdateToolBar));
		Log(Format("mode: %s", m == ModeManager::MODE_EDIT ? "Edit" : "View"));
	};
}

// ---------------------------------------------------------------------------
// DockInit — four-step nide pattern

void MainWindow::DockInit()
{
	// Wire log view first so all DockInit events appear in the Debug tab
	log_.WhenRecord = [=](const AppLogRecord& r) { debug_tab_.AddRecord(r); };

	InitDockers();            // 1. idempotent registration + cursor wiring
	OnResetDockLayout();      // 2. close all + apply C++ default positions
	CacheDefaultLayout();     // 3. snapshot default in memory
	InitRegistry();           // 4a. init AppRegistry (attaches log_), load registry file
	LoadAppState();           // 4b. restore active tab and mode
	if(!LoadUserLayout())     // 4c. try to load saved dock layout blob
		Log("layout: using C++ default");
	loaded_ = true;

	Log("registry config: " + registry_.GetConfigDir());
	Log("registry state:  " + registry_.GetStateDir());
}

// ---------------------------------------------------------------------------
// Close

void MainWindow::Close()
{
	if(loaded_) {
		SaveUserLayout();
		SaveAppState();
		registry_.Save();
		Log("registry: saved");
	}
	TopWindow::Close();
}

// ---------------------------------------------------------------------------
// Docker registration (idempotent — DockViewX constructors set up their own content)

void MainWindow::InitDockers()
{
	dock_a_.Title("Dock A").SizeHint(Size(280, 220));
	Register(dock_a_);

	dock_b_.Title("Dock B").SizeHint(Size(280, 220));
	Register(dock_b_);

	dock_c_.Title("Dock C").SizeHint(Size(360, 220));
	Register(dock_c_);

	// Cursor fan-out: mode fires → all views receive (no MainWindow backref needed)
	mode_.WhenCursorChanged << [=](Point p) { dock_a_.OnCursorChanged(p); };
	mode_.WhenCursorChanged << [=](Point p) { dock_b_.OnCursorChanged(p); };
	mode_.WhenCursorChanged << [=](Point p) { dock_c_.OnCursorChanged(p); };

	// Cursor emit: views fire → mode fans out
	dock_a_.WhenCursorEmit = [=](Point p) { mode_.FireCursor(p); };
	dock_b_.WhenCursorEmit = [=](Point p) { mode_.FireCursor(p); };
	dock_c_.WhenCursorEmit = [=](Point p) { mode_.FireCursor(p); };

	// Keep emit_cursor flag on views in sync with mode
	mode_.WhenModeChanged << [=](int m) {
		bool edit = (m == ModeManager::MODE_EDIT);
		dock_a_.emit_cursor = edit;
		dock_b_.emit_cursor = edit;
		dock_c_.emit_cursor = edit;
	};
}

// ---------------------------------------------------------------------------
// Layout reset

void MainWindow::OnResetDockLayout()
{
	for(DockableCtrl* dc : GetDockableCtrls())
		DockWindow::Close(*dc);

	DockLeft(dock_a_);
	DockRight(dock_b_);
	DockBottom(dock_c_);

	Log("layout: reset to C++ default");
}

void MainWindow::CacheDefaultLayout()
{
	StringStream out;
	SerializeWindow(out);
	default_layout_data_ = out.GetResult();
}

void MainWindow::SetDefaultLayout()
{
	if(default_layout_data_.IsEmpty()) return;
	for(DockableCtrl* dc : GetDockableCtrls())
		DockWindow::Close(*dc);
	StringStream in(default_layout_data_);
	SerializeWindow(in);
}

void MainWindow::SetDockVisible(DockableCtrl& dc, bool visible)
{
	if(!IsOpen()) { dc.Show(visible); return; }
	if(visible)  { if(dc.IsHidden()) RestoreDockerPos(dc, false); }
	else         { if(!dc.IsHidden()) DockWindow::Close(dc); }
}

// ---------------------------------------------------------------------------
// AppRegistry integration

static String sTabLayoutKey(int tab)
{
	switch(tab) {
	case 1:  return "dock.layout.tab2";
	case 2:  return "dock.layout.debug";
	default: return "dock.layout.tab1";
	}
}

void MainWindow::InitRegistry()
{
	registry_.Vendor("AiUpp").AppId("DockingTemplate2").Profile("default").SetLog(&log_);
	registry_.Load();
	Log("registry: loaded");
}

bool MainWindow::LoadUserLayout()
{
	String data;
	if(!registry_.LoadBlob(sTabLayoutKey(current_tab_), data)) {
		Log(Format("layout: no saved layout for tab %d", current_tab_));
		return false;
	}
	StringStream in(data);
	SerializeWindow(in);
	Log(Format("layout loaded: tab %d (%d bytes)", current_tab_, data.GetCount()));
	return true;
}

void MainWindow::SaveUserLayout()
{
	if(!IsOpen()) return;
	StringStream out;
	SerializeWindow(out);
	String data = out.GetResult();
	registry_.SaveBlob(sTabLayoutKey(current_tab_), data);
	Log(Format("layout saved: tab %d (%d bytes)", current_tab_, data.GetCount()));
}

void MainWindow::LoadAppState()
{
	AppState st;
	if(!registry_.LoadJson("app.state", st)) {
		Log("app state: not found, using defaults");
		return;
	}
	// Restore active tab: stable name first, index fallback
	int tab = -1;
	for(int i = 0; i < main_tabs_.GetCount(); i++) {
		if(main_tabs_.GetItem(i).GetText() == st.active_tab_name) {
			tab = i;
			break;
		}
	}
	if(tab < 0 && st.active_tab_index >= 0 && st.active_tab_index < main_tabs_.GetCount())
		tab = st.active_tab_index;
	if(tab >= 0) {
		main_tabs_.Set(tab);
		current_tab_ = tab;
		Log(Format("app state: tab '%s' (index %d)", st.active_tab_name, tab));
	} else {
		Log("app state: tab not found, using default");
	}
	if(st.editor_mode == ModeManager::MODE_EDIT)
		mode_.SetMode(ModeManager::MODE_EDIT);
	Log(Format("app state: mode: %s",
	           st.editor_mode == ModeManager::MODE_EDIT ? "Edit" : "View"));
}

void MainWindow::SaveAppState()
{
	AppState st;
	int tab = main_tabs_.Get();
	st.active_tab_name  = main_tabs_.GetItem(tab).GetText();
	st.active_tab_index = tab;
	st.editor_mode      = (int)mode_.GetMode();
	registry_.SaveJson("app.state", st);
	Log("app state: saved");
}

// ---------------------------------------------------------------------------
// Toolbar

void MainWindow::UpdateToolBar(Bar& bar)
{
	int tab = main_tabs_.Get();
	if(tab == 0)      ToolBarTab1(bar);
	else if(tab == 1) ToolBarTab2(bar);
	else              ToolBarTabDebug(bar);
}

void MainWindow::OnMainTabChanged()
{
	int next = main_tabs_.Get();
	if(!loaded_ || next == current_tab_) return;

	SaveUserLayout();
	Log(Format("tab: %d → %d", current_tab_, next));
	current_tab_ = next;

	if(!LoadUserLayout())
		SetDefaultLayout();
	toolbar_.Set(THISBACK(UpdateToolBar));
	Log(Format("toolbar: tab %d refreshed", next));
}

// ---------------------------------------------------------------------------
// Layout dialog helpers

void MainWindow::OnSaveLayoutAs()
{
	String name;
	if(EditText(name, "Save Layout", "Name:") && !name.IsEmpty())
		SaveLayout(name);
}

void MainWindow::OnLoadLayout()
{
	DockManager();
}
