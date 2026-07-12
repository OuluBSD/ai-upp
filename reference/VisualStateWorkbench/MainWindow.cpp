#include "MainWindow.h"

static const char* kTabNameFrame   = "Frame";
static const char* kTabNameRegions = "Regions";
static const char* kTabNameDebug   = "Debug";

// ---------------------------------------------------------------------------
// OpenImportDialog — single File-menu entry point for loading a session.
//
// Replaces the four separate "Open Session…"/"Import Image Sequence…"/
// "Load Sample Session"/"Load E2E Sample Session" menu items with one small
// modal dialog: an explicit source-type chooser plus a directory picker that
// only applies to the two directory-based source types. MainWindow's
// OnOpenImportSession() reads the chosen type/path back out and dispatches
// to the exact same existing calls those four old menu items used
// (OpenSessionPath()/DispatchImageSequenceImport()/LoadSampleSession()/
// OnLoadE2ESample()) -- this dialog changes nothing about what happens once
// a source is chosen, only how the user picks one.
//
// Kept as a plain, file-local (anonymous namespace) TopWindow rather than a
// new pair of .h/.cpp files: it's small, single-purpose, only ever
// constructed from MainWindow::OnOpenImportSession(), and this mirrors the
// existing style of file-local helpers in this same file (HasVsmFiles(),
// HasJpegFiles() below) and in DockPanels.cpp's anonymous-namespace helpers.
// No changes to VisualStateWorkbench.upp are needed as a result.

namespace {

class OpenImportDialog : public TopWindow {
public:
	typedef OpenImportDialog CLASSNAME;

	enum SourceType {
		SESSION_DIR    = 0,
		IMAGE_SEQUENCE = 1,
		SAMPLE         = 2,
		E2E_SAMPLE     = 3,
		TEXAS_SESSION  = 4,
	};

	OpenImportDialog();

	SourceType GetSourceType() const { return (SourceType)(int)type_drop_.GetData(); }
	String     GetPath() const       { return ~path_edit_; }

private:
	Label      type_lbl_, path_lbl_;
	DropList   type_drop_;
	EditString path_edit_;
	Button     browse_btn_;
	Button     open_btn_, cancel_btn_;

	void OnTypeChanged();
	void OnBrowse();
	void OnOpen();
	bool NeedsPath() const;
};

OpenImportDialog::OpenImportDialog()
{
	Title("Open/Import Session").Sizeable(false);
	SetRect(0, 0, 440, 130);

	type_lbl_.SetLabel("Source:");
	type_drop_.Add(SESSION_DIR,    "Existing session directory");
	type_drop_.Add(IMAGE_SEQUENCE, "Image sequence (.vsm/.jpg/.png)");
	type_drop_.Add(TEXAS_SESSION,  "TexasHoldem session (metadata.json + frames/)");
	type_drop_.Add(SAMPLE,         "Built-in sample data");
	type_drop_.Add(E2E_SAMPLE,     "Built-in E2E sample data");
	type_drop_.SetIndex(0);
	type_drop_.WhenAction = THISBACK(OnTypeChanged);

	path_lbl_.SetLabel("Directory:");
	browse_btn_.SetLabel("Browse\xE2\x80\xA6");
	browse_btn_ <<= THISBACK(OnBrowse);

	open_btn_.SetLabel("Open");
	open_btn_ <<= THISBACK(OnOpen);
	cancel_btn_.SetLabel("Cancel");
	Rejector(cancel_btn_, IDCANCEL);

	Add(type_lbl_.LeftPos(8, 100).TopPos(12, 20));
	Add(type_drop_.HSizePos(112, 8).TopPos(12, 20));
	Add(path_lbl_.LeftPos(8, 100).TopPos(40, 20));
	Add(path_edit_.HSizePos(112, 96).TopPos(40, 20));
	Add(browse_btn_.RightPos(8, 80).TopPos(40, 20));
	Add(open_btn_.RightPos(96, 80).BottomPos(10, 24));
	Add(cancel_btn_.RightPos(8, 80).BottomPos(10, 24));

	OnTypeChanged();
}

bool OpenImportDialog::NeedsPath() const
{
	SourceType t = (SourceType)(int)type_drop_.GetData();
	return t == SESSION_DIR || t == IMAGE_SEQUENCE || t == TEXAS_SESSION;
}

void OpenImportDialog::OnTypeChanged()
{
	bool need_path = NeedsPath();
	path_lbl_.Show(need_path);
	path_edit_.Show(need_path);
	browse_btn_.Show(need_path);
}

void OpenImportDialog::OnBrowse()
{
	String dir = SelectDirectory();
	if(!dir.IsEmpty())
		path_edit_.SetData(dir);
}

void OpenImportDialog::OnOpen()
{
	if(NeedsPath() && String(~path_edit_).IsEmpty()) {
		PromptOK("Please choose a directory first.");
		return;
	}
	Break(IDOK);
}

} // anonymous namespace

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
	if(!LoadUserLayout()) {
		Log("layout: using default");
		had_prior_session_state_ = false;
	} else {
		had_prior_session_state_ = true;
	}
	loaded_ = true;
	LoadOverlayState();

	replay_.SetLog(&log_);
	LoadSampleSession();
	RunJpegSmokeTest();

	// Open pipeline cache in temp cache directory
	String cache_dir = AppendFileName(GetTempPath(), "vsm_wb_cache");
	pipeline_cache_.SetLog(&log_);
	pipeline_cache_.Open(cache_dir);

	Log("registry config: " + registry_.GetConfigDir());
	LoadSampleAnnotation();

	// Show empty-state placeholder if no prior session state was restored
	frame_canvas_.SetShowEmptyStatePlaceholder(!had_prior_session_state_);
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

	pipeline_dock_.Title("Pipeline Editor").SizeHint(Size(280, 220));
	Register(pipeline_dock_);

	template_dock_.Title("Template Rules").SizeHint(Size(300, 260));
	Register(template_dock_);

	ocr_dock_.Title("OCR Rules").SizeHint(Size(300, 260));
	Register(ocr_dock_);

	model_dock_.Title("Model State").SizeHint(Size(380, 300));
	Register(model_dock_);
	model_dock_.WhenJumpToFrame = [=](int f) { OnJumpToFrame(f); };

	layout_dock_.Title("Layout Bindings").SizeHint(Size(560, 260));
	Register(layout_dock_);
	layout_dock_.WhenBindingSelected = [=](int ri) { OnLayoutBindingSelected(ri); };

	logic_dock_.Title("Logic State").SizeHint(Size(420, 300));
	Register(logic_dock_);

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

	// Primary LEFT tier: visible by default, one pane each. Defining what to
	// annotate (AnnotationEditorPanel) and seeing the payoff (ModelStatePanel,
	// divergences) are what a new session needs first.
	DockLeft(annotation_dock_);
	DockLeft(model_dock_);

	// Secondary LEFT tier: PipelineEditorPanel/TemplateRulePanel/OcrRulePanel
	// are only reached once a session + annotations already exist, so they
	// share ONE tabbed dock slot ("Rules & Preprocessing") instead of three
	// separate always-visible panes. DockWindow's grouping primitive for this
	// is Tabify(target, dc) -- "attach dc as a tab to target's container,
	// wherever it is" (see uppsrc/Docking/Docking.h) -- the same mechanism
	// reference/DockingTemplate2/INVESTIGATION.md documents adopting from
	// nide's RecognizerEditorWindow::OnResetDockLayout() for exactly this
	// "group several dockables into one tabbed slot" case.
	//
	// DockCont::ChildAdded() (uppsrc/Docking/DockCont.cpp) inserts every newly
	// tabbed control at tab index 0 AND makes it the active/foreground tab, so
	// whichever control is Tabify()'d *last* ends up in front. Calls below are
	// ordered so PipelineEditorPanel -- preprocessing, which logically comes
	// before template/OCR rule matching -- is Tabify()'d last and is the
	// default front tab; TemplateRulePanel and OcrRulePanel are present in the
	// same group (reachable via their tabs) but not in front by default.
	DockLeft(template_dock_);
	Tabify(template_dock_, ocr_dock_);
	Tabify(template_dock_, pipeline_dock_);

	// Layout Bindings (task 0132): a wide per-frame table, docked at the bottom
	// alongside the Replay Timeline it is scrubbed by, tabbed so it does not
	// steal vertical space from the frame canvas until the user reaches for it.
	DockBottom(layout_dock_);
	Tabify(layout_dock_, timeline_dock_);

	// Logic-State Timeline (task 0133): pairs naturally with Layout Bindings
	// (both overlay derived data on the same frame/timeline) — joins the same
	// bottom tabbed group rather than claiming its own always-visible pane.
	Tabify(layout_dock_, logic_dock_);

	Log("layout: reset to default (primary: Annotation Editor/Model State; "
	    "secondary: Rules & Preprocessing tab group; bottom: Layout Bindings/Replay Timeline)");
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

