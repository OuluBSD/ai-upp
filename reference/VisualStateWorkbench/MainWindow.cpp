#include "MainWindow.h"

static const char* kTabNameFrame   = "Frame";
static const char* kTabNameRegions = "Regions";
static const char* kTabNameDebug   = "Debug";

// ---------------------------------------------------------------------------
// Constructor

MainWindow::MainWindow()
{
	Title("VisualStateWorkbench").Sizeable().Zoomable();

	AddFrame(menu_);
	menu_.Set(THISBACK(MainMenu));

	AddFrame(toolbar_);
	toolbar_.Set(THISBACK(UpdateToolBar));

	// Frame tab
	main_tabs_.Add(frame_canvas_.SizePos(), kTabNameFrame);

	// Regions tab
	regions_list_.AddColumn("ID", 120);
	regions_list_.AddColumn("Frame", 50);
	regions_list_.AddColumn("Action", 80);
	regions_list_.AddColumn("Rect");
	regions_list_.WhenSel = [=] { OnRegionListSel(); };
	regions_area_.Add(regions_list_.SizePos());
	main_tabs_.Add(regions_area_.SizePos(), kTabNameRegions);

	// Debug tab
	main_tabs_.Add(debug_tab_.SizePos(), kTabNameDebug);

	main_tabs_.WhenSet = [=] { OnMainTabChanged(); };
	Add(main_tabs_.SizePos());

	// Frame canvas: region selection
	frame_canvas_.WhenRegionSelected = [=](const String& id) {
		OnRegionSelected(id);
	};
}

// ---------------------------------------------------------------------------
// DockInit

