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
	DockLeft(pipeline_dock_);
	DockLeft(template_dock_);
	DockLeft(ocr_dock_);
	DockLeft(model_dock_);
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
	// active session, discarding any opened/imported session (B) identity.
	has_src_session_ = false;
	src_step_pos_    = 0;
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
	// Operate on whichever session is active: the opened/imported session
	// (B, src_source_) once one has been opened, else the sample replay
	// session (A, replay_). This mirrors the has_src_session_ branch
	// OnRunPipeline() already uses for "Run Pipeline".
	if(has_src_session_) {
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
	if(has_src_session_) {
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
	// Resetting the sample is only destructive when an opened/imported
	// session (B) is currently active -- confirm before discarding it.
	if(has_src_session_) {
		if(!PromptYesNo("This will discard the currently opened session and "
		                "reload the built-in sample. Continue?"))
			return;
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
}

// ---------------------------------------------------------------------------
// Overlay toggles

void MainWindow::LoadOverlayState()
{
	bool r = (bool)registry_.Get("overlay.regions",     true);
	bool a = (bool)registry_.Get("overlay.annotations", true);
	bool t = (bool)registry_.Get("overlay.template",    true);
	bool o = (bool)registry_.Get("overlay.ocr",         true);
	frame_canvas_.SetShowRegions(r);
	frame_canvas_.SetShowAnnotations(a);
	frame_canvas_.SetShowTemplate(t);
	frame_canvas_.SetShowOcr(o);
}

void MainWindow::SaveOverlayState()
{
	registry_.Set("overlay.regions",     (bool)frame_canvas_.ShowRegions());
	registry_.Set("overlay.annotations", (bool)frame_canvas_.ShowAnnotations());
	registry_.Set("overlay.template",    (bool)frame_canvas_.ShowTemplate());
	registry_.Set("overlay.ocr",         (bool)frame_canvas_.ShowOcr());
}

void MainWindow::OnToggleOverlay(int which)
{
	bool state = false;
	switch(which) {
	case 0: frame_canvas_.SetShowRegions    (!frame_canvas_.ShowRegions());     state = frame_canvas_.ShowRegions();     break;
	case 1: frame_canvas_.SetShowAnnotations(!frame_canvas_.ShowAnnotations());  state = frame_canvas_.ShowAnnotations(); break;
	case 2: frame_canvas_.SetShowTemplate   (!frame_canvas_.ShowTemplate());    state = frame_canvas_.ShowTemplate();   break;
	case 3: frame_canvas_.SetShowOcr        (!frame_canvas_.ShowOcr());         state = frame_canvas_.ShowOcr();        break;
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
	// Opened/imported sessions (B) have no VsmRegionNode-shaped region list
	// in the headless API today (see RefreshAfterSourceStep()), so there is
	// nothing to search there -- report that plainly rather than
	// (incorrectly) matching against the unrelated sample session's (A)
	// regions, which is what happened before this fix.
	if(has_src_session_) {
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
	if(has_src_session_) {
		Log("region list: no region data available for the opened session");
		return;
	}
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
	if(!annotation_path_.IsEmpty()) {
		annotation_layer_.Save(annotation_path_);
		save_success = true;
	}
	annotation_dock_.NotifySaveResult(save_success, annotation_path_.IsEmpty());
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Ground-truth comparison

void MainWindow::OnCompareGroundTruth()
{
	if(!has_src_session_) {
		Log("ground truth: open a session first");
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
	// -- see OnStep()/OnRunAll()/OnRegionSelected()/OnRegionListSel().
	has_src_session_ = true;
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

void MainWindow::OnOpenSession()
{
	String start = registry_.Get("session.last_path", GetTempPath());
	String path  = SelectDirectory();
	if(path.IsEmpty()) return;
	OpenSessionPath(path);
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

void MainWindow::OnImportImageSequence()
{
	String src_dir = SelectDirectory();
	if(src_dir.IsEmpty()) return;

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

	VsmPipelineRunSummary summary;
	if(has_src_session_) {
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
	if(has_src_session_ && session_store_.IsOpen() && session_store_.LoadFrameImage(frame, img))
		current_frame_img_ = pick(img);
	else
		current_frame_img_ = VsmImageBuffer();
	PushCurrentFrameToPanels();

	Log(Format("divergence: jump to frame %d", frame));
}