void MainWindow::PushCurrentFrameToPanels()
{
	pipeline_dock_.SetCurrentFrame(current_frame_img_);
	template_dock_.SetCurrentFrame(current_frame_img_);
	ocr_dock_.SetCurrentFrame(current_frame_img_);
}

void MainWindow::LoadSampleSession()
{
	Log("session: loading sample…");

	// Loading the sample always makes the sample replay session (A) the
	// active session, discarding any opened/imported (B) or TexasHoldem (C)
	// session identity.
	active_session_ = ACTIVE_SAMPLE;
	src_step_pos_    = 0;
	ClearTexasSession();
	// The sample session has no real per-frame image bytes at all (only
	// VsmSession regions/changes) -- so there is no real "current frame" to
	// offer the rule/pipeline preview panels while it's active.
	current_frame_img_ = VsmImageBuffer();
	PushCurrentFrameToPanels();

	String json = VsmMakeSampleJson();
	String tmp  = AppendFileName(GetTempPath(), "vsm_wb_sample.json");
	SaveFile(tmp, json);
	if(replay_.Load(tmp)) {
		FileDelete(tmp);
		const VsmSession& s = replay_.GetSession();
		frame_canvas_.SetSession(&s);
		frame_canvas_.SetChangedRegions(Vector<VsmChangedRect>());
		RebuildRegionsList();
		timeline_dock_.SetProgress(0, replay_.GetTotalEvents());
		Log(Format("session: loaded '%s' — %d events", s.session_id,
		           replay_.GetTotalEvents()));
	} else {
		FileDelete(tmp);
		Log("session: failed to load sample");
		PromptOK("Could not load sample session.\nSee Debug tab for details.");
	}

	// (Re-)anchor the session store at the sample root. This is not guarded
	// by "IsOpen()" because session_store_ may currently be pointed at an
	// opened/imported session's directory (B) -- e.g. after OnResetReplay()
	// discards B -- and must be re-pointed at the sample so SessionInfoPanel
	// doesn't keep showing the discarded session's manifest.
	String store_root = AppendFileName(GetTempPath(), "vsm_wb_store");
	session_store_.SetLog(&log_);
	if(DirectoryExists(store_root))
		session_store_.Open(store_root);
	else
		session_store_.Create(store_root, "wb-sample-001", 320, 240, "synthetic");
	if(session_store_.IsOpen())
		session_dock_.SetManifest(session_store_.GetManifest());
}

void MainWindow::OnStep()
{
	// Operate on whichever session is active (active_session_): the TexasHoldem
	// session (C), the opened/imported session (B, src_source_), or the sample
	// replay session (A, replay_).
	if(active_session_ == ACTIVE_TEXAS) {
		if(th_step_pos_ >= th_session_.info.frame_count - 1) {
			Log("texas session: already at last frame");
			return;
		}
		SetTexasFrame(th_step_pos_ + 1);
		return;
	}
	if(active_session_ == ACTIVE_IMPORTED) {
		VsmImageBuffer img;
		int64 ts_ms = 0;
		if(!src_source_.ReadFrame(img, ts_ms)) {
			Log("session: already at end (opened session)");
			return;
		}
		src_step_pos_++;
		current_frame_img_ = pick(img);
		RefreshAfterSourceStep();
		return;
	}
	if(!replay_.CanStep()) {
		Log("replay: already at end");
		return;
	}
	replay_.Step();
	RefreshAfterStep();
}