void MainWindow::DockInit()
{
	log_.WhenRecord = [=](const AppLogRecord& r) { debug_tab_.AddRecord(r); };

	InitDockers();
	OnResetDockLayout();
	CacheDefaultLayout();
	InitRegistry();
	LoadAppState();
	if(!LoadUserLayout())
		Log("layout: using default");
	loaded_ = true;

	replay_.SetLog(&log_);
	LoadSampleSession();

	Log("registry config: " + registry_.GetConfigDir());
	LoadSampleAnnotation();
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
// Docker registration

void MainWindow::InitDockers()
{
	props_dock_.Title("Region Properties").SizeHint(Size(260, 200));
	Register(props_dock_);

	timeline_dock_.Title("Replay Timeline").SizeHint(Size(400, 64));
	Register(timeline_dock_);

	session_dock_.Title("Session Info").SizeHint(Size(260, 160));
	Register(session_dock_);

	annotation_dock_.Title("Annotation Editor").SizeHint(Size(300, 280));
	Register(annotation_dock_);
	annotation_dock_.WhenLayerChanged = [=] { OnAnnotationChanged(); };

	timeline_dock_.WhenStep   = [=] { OnStep(); };
	timeline_dock_.WhenRunAll = [=] { OnRunAll(); };
	timeline_dock_.WhenReset  = [=] { OnResetReplay(); };
}

void MainWindow::OnResetDockLayout()
{
	for(DockableCtrl* dc : GetDockableCtrls())
		DockWindow::Close(*dc);

	DockRight(props_dock_);
	DockRight(session_dock_);
	DockLeft(annotation_dock_);
	DockBottom(timeline_dock_);
	Log("layout: reset to default");
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

// ---------------------------------------------------------------------------
// AppRegistry integration

void MainWindow::InitRegistry()
{
	registry_.Vendor("AiUpp").AppId("VisualStateWorkbench").Profile("default").SetLog(&log_);
	registry_.Load();
	Log("registry: loaded");
}

bool MainWindow::LoadUserLayout()
{
	String data;
	if(!registry_.LoadBlob("dock.layout", data)) {
		Log("layout: no saved layout");
		return false;
	}
	StringStream in(data);
	SerializeWindow(in);
	Log(Format("layout loaded (%d bytes)", data.GetCount()));
	return true;
}

void MainWindow::SaveUserLayout()
{
	if(!IsOpen()) return;
	StringStream out;
	SerializeWindow(out);
	String data = out.GetResult();
	registry_.SaveBlob("dock.layout", data);
	Log(Format("layout saved (%d bytes)", data.GetCount()));
}

void MainWindow::LoadAppState()
{
	AppState st;
	if(!registry_.LoadJson("app.state", st)) {
		Log("app state: not found, using defaults");
		return;
	}
	int tab = -1;
	for(int i = 0; i < main_tabs_.GetCount(); i++) {
		if(main_tabs_.GetItem(i).GetText() == st.active_tab_name) {
			tab = i; break;
		}
	}
	if(tab < 0 && st.active_tab_index >= 0 && st.active_tab_index < main_tabs_.GetCount())
		tab = st.active_tab_index;
	if(tab >= 0) {
		main_tabs_.Set(tab);
		current_tab_ = tab;
	}
	Log(Format("app state: tab '%s' loaded", st.active_tab_name));
}

void MainWindow::SaveAppState()
{
	AppState st;
	int tab = main_tabs_.Get();
	st.active_tab_name  = main_tabs_.GetItem(tab).GetText();
	st.active_tab_index = tab;
	registry_.SaveJson("app.state", st);
	Log("app state: saved");
}

// ---------------------------------------------------------------------------
// Session handling

void MainWindow::LoadSampleSession()
{
	Log("session: loading sample…");
	String json = VsmMakeSampleJson();
	String tmp  = AppendFileName(GetTempPath(), "vsm_wb_sample.json");
	SaveFile(tmp, json);
	if(replay_.Load(tmp)) {
		FileDelete(tmp);
		const VsmSession& s = replay_.GetSession();
		frame_canvas_.SetSession(&s);
		RebuildRegionsList();
		timeline_dock_.SetProgress(0, replay_.GetTotalEvents());
		Log(Format("session: loaded '%s' — %d events", s.session_id,
		           replay_.GetTotalEvents()));
	} else {
		FileDelete(tmp);
		Log("session: failed to load sample");
	}

	// Create a sample session store to show manifest
	String store_root = AppendFileName(GetTempPath(), "vsm_wb_store");
	session_store_.SetLog(&log_);
	if(!session_store_.IsOpen()) {
		if(DirectoryExists(store_root))
			session_store_.Open(store_root);
		else
			session_store_.Create(store_root, "wb-sample-001", 320, 240, "synthetic");
		if(session_store_.IsOpen())
			session_dock_.SetManifest(session_store_.GetManifest());
	}
}

void MainWindow::OnStep()
{
	if(!replay_.CanStep()) {
		Log("replay: already at end");
		return;
	}
	replay_.Step();
	RefreshAfterStep();
}

void MainWindow::OnRunAll()
{
	replay_.RunAll();
	RefreshAfterStep();
	Log("replay: run all complete");
}

void MainWindow::OnResetReplay()
{
	LoadSampleSession();
}

void MainWindow::RefreshAfterStep()
{
	int pos   = replay_.GetEventPosition();
	int total = replay_.GetTotalEvents();
	timeline_dock_.SetProgress(pos, total);

	const VsmSession& s = replay_.GetSession();
	// Collect all changed regions seen so far
	Vector<VsmChangedRect> visible;
	for(const VsmChangeEvent& ce : s.changes)
		for(const VsmChangedRect& r : ce.regions)
			visible.Add(r);
	frame_canvas_.SetChangedRegions(visible);
}

// ---------------------------------------------------------------------------
// Toolbar

void MainWindow::UpdateToolBar(Bar& bar)
{
	bar.Add("Step",    CtrlImg::right_arrow(), [=] { OnStep(); });
	bar.Add("Run All", CtrlImg::go_forward(),  [=] { OnRunAll(); });
	bar.Separator();
	bar.Add("Reset",   CtrlImg::undo(),        [=] { OnResetReplay(); });
}

// ---------------------------------------------------------------------------
// Tab change

void MainWindow::OnMainTabChanged()
{
	int next = main_tabs_.Get();
	if(!loaded_ || next == current_tab_) return;
	current_tab_ = next;
	toolbar_.Set(THISBACK(UpdateToolBar));
	Log(Format("tab: switched to %d", next));
}

// ---------------------------------------------------------------------------
// Region selection

void MainWindow::OnRegionSelected(const String& id)
{
	if(id.IsEmpty()) {
		props_dock_.Clear();
		Log("region: selection cleared");
		return;
	}
	// Find region by synthesized id "region-N"
	const VsmSession& s = replay_.GetSession();
	const VsmRegionNode* found = nullptr;
	for(const VsmRegionNode& rn : s.regions) {
		if(rn.id == id) { found = &rn; break; }
	}
	props_dock_.SetRegion(id, found);
	Log(Format("region selected: %s", id));
}

void MainWindow::OnRegionListSel()
{
	int row = regions_list_.GetCursor();
	if(row < 0) return;
	const VsmSession& s = replay_.GetSession();
	if(row >= s.regions.GetCount()) return;
	const VsmRegionNode& rn = s.regions[row];
	props_dock_.SetRegion(rn.id, &rn);
	Log(Format("region list selected: %s", rn.id));
}

// ---------------------------------------------------------------------------
// Annotation

void MainWindow::LoadSampleAnnotation()
{
	annotation_layer_ = VsmAnnotationLayer();
	annotation_layer_.schema     = 1;
	annotation_layer_.session_id = "wb-sample-001";

	// Seed with one annotation based on the sample session region
	VsmRegionAnnotation& a = annotation_layer_.annotations.Add();
	a.id   = "ann-001";
	a.name = "Login Button";
	a.x = 10; a.y = 20; a.w = 80; a.h = 40;
	a.linked_region_ids.Add("rgn-0001");

	// Save to session annotations dir if store is open
	if(session_store_.IsOpen()) {
		annotation_path_ = AppendFileName(
		    session_store_.GetPaths().annotations_dir, "annotations.json");
		// Load existing if present; otherwise save the seed
		if(FileExists(annotation_path_))
			annotation_layer_.Load(annotation_path_);
		else
			annotation_layer_.Save(annotation_path_);
	}

	annotation_dock_.SetLayer(&annotation_layer_);
	Log(Format("annotations: loaded %d annotation(s)", annotation_layer_.annotations.GetCount()));
}

void MainWindow::OnAnnotationChanged()
{
	// Validate
	auto errs = annotation_layer_.Validate();
	for(const auto& e : errs)
		LogWarn(log_, "Annotation", "Validation: " + e.annotation_id + ": " + e.message);
	if(errs.IsEmpty())
		Log("annotations: validation OK");

	// Persist
	if(!annotation_path_.IsEmpty())
		annotation_layer_.Save(annotation_path_);
}

// ---------------------------------------------------------------------------

void MainWindow::RebuildRegionsList()
{
	regions_list_.Clear();
	for(const VsmRegionNode& rn : replay_.GetSession().regions) {
		String rect = Format("(%d,%d) ", rn.x, rn.y) + IntStr(rn.w) + "x" + IntStr(rn.h);
		regions_list_.Add(rn.id, rn.frame,
		                  rn.action.IsEmpty() ? "—" : rn.action,
		                  rect);
	}
}