void MainWindow::OnRunAll()
{
	if(active_session_ == ACTIVE_TEXAS) {
		if(th_session_.info.frame_count > 0)
			SetTexasFrame(th_session_.info.frame_count - 1);
		Log("texas session: run all complete");
		return;
	}
	if(active_session_ == ACTIVE_IMPORTED) {
		VsmImageBuffer img;
		int64 ts_ms = 0;
		bool got_any = false;
		while(src_source_.ReadFrame(img, ts_ms)) {
			src_step_pos_++;
			got_any = true;
		}
		// img now holds the last successfully read frame (ReadFrame leaves
		// its out-param untouched on the final "false" call) -- that's the
		// real current frame after Run All.
		if(got_any)
			current_frame_img_ = pick(img);
		RefreshAfterSourceStep();
		Log("session: run all complete (opened session)");
		return;
	}
	replay_.RunAll();
	RefreshAfterStep();
	Log("replay: run all complete");
}

void MainWindow::OnResetReplay()
{
	// Resetting the sample is only destructive when a real session (B imported
	// or C TexasHoldem) is currently active -- confirm before discarding it.
	if(active_session_ != ACTIVE_SAMPLE) {
		if(!PromptYesNo("This will discard the currently opened session and "
		                "reload the built-in sample. Continue?"))
			return;
		if(active_session_ == ACTIVE_IMPORTED)
			src_source_.Close();
	}
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

void MainWindow::RefreshAfterSourceStep()
{
	// Opened/imported sessions (B) have no VsmSession-shaped region/change
	// data in the headless API (VsmSessionStore/VsmSessionStoreSource only
	// track frames+crops) -- so there is no changed-regions overlay to
	// populate here, unlike RefreshAfterStep()'s session-A path. Advance
	// what IS available: frame position and progress.
	timeline_dock_.SetProgress(src_step_pos_, src_source_.GetFrameCount());
	frame_canvas_.SetFrame(src_step_pos_);
	frame_canvas_.SetChangedRegions(Vector<VsmChangedRect>());
	PushCurrentFrameToPanels();
}

// ---------------------------------------------------------------------------
// Toolbar

void MainWindow::UpdateToolBar(Bar& bar)
{
	bar.Add("Step",         CtrlImg::right_arrow(), [=] { OnStep(); });
	bar.Add("Run All",      CtrlImg::go_forward(),  [=] { OnRunAll(); });
	bar.Separator();
	bar.Add("Reset",        CtrlImg::undo(),        [=] { OnResetReplay(); });
	bar.Separator();
	bar.Add("Run Pipeline", CtrlImg::go_forward(),  [=] { OnRunPipeline(); });
	bar.Separator();
	bar.Add("Regions",     [=] { OnToggleOverlay(0); }).Check(frame_canvas_.ShowRegions());
	bar.Add("Annotations", [=] { OnToggleOverlay(1); }).Check(frame_canvas_.ShowAnnotations());
	bar.Add("Template",    [=] { OnToggleOverlay(2); }).Check(frame_canvas_.ShowTemplate());
	bar.Add("OCR",         [=] { OnToggleOverlay(3); }).Check(frame_canvas_.ShowOcr());
	bar.Add("Layout",      [=] { OnToggleOverlay(4); }).Check(frame_canvas_.ShowLayout());
}

// ---------------------------------------------------------------------------
// Overlay toggles

void MainWindow::LoadOverlayState()
{
	bool r = (bool)registry_.Get("overlay.regions",     true);
	bool a = (bool)registry_.Get("overlay.annotations", true);
	bool t = (bool)registry_.Get("overlay.template",    true);
	bool o = (bool)registry_.Get("overlay.ocr",         true);
	bool l = (bool)registry_.Get("overlay.layout",      true);
	frame_canvas_.SetShowRegions(r);
	frame_canvas_.SetShowAnnotations(a);
	frame_canvas_.SetShowTemplate(t);
	frame_canvas_.SetShowOcr(o);
	frame_canvas_.SetShowLayout(l);
}

void MainWindow::SaveOverlayState()
{
	registry_.Set("overlay.regions",     (bool)frame_canvas_.ShowRegions());
	registry_.Set("overlay.annotations", (bool)frame_canvas_.ShowAnnotations());
	registry_.Set("overlay.template",    (bool)frame_canvas_.ShowTemplate());
	registry_.Set("overlay.ocr",         (bool)frame_canvas_.ShowOcr());
	registry_.Set("overlay.layout",      (bool)frame_canvas_.ShowLayout());
}

void MainWindow::OnToggleOverlay(int which)
{
	bool state = false;
	switch(which) {
	case 0: frame_canvas_.SetShowRegions    (!frame_canvas_.ShowRegions());     state = frame_canvas_.ShowRegions();     break;
	case 1: frame_canvas_.SetShowAnnotations(!frame_canvas_.ShowAnnotations());  state = frame_canvas_.ShowAnnotations(); break;
	case 2: frame_canvas_.SetShowTemplate   (!frame_canvas_.ShowTemplate());    state = frame_canvas_.ShowTemplate();   break;
	case 3: frame_canvas_.SetShowOcr        (!frame_canvas_.ShowOcr());         state = frame_canvas_.ShowOcr();        break;
	case 4: frame_canvas_.SetShowLayout     (!frame_canvas_.ShowLayout());      state = frame_canvas_.ShowLayout();     break;
	}
	SaveOverlayState();
	toolbar_.Set(THISBACK(UpdateToolBar));
	Log(Format("overlay[%d] = %s", which, state ? "on" : "off"));
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
	// Look up regions from whichever session is actually being displayed.
	// TexasHoldem sessions (C) DO have region data: the canvas fires
	// "region-N" where N indexes th_frame_nodes_ (the current frame's region
	// nodes, in overlay draw order) -- map that straight back to the node.
	if(active_session_ == ACTIVE_TEXAS) {
		if(id.StartsWith("region-")) {
			int n = ScanInt(id.Mid(7));
			if(n >= 0 && n < th_frame_nodes_.GetCount()) {
				const VsmRegionNode* rn = th_frame_nodes_[n];
				props_dock_.SetRegion(rn->id, rn);
				Log("region selected: " + rn->id);
				return;
			}
		}
		props_dock_.Clear();
		return;
	}
	// Opened/imported sessions (B) have no VsmRegionNode-shaped region list
	// in the headless API today (see RefreshAfterSourceStep()), so there is
	// nothing to search there -- report that plainly rather than
	// (incorrectly) matching against the unrelated sample session's (A)
	// regions, which is what happened before this fix.
	if(active_session_ == ACTIVE_IMPORTED) {
		props_dock_.Clear();
		Log("region: no region data available for the opened session (" + id + ")");
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
	if(active_session_ == ACTIVE_TEXAS) {
		if(row >= th_session_.regions.GetCount()) return;
		const VsmRegionNode& rn = th_session_.regions[row];
		props_dock_.SetRegion(rn.id, &rn);
		Log(Format("region list selected: %s", rn.id));
		return;
	}
	if(active_session_ == ACTIVE_IMPORTED) {
		Log("region list: no region data available for the opened session");
		return;
	}
	const VsmSession& s = replay_.GetSession();
	if(row >= s.regions.GetCount()) return;
	const VsmRegionNode& rn = s.regions[row];
	props_dock_.SetRegion(rn.id, &rn);
	Log(Format("region list selected: %s", rn.id));
}

void MainWindow::OnLayoutBindingSelected(int region_index)
{
	// A click in the Layout Bindings panel selects the SAME region a click on
	// the canvas / Regions list would, reusing the existing wiring rather than a
	// parallel mechanism: OnRegionSelected() drives the Region Properties panel,
	// the canvas highlights the region (+ its matched layout candidate), and the
	// Regions list cursor is moved to the matching global row.
	if(active_session_ != ACTIVE_TEXAS)
		return;
	if(region_index < 0 || region_index >= th_frame_nodes_.GetCount())
		return;

	OnRegionSelected(Format("region-%d", region_index));
	frame_canvas_.SelectRegion(region_index);

	// Map the current-frame region node back to its row in the whole-session
	// Regions list (th_frame_nodes_ holds pointers into th_session_.regions).
	const VsmRegionNode* rn = th_frame_nodes_[region_index];
	int global_row = (int)(rn - th_session_.regions.begin());
	if(global_row >= 0 && global_row < th_session_.regions.GetCount())
		regions_list_.SetCursor(global_row);
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
	frame_canvas_.SetAnnotationLayer(&annotation_layer_);
	frame_canvas_.WhenAnnotationCreated = [=] { OnAnnotationChanged(); };
	frame_canvas_.WhenAnnotationMoved   = [=] { OnAnnotationChanged(); };
	Log(Format("annotations: loaded %d annotation(s)", annotation_layer_.annotations.GetCount()));

	// Seed a default pipeline for the workbench
	if(current_pipeline_.steps.IsEmpty()) {
		current_pipeline_.id   = "pipe-default";
		current_pipeline_.name = "Default";
		VsmPreprocessStep s1; s1.type = VSM_PREP_GRAYSCALE;
		VsmPreprocessStep s2; s2.type = VSM_PREP_NORMALIZE_32;
		current_pipeline_.steps.Add(s1);
		current_pipeline_.steps.Add(s2);
	}
	pipeline_dock_.SetPipeline(&current_pipeline_);

	// Seed a sample template rule
	if(template_rules_.IsEmpty()) {
		VsmTemplateRule& r = template_rules_.Add();
		r.rule_id       = "rule-001";
		r.annotation_id = "ann-001";
		r.mode          = VSM_TM_PRESENCE;
		r.requirement   = VSM_TMR_OPTIONAL;
		r.threshold     = 0.8;
		r.pipeline_id   = current_pipeline_.id;
		VsmTemplateCandidate& c = r.candidates.Add();
		c.asset_id = "asset-sample"; c.label = "Sample";
	}
	template_dock_.SetRules(&template_rules_);

	// Seed a sample OCR rule
	if(ocr_rules_.IsEmpty()) {
		VsmOcrRule& r               = ocr_rules_.Add();
		r.rule_id                   = "ocr-001";
		r.annotation_id             = "ann-001";
		r.pipeline_id               = current_pipeline_.id;
		r.expectation.mode          = VSM_EXPECT_EXACT;
		r.expectation.expected_text = "Login";
		r.confidence_threshold      = 0.5;
	}
	ocr_dock_.SetRules(&ocr_rules_);

	// Seed a model rule that reads from the OCR rule above
	model_runtime_.Reset();
	model_runtime_.SetLog(&log_);
	{
		VsmModelRule mr;
		mr.rule_id        = "mr-001";
		mr.type           = VSM_MR_SET_PROP_FROM_OCR;
		mr.object_id      = "app-screen";
		mr.property_key   = "screen";
		mr.source_rule_id = "ocr-001";
		model_runtime_.AddRule(mr);
	}
	model_dock_.SetRuntime(&model_runtime_);
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
	bool save_success = false;
	if(!annotation_path_.IsEmpty())
		save_success = annotation_layer_.Save(annotation_path_);
	annotation_dock_.NotifySaveResult(save_success, annotation_path_.IsEmpty());
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Ground-truth comparison

void MainWindow::OnCompareGroundTruth()
{
	// This is the legacy generic .vsm/VsmGroundTruthLoader flow — it only
	// applies to opened/imported sessions (B). TexasHoldem sessions (C) have
	// their own ground-truth data (th_session_.ground_truth) consumed by later
	// M06 tasks, not by this comparator.
	if(active_session_ != ACTIVE_IMPORTED) {
		Log("ground truth: open an imported session first");
		return;
	}

	FileSel fs;
	fs.Type("VSM ground truth JSON", "*.json");
	if(!fs.ExecuteOpen("Select Ground Truth JSON"))
		return;
	String gt_path = ~fs;

	VsmGroundTruthSession gt;
	VsmGroundTruthLoader loader;
	loader.SetLog(&log_);
	if(!loader.Load(gt_path, gt)) {
		Log("ground truth: failed to load " + gt_path);
		PromptOK("Could not load ground truth JSON: " + gt_path + "\nSee Debug tab for details.");
		return;
	}

	// Re-open source from the beginning
	src_source_.Close();
	if(!src_source_.Open(session_store_.GetPaths().root)) {
		Log("ground truth: cannot reopen session source");
		PromptOK("Could not reopen session for comparison.\nSee Debug tab for details.");
		return;
	}

	VsmObservationPipeline pipe;
	pipe.SetLog(&log_);
	pipe.SetSessionStore(&session_store_);
	pipe.SetAnnotationLayer(&annotation_layer_);
	pipe.SetPreprocessPipeline(&current_pipeline_);
	pipe.SetTemplateRules(&template_rules_);
	pipe.SetOcrRules(&ocr_rules_);
	pipe.SetModelRuntime(&model_runtime_);
	VsmTemplateMatcher matcher;
	matcher.SetLog(&log_);
	pipe.SetTemplateMatcher(&matcher);

	VsmPipelineRunWithGTSummary summary = pipe.RunWithGroundTruth(src_source_, gt);
	const VsmComparisonResult& cmp = summary.comparison;

	model_dock_.SetRuntime(&model_runtime_);
	model_dock_.Refresh();

	String msg = Format("Comparison complete: %d matched, %d missing, %d unexpected",
	                    cmp.matched, cmp.missing, cmp.unexpected);
	Log(msg);
	PromptOK(msg);
}

// ---------------------------------------------------------------------------
// E2E sample session loader

void MainWindow::OnLoadE2ESample()
{
	String root = AppendFileName(GetTempPath(), "vsm_e2e_sample");
	if(!DirectoryExists(root)) {
		Log("e2e: run VisualStateEndToEndSample.exe first to generate the session");
		PromptOK("End-to-end sample session not found.\n\n"
		         "Please run VisualStateEndToEndSample.exe first to generate the session.");
		return;
	}
	OpenSessionPath(root);
	Log("e2e: session opened");
}

// ---------------------------------------------------------------------------
// Session open / import flow

void MainWindow::OpenSessionPath(const String& path)
{
	if(path.IsEmpty()) return;
	Log("session: opening " + path);

	src_source_.SetLog(&log_);
	if(!src_source_.Open(path)) {
		Log("session: open failed — " + src_source_.GetLastError());
		PromptOK("Could not open session: " + path + "\nSee Debug tab for details.");
		return;
	}

	session_store_.SetLog(&log_);
	if(!session_store_.Open(path)) {
		Log("session: store open failed");
		src_source_.Close();
		PromptOK("Could not read session metadata from: " + path + "\nSee Debug tab for details.");
		return;
	}

	// This session (B) becomes the active session: toolbar Step/Run All and
	// region lookups now act on it instead of the sample replay session (A)
	// -- see OnStep()/OnRunAll()/OnRegionSelected()/OnRegionListSel(). Also
	// discards any previously active TexasHoldem session (C) display state.
	ClearTexasSession();
	active_session_ = ACTIVE_IMPORTED;
	src_step_pos_    = 0;
	// No frame has been read from the newly opened session yet (Step/Run
	// All haven't run) -- clear any stale frame left over from whatever was
	// active before, and let the rule/pipeline preview panels know.
	current_frame_img_ = VsmImageBuffer();
	PushCurrentFrameToPanels();
	session_dock_.SetManifest(session_store_.GetManifest());

	// Clear any leftover session-A overlay/list state so the Frame/Regions
	// tabs don't show the unrelated sample's changed-regions or region rows
	// while this session is active.
	frame_canvas_.SetChangedRegions(Vector<VsmChangedRect>());
	regions_list_.Clear();

	// Persist last opened path
	registry_.Set("session.last_path", path);

	// Load annotation layer from session
	String ann_file = AppendFileName(session_store_.GetPaths().annotations_dir,
	                                 "annotations.json");
	annotation_layer_ = VsmAnnotationLayer();
	annotation_layer_.session_id = session_store_.GetManifest().session_id;
	if(FileExists(ann_file)) {
		annotation_layer_.Load(ann_file);
		Log("annotations: loaded " + IntStr(annotation_layer_.annotations.GetCount()) + " from session");
	} else {
		annotation_path_ = ann_file;
	}
	annotation_path_ = ann_file;
	annotation_dock_.SetLayer(&annotation_layer_);
	frame_canvas_.SetAnnotationLayer(&annotation_layer_);
	frame_canvas_.WhenAnnotationCreated = [=] { OnAnnotationChanged(); };
	frame_canvas_.WhenAnnotationMoved   = [=] { OnAnnotationChanged(); };

	Log("session: opened — " + session_store_.GetManifest().session_id +
	    " frames=" + IntStr(src_source_.GetFrameCount()) +
	    " " + IntStr(src_source_.GetWidth()) + "x" + IntStr(src_source_.GetHeight()));
}

void MainWindow::OnOpenImportSession()
{
	OpenImportDialog dlg;
	if(dlg.Execute() != IDOK) return;

	// User has explicitly loaded a session, so hide the empty-state placeholder
	frame_canvas_.SetShowEmptyStatePlaceholder(false);

	String path = dlg.GetPath();
	switch(dlg.GetSourceType()) {
	case OpenImportDialog::SESSION_DIR:
		OpenSessionPath(path);
		break;
	case OpenImportDialog::IMAGE_SEQUENCE:
		DispatchImageSequenceImport(path);
		break;
	case OpenImportDialog::TEXAS_SESSION:
		OpenTexasHoldemSession(path);
		break;
	case OpenImportDialog::SAMPLE:
		LoadSampleSession();
		break;
	case OpenImportDialog::E2E_SAMPLE:
		OnLoadE2ESample();
		break;
	}
}

// ---------------------------------------------------------------------------
// TexasHoldem M01/M02 session source (task 0131)

void MainWindow::ClearTexasSession()
{
	th_session_ = VsmTexasHoldemSession();
	th_step_pos_ = 0;
	th_frame_nodes_.Clear();
	frame_canvas_.SetFrameImage(Image());
	frame_canvas_.SetInfoText(String());

	// Layout-binding state (task 0132): drop the model/bindings so the panel and
	// canvas overlay don't linger when switching to the sample or an imported
	// session. Clear the canvas's borrowed pointers BEFORE the backing vectors
	// go away.
	frame_canvas_.SetLayoutBindings(nullptr);
	frame_canvas_.SetLayoutModel(nullptr);
	th_frame_bindings_.Clear();
	th_layout_model_ = VsmSessionLayoutModel();
	th_form_path_.Clear();
	layout_dock_.Clear();

	// Logic-state timeline (task 0133): drop the derived model so the panel
	// doesn't linger when switching to the sample or an imported session.
	th_logic_model_ = VsmSessionLogicModel();
	logic_dock_.SetModel(nullptr);
	logic_dock_.Clear();
}

void MainWindow::OpenTexasHoldemSession(const String& path)
{
	if(path.IsEmpty()) return;
	Log("texas session: opening " + path);

	VsmTexasHoldemSession sess;
	VsmTexasHoldemLoadResult r = VsmLoadTexasHoldemSession(path, sess, &log_);
	if(!r.success) {
		Log("texas session: load failed — " + r.error);
		PromptOK("Could not open TexasHoldem session:\n" + path + "\n\n" + r.error +
		         "\nSee Debug tab for details.");
		return;
	}

	// This TexasHoldem session (C) becomes the single active session. Close any
	// imported source (B) so it doesn't linger as a second "real" session.
	if(active_session_ == ACTIVE_IMPORTED)
		src_source_.Close();
	th_session_    = pick(sess);
	active_session_ = ACTIVE_TEXAS;
	th_step_pos_   = 0;

	// TexasHoldem sessions carry their own per-frame pixels via the adapter, so
	// the generic VsmImageBuffer "current frame" fed to the rule/pipeline
	// preview panels stays empty (those panels are out of scope here).
	current_frame_img_ = VsmImageBuffer();
	PushCurrentFrameToPanels();

	// Session Info panel from the real metadata.json.
	session_dock_.SetTexasHoldemInfo(th_session_.info);

	// Regions tab: one row per changed region per transition.
	RebuildTexasRegionsList();

	// A fresh, empty annotation layer (TexasHoldem sessions have no
	// annotations.json). Keep annotation authoring working, but with no backing
	// file to persist to (annotation_path_ empty -> NotifySaveResult path_empty).
	annotation_layer_ = VsmAnnotationLayer();
	annotation_layer_.session_id = th_session_.info.session_id;
	annotation_path_ = String();
	annotation_dock_.SetLayer(&annotation_layer_);
	frame_canvas_.SetSession(nullptr);
	frame_canvas_.SetAnnotationLayer(&annotation_layer_);
	frame_canvas_.WhenAnnotationCreated = [=] { OnAnnotationChanged(); };
	frame_canvas_.WhenAnnotationMoved   = [=] { OnAnnotationChanged(); };

	registry_.Set("session.last_path", path);

	// Layout-binding model (task 0132): resolve the GameTable_<provider>.form
	// for this session and build its (GUI-independent) candidate model once.
	// Search roots are exe-relative (the .form files live under game/TexasHoldem
	// in the repo, and the exe runs from bin/) plus cwd-relative fallbacks. If
	// none is found the layout view degrades to an "unavailable" state; the rest
	// of the TexasHoldem session flow is unaffected.
	Vector<String> form_roots;
	form_roots.Add(NormalizePath(AppendFileName(GetExeFolder(), "../game/TexasHoldem")));
	form_roots.Add(NormalizePath(AppendFileName(GetExeFolder(), "../../game/TexasHoldem")));
	form_roots.Add(NormalizePath(AppendFileName(GetCurrentDirectory(), "game/TexasHoldem")));
	form_roots.Add("game/TexasHoldem");
	th_form_path_    = VsmDefaultFormPathForProvider(th_session_.info.provider, form_roots);
	th_layout_model_ = VsmBuildSessionLayoutModel(th_session_, th_form_path_);
	layout_dock_.SetModel(&th_layout_model_);
	frame_canvas_.SetLayoutModel(&th_layout_model_);
	if(th_form_path_.IsEmpty())
		Log("texas layout: no GameTable_" + th_session_.info.provider +
		    ".form found on search roots; layout bindings unavailable");
	else if(th_layout_model_.loaded)
		Log(Format("texas layout: model built from %s — %d candidates (%d el + %d subslot) "
		           "scale sx=%s sy=%s", th_form_path_, th_layout_model_.candidates.GetCount(),
		           th_layout_model_.element_count, th_layout_model_.subslot_count,
		           DblStr(th_layout_model_.sx), DblStr(th_layout_model_.sy)));
	else
		Log("texas layout: model build FAILED from " + th_form_path_ + " — " + th_layout_model_.error);

	// Logic-state timeline model (task 0133): built once here from the SAME
	// resolved th_form_path_ the layout-binding model above uses. Calls the
	// shared uppsrc/VisualStateLogic/LogicCompare.h M05 derivation pipeline
	// (VsmDeriveSessionLogicStates), the exact same template-match scoring
	// reference/VisualStateLogicCompare's CLI performs — so, like that CLI,
	// this can take a noticeable amount of time for longer sessions (the cost
	// is dominated by per-slot/per-seat template-match scoring across every
	// transition, not anything this adapter adds). Degrades to an
	// "unavailable" state in the panel if no .form was resolved or derivation
	// fails; the rest of the TexasHoldem session flow is unaffected either way.
	if(th_form_path_.IsEmpty()) {
		th_logic_model_ = VsmSessionLogicModel();
		logic_dock_.SetModel(nullptr);
		Log("texas logic-state: no .form resolved; logic-state timeline unavailable");
	}
	else {
		th_logic_model_ = VsmBuildSessionLogicModel(th_session_, th_form_path_);
		logic_dock_.SetModel(&th_logic_model_);
		if(th_logic_model_.loaded)
			Log(Format("texas logic-state: derived %d frame record(s) from %s",
			           th_logic_model_.records.GetCount(), th_form_path_));
		else
			Log("texas logic-state: derivation FAILED from " + th_form_path_ +
			    " — " + th_logic_model_.error);
	}

	// Show the first frame (frame 0 has no predecessor -> no changed regions).
	SetTexasFrame(0);

	Log(Format("texas session: opened — %s provider=%s %dx%d frames=%d "
	           "regions=%d(records) distinct=%d gt=%d",
	           th_session_.info.session_id, th_session_.info.provider,
	           th_session_.info.table_width, th_session_.info.table_height,
	           th_session_.info.frame_count, r.region_records, r.distinct_regions,
	           r.ground_truth_records));
}

void MainWindow::SetTexasFrame(int frame_id)
{
	if(th_session_.IsEmpty()) return;
	int fc = th_session_.info.frame_count;
	if(frame_id < 0) frame_id = 0;
	if(fc > 0 && frame_id > fc - 1) frame_id = fc - 1;
	th_step_pos_ = frame_id;

	// Real frame image under the overlays.
	VsmFrameImage fi;
	if(VsmLoadTexasHoldemFrameImage(th_session_, frame_id, fi))
		frame_canvas_.SetFrameImage(VsmFrameImageToImage(fi));
	else
		frame_canvas_.SetFrameImage(Image());

	// Overlay: this frame's changed-region rectangles, in stable detection
	// order so canvas "region-N" click indices line up with th_frame_nodes_.
	th_frame_nodes_ = th_session_.RegionsForFrame(frame_id);
	Vector<VsmChangedRect> rects;
	for(const VsmRegionNode* rn : th_frame_nodes_) {
		VsmChangedRect cr;
		cr.x = rn->x; cr.y = rn->y; cr.w = rn->w; cr.h = rn->h;
		rects.Add(cr);
	}
	frame_canvas_.SetChangedRegions(rects);
	frame_canvas_.SetFrame(frame_id);
	frame_canvas_.SetInfoText(Format("TexasHoldem: %s  %dx%d  frame: %d/%d",
	                                 th_session_.info.session_id,
	                                 th_session_.info.table_width,
	                                 th_session_.info.table_height,
	                                 frame_id, fc - 1));

	// Layout bindings for this frame (task 0132): recomputed via the same shared
	// M04 matching functions the CLI uses. th_frame_bindings_'s region_index
	// values line up 1:1 with th_frame_nodes_ / the "region-N" overlay order, so
	// selecting a binding maps straight back to a region.
	th_frame_bindings_ = VsmComputeFrameBindings(th_session_, th_layout_model_, frame_id);
	layout_dock_.SetFrameBindings(frame_id, th_frame_bindings_);
	frame_canvas_.SetLayoutBindings(&th_frame_bindings_);

	// Logic-state timeline for this frame (task 0133): the model itself was
	// already fully derived once in OpenTexasHoldemSession(); scrubbing here
	// is just an O(1) lookup into th_logic_model_.records[frame_id].
	logic_dock_.SetFrame(frame_id);

	timeline_dock_.SetProgress(frame_id, fc);
}

void MainWindow::RebuildTexasRegionsList()
{
	regions_list_.Clear();
	for(const VsmRegionNode& rn : th_session_.regions) {
		String rect = Format("(%d,%d) ", rn.x, rn.y) + IntStr(rn.w) + "x" + IntStr(rn.h);
		regions_list_.Add(rn.id, rn.frame,
		                  rn.action.IsEmpty() ? "—" : rn.action,
		                  rect);
	}
}

// ---------------------------------------------------------------------------
// Import dispatch helpers

static bool HasVsmFiles(const String& dir)
{
	FindFile ff;
	if(!ff.Search(AppendFileName(dir, "*.vsm"))) return false;
	do { if(!ff.IsDirectory()) return true; } while(ff.Next());
	return false;
}

static bool HasJpegFiles(const String& dir)
{
	static const char* exts[] = { "*.jpg", "*.jpeg", "*.png", nullptr };
	for(int i = 0; exts[i]; i++) {
		FindFile ff;
		if(ff.Search(AppendFileName(dir, exts[i]))) {
			do { if(!ff.IsDirectory()) return true; } while(ff.Next());
		}
	}
	return false;
}

void MainWindow::RunVsmImport(const String& src_dir)
{
	String out_dir = AppendFileName(GetTempPath(),
	                                "vsm_import_" + IntStr((int)GetTickCount()));
	Log("import: scanning .vsm in " + src_dir);

	VsmImageSequenceImportOptions opts;
	opts.source_dir = src_dir;
	opts.output_dir = out_dir;
	opts.fps        = 30;

	VsmImageSequenceImporter importer;
	importer.SetLog(&log_);
	VsmImageSequenceImportResult res = importer.Import(opts);

	if(!res.success) {
		Log("import: failed — check debug log");
		return;
	}

	Log(Format("import: %d frames imported into %s", res.frames_imported, out_dir));
	for(const VsmImportWarning& w : res.warnings)
		Log("import warning: " + w.filename + " — " + w.message);

	OpenSessionPath(out_dir);
}

void MainWindow::RunJpegImport(const String& src_dir)
{
	String out_dir = AppendFileName(GetTempPath(),
	                                "vsm_jpeg_" + IntStr((int)GetTickCount()));
	Log("import: scanning .jpg/.jpeg/.png in " + src_dir);

	VsmJpegImportOptions opts;
	opts.source_dir = src_dir;
	opts.output_dir = out_dir;
	opts.fps        = 30;
	opts.grayscale  = true;

	JpegSequenceImporter importer;
	importer.SetLog(&log_);
	VsmJpegImportResult res = importer.Import(opts);

	if(!res.success) {
		Log("import: JPEG/PNG import failed — check debug log");
		return;
	}

	Log(Format("import: %d/%d frames imported into %s",
	           res.frames_imported, res.frames_scanned, out_dir));
	for(const String& w : res.warnings)
		Log("import warning: " + w);

	OpenSessionPath(out_dir);
}

void MainWindow::DispatchImageSequenceImport(const String& src_dir)
{
	registry_.Set("session.last_import_dir", src_dir);

	bool has_vsm  = HasVsmFiles(src_dir);
	bool has_jpeg = HasJpegFiles(src_dir);

	if(has_jpeg && !has_vsm)   RunJpegImport(src_dir);
	else if(has_vsm)           RunVsmImport(src_dir);
	else                       PromptOK("No .vsm or .jpg/.png files found.");
}

void MainWindow::RunJpegSmokeTest()
{
	String src_dir = AppendFileName(GetTempPath(), "vsm_jpeg_smoke_src");
	String out_dir = AppendFileName(GetTempPath(), "vsm_jpeg_smoke_out");

	// Write 3 synthetic JPEGs (64×64 gray ramps)
	RealizeDirectory(src_dir);
	for(int i = 0; i < 3; i++) {
		ImageBuffer ib(64, 64);
		byte gray = (byte)(64 + i * 64);
		RGBA px;
		px.r = gray; px.g = gray; px.b = gray; px.a = 255;
		for(int y = 0; y < 64; y++)
			for(int x = 0; x < 64; x++)
				ib[y][x] = px;
		Image img = ib;
		String fname = Format("%04d_frame.jpg", i);
		JPGEncoder(80).SaveFile(AppendFileName(src_dir, fname), img);
	}

	if(DirectoryExists(out_dir))
		DeleteFolderDeep(out_dir);

	VsmJpegImportOptions opts;
	opts.source_dir   = src_dir;
	opts.output_dir   = out_dir;
	opts.fps          = 25;
	opts.sort_numeric = true;
	opts.grayscale    = true;

	JpegSequenceImporter importer;
	importer.SetLog(&log_);
	VsmJpegImportResult res = importer.Import(opts);

	if(res.success && res.frames_imported == 3)
		Log(Format("JPEG smoke test OK: %d/%d frames imported",
		           res.frames_imported, res.frames_scanned));
	else
		Log(Format("JPEG smoke test FAIL: success=%s imported=%d scanned=%d",
		           res.success ? "true" : "false",
		           res.frames_imported, res.frames_scanned));
}

// ---------------------------------------------------------------------------
// Cache

void MainWindow::OnClearCache()
{
	int prior_count = pipeline_cache_.GetCount();
	pipeline_cache_.Clear();
	Log(Format("cache: cleared (was %d entries)", prior_count));
}

// ---------------------------------------------------------------------------
// Pipeline runner

void MainWindow::OnRunPipeline()
{
	Log("pipeline: starting run…");

	VsmObservationPipeline pipe;
	pipe.SetLog(&log_);
	if(session_store_.IsOpen())
		pipe.SetSessionStore(&session_store_);
	pipe.SetAnnotationLayer(&annotation_layer_);
	pipe.SetPreprocessPipeline(&current_pipeline_);
	pipe.SetTemplateRules(&template_rules_);
	pipe.SetOcrRules(&ocr_rules_);
	pipe.SetModelRuntime(&model_runtime_);

	VsmTemplateMatcher matcher;
	matcher.SetLog(&log_);
	pipe.SetTemplateMatcher(&matcher);

	// The generic observation pipeline runs against the sample (A) or an
	// opened/imported (B) session. TexasHoldem sessions (C) use the M05
	// template-match / logic-state pipeline (reference/VisualStateLogicCompare),
	// not this one -- guard it out rather than run against a stale source.
	if(active_session_ == ACTIVE_TEXAS) {
		Log("pipeline: not applicable to TexasHoldem sessions in this task");
		return;
	}

	VsmPipelineRunSummary summary;
	if(active_session_ == ACTIVE_IMPORTED) {
		// Re-open source so we read from the beginning
		String path = session_store_.GetPaths().root;
		src_source_.Close();
		src_source_.Open(path);
		summary = pipe.RunFromSource(src_source_);
		// RunFromSource() reads src_source_ through to the end; keep the
		// Step/Run All position bookkeeping in sync so a subsequent Step
		// correctly reports "already at end" instead of appearing stale.
		src_step_pos_ = summary.frames_processed;
		RefreshAfterSourceStep();
	} else {
		const VsmSession& session = replay_.GetSession();
		if(session.session_id.IsEmpty()) {
			Log("pipeline: no session loaded");
			return;
		}
		pipe.SetSession(&session);
		summary = pipe.Run();
	}

	// Show summary + cache stats in log
	Log(Format("pipeline: done — frames=%d obs=%d transitions=%d divergences=%d  cache hits=%d misses=%d",
	           summary.frames_processed, summary.observations_made, summary.transitions,
	           summary.divergences, pipeline_cache_.GetHits(), pipeline_cache_.GetMisses()));
	pipeline_cache_.ResetStats();

	if(!summary.success) {
		Log("pipeline: run failed");
		PromptOK("Pipeline run failed.\nSee Debug tab for details.");
		return;
	}

	// Save outputs to session store if open
	if(session_store_.IsOpen())
		pipe.SaveOutputs(session_store_.GetPaths().root, summary.run_id);

	// Refresh model state panel
	model_dock_.SetRuntime(&model_runtime_);
	model_dock_.Refresh();
}

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

// ---------------------------------------------------------------------------
// Jump to frame

void MainWindow::OnJumpToFrame(int frame)
{
	frame_canvas_.SetFrame(frame);
	// Update session info panel to reflect the active frame
	if(session_store_.IsOpen()) {
		const VsmSessionManifest& m = session_store_.GetManifest();
		if(frame >= 0 && frame < m.frames.GetCount())
			session_dock_.SetManifest(m);
	}

	// Load the real frame image for the jumped-to frame, if available.
	// Only opened/imported sessions (B) have real per-frame image bytes in
	// session_store_ -- the sample session (A) never does (see
	// LoadSampleSession()) -- so jumping while the sample is active leaves
	// no real frame for the rule/pipeline preview panels.
	VsmImageBuffer img;
	if(active_session_ == ACTIVE_IMPORTED && session_store_.IsOpen() && session_store_.LoadFrameImage(frame, img))
		current_frame_img_ = pick(img);
	else
		current_frame_img_ = VsmImageBuffer();
	PushCurrentFrameToPanels();

	Log(Format("divergence: jump to frame %d", frame));
}
