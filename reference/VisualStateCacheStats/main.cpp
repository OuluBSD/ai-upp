#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

// Helper: Create a synthetic session with a few frames
static VsmSession MakeSyntheticSession()
{
	VsmSession session;
	session.session_id = "cache-stats-demo";
	session.schema = 1;

	// Add two frames
	for(int i = 0; i < 2; i++) {
		VsmFrameRef& f = session.frames.Add();
		f.frame = i;
		f.ts = Format("%04d-01-15T14:%02d:%02d.000Z", 2026, i, i);
		f.image_file = Format("frames/%08d.vsm", i);
	}

	return session;
}

// Helper: Fill an image with solid color
static void FillSolidRGBA(VsmFrameImage& img, int w, int h,
                          byte r, byte g, byte b)
{
	img.Set(w, h, nullptr);
	for(int i = 0; i < w * h * 4; i += 4) {
		img.data[i + 0] = r;
		img.data[i + 1] = g;
		img.data[i + 2] = b;
		img.data[i + 3] = 255;
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	String session_dir;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateCacheStats [<session_dir>]\n";
			SetExitCode(0);
			return;
		} else {
			session_dir = arg;
		}
	}

	Cout() << "=== VisualStateModel Pipeline Cache Statistics CLI ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Create a temporary session store or load a real one
	String root;
	bool is_synthetic = session_dir.IsEmpty();

	if(is_synthetic) {
		// Synthetic path: create temporary session
		root = AppendFileName(GetTempPath(), "vsm_cache_stats_demo");
		if(DirectoryExists(root)) DeleteFolderDeep(root);

		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Create(root, "cache-stats-demo", 32, 32)) {
			Fail("SessionStore Create");
			return;
		}
		Cout() << "Created session store: " << root << "\n";

		// Create synthetic frames
		VsmImageBuffer f0 = VsmImageBuffer::MakeSolid(32, 32, 100, 1);
		VsmImageBuffer f1 = VsmImageBuffer::MakeSolid(32, 32, 150, 1);
		store.SaveFrameImage(0, f0);
		store.SaveFrameImage(1, f1);
		store.SaveManifest();
		Cout() << "Created 2 frames\n\n";
	} else {
		// Real session path: load existing session
		root = session_dir;
		if(!DirectoryExists(root)) {
			Fail(Format("Session directory not found: %s", root));
			return;
		}
		VsmSessionStore store;
		store.SetLog(&log);
		if(!store.Open(root)) {
			Fail(Format("Failed to open session: %s", root));
			return;
		}
		Cout() << "Loaded session from: " << root << "\n\n";
	}

	// Create cache
	String cache_dir = AppendFileName(root, "cache");
	VsmPipelineCache cache;
	cache.SetLog(&log);
	if(!cache.Open(cache_dir)) {
		Fail("Cache Open");
		return;
	}
	Cout() << "Opened cache at: " << cache_dir << "\n\n";

	// Build a synthetic session
	VsmSession session = MakeSyntheticSession();

	// Create preprocessing pipeline
	VsmPreprocessPipeline preprocess_pipe;
	preprocess_pipe.id   = "pipe-demo";
	preprocess_pipe.name = "Demo Preprocessing";
	VsmPreprocessStep gs;
	gs.type = VSM_PREP_GRAYSCALE;
	VsmPreprocessStep n32;
	n32.type = VSM_PREP_NORMALIZE_32;
	preprocess_pipe.steps.Add(gs);
	preprocess_pipe.steps.Add(n32);

	// Create template rules
	VsmFingerprint32 fp;
	memset(fp.data, 128, sizeof(fp.data));

	VsmTemplateMatcher matcher;
	matcher.SetLog(&log);
	matcher.AddSyntheticAsset("asset-demo-0", fp);
	matcher.AddSyntheticAsset("asset-demo-1", fp);

	Vector<VsmTemplateRule> tmpl_rules;

	{
		VsmTemplateRule& r = tmpl_rules.Add();
		r.rule_id = "rule-demo-0";
		r.annotation_id = "ann-demo-0";
		r.mode = VSM_TM_PRESENCE;
		r.threshold = 0.5;
		r.requirement = VSM_TMR_OPTIONAL;
		VsmTemplateCandidate& c = r.candidates.Add();
		c.asset_id = "asset-demo-0";
		c.label = "DemoAsset0";
	}

	{
		VsmTemplateRule& r = tmpl_rules.Add();
		r.rule_id = "rule-demo-1";
		r.annotation_id = "ann-demo-1";
		r.mode = VSM_TM_PRESENCE;
		r.threshold = 0.5;
		r.requirement = VSM_TMR_OPTIONAL;
		VsmTemplateCandidate& c = r.candidates.Add();
		c.asset_id = "asset-demo-1";
		c.label = "DemoAsset1";
	}

	// Create annotation layer
	VsmAnnotationLayer ann;
	ann.schema = 1;
	ann.session_id = "cache-stats-demo";
	{
		VsmRegionAnnotation& a = ann.annotations.Add();
		a.id = "ann-demo-0";
		a.name = "Region 0";
		a.x = 0; a.y = 0; a.w = 32; a.h = 32;
	}
	{
		VsmRegionAnnotation& a = ann.annotations.Add();
		a.id = "ann-demo-1";
		a.name = "Region 1";
		a.x = 0; a.y = 0; a.w = 32; a.h = 32;
	}

	// Setup OCR engine
	VsmFakeOcrEngine fake_ocr("Demo", 0.90);

	// Setup model runtime
	VsmModelRuntime model_rt;
	model_rt.SetLog(&log);

	// === FIRST RUN ===
	Cout() << "--- RUN 1 (Cache Empty) ---\n";

	{
		VsmSessionStoreSource src1;
		src1.SetLog(&log);
		if(!src1.Open(root)) {
			Fail("FrameSource Open (run 1)");
			return;
		}

		VsmObservationPipeline pipe1;
		pipe1.SetLog(&log);
		pipe1.SetAnnotationLayer(&ann);
		pipe1.SetPreprocessPipeline(&preprocess_pipe);
		pipe1.SetTemplateRules(&tmpl_rules);
		pipe1.SetTemplateMatcher(&matcher);
		pipe1.SetOcrRules(&Vector<VsmOcrRule>());
		pipe1.SetOcrEngine(&fake_ocr);
		pipe1.SetModelRuntime(&model_rt);

		VsmPipelineRunSummary summary1 = pipe1.RunFromSource(src1);
		if(!summary1.success) {
			Fail("Pipeline RunFromSource (run 1)");
			return;
		}

		Cout() << "  Frames processed: " << summary1.frames_processed << "\n";
		Cout() << "  Observations made: " << summary1.observations_made << "\n";
	}

	// Simulate cache lookups for the preprocessing/rule computations
	// (demonstrating cache miss on first attempt)
	{
		for(int frame_idx = 0; frame_idx < 2; frame_idx++) {
			for(int rule_idx = 0; rule_idx < 2; rule_idx++) {
				VsmCacheKey key;
				key.asset_id = Format("frames/%08d.vsm", frame_idx);
				key.pipeline_id = "pipe-demo";
				key.rule_id = Format("rule-demo-%d", rule_idx);
				key.rule_type = "template";

				String result;
				// This should miss on first run
				cache.Get(key, result);
				// Store something for next run
				cache.Put(key, Format("{\"frame\":%d,\"rule\":%d}", frame_idx, rule_idx));
			}
		}
	}

	int hits_run1 = cache.GetHits();
	int misses_run1 = cache.GetMisses();
	int entries_run1 = cache.GetCount();

	Cout() << "  Cache hits: " << hits_run1 << "\n";
	Cout() << "  Cache misses: " << misses_run1 << "\n";
	Cout() << "  Cache entries: " << entries_run1 << "\n\n";

	// Save cache to disk
	if(!cache.Save()) {
		Fail("Cache Save after run 1");
		return;
	}
	Cout() << "Cache saved to disk\n\n";

	// Reset model runtime for clean second run
	model_rt.Reset();

	// === SECOND RUN ===
	Cout() << "--- RUN 2 (Cache Loaded) ---\n";

	{
		VsmSessionStoreSource src2;
		src2.SetLog(&log);
		if(!src2.Open(root)) {
			Fail("FrameSource Open (run 2)");
			return;
		}

		VsmObservationPipeline pipe2;
		pipe2.SetLog(&log);
		pipe2.SetAnnotationLayer(&ann);
		pipe2.SetPreprocessPipeline(&preprocess_pipe);
		pipe2.SetTemplateRules(&tmpl_rules);
		pipe2.SetTemplateMatcher(&matcher);
		pipe2.SetOcrRules(&Vector<VsmOcrRule>());
		pipe2.SetOcrEngine(&fake_ocr);
		pipe2.SetModelRuntime(&model_rt);

		VsmPipelineRunSummary summary2 = pipe2.RunFromSource(src2);
		if(!summary2.success) {
			Fail("Pipeline RunFromSource (run 2)");
			return;
		}

		Cout() << "  Frames processed: " << summary2.frames_processed << "\n";
		Cout() << "  Observations made: " << summary2.observations_made << "\n";
	}

	// Simulate cache lookups for the same computations
	// (should hit on second attempt)
	{
		for(int frame_idx = 0; frame_idx < 2; frame_idx++) {
			for(int rule_idx = 0; rule_idx < 2; rule_idx++) {
				VsmCacheKey key;
				key.asset_id = Format("frames/%08d.vsm", frame_idx);
				key.pipeline_id = "pipe-demo";
				key.rule_id = Format("rule-demo-%d", rule_idx);
				key.rule_type = "template";

				String result;
				// This should hit on second run
				cache.Get(key, result);
			}
		}
	}

	int hits_run2 = cache.GetHits();
	int misses_run2 = cache.GetMisses();
	int entries_run2 = cache.GetCount();

	Cout() << "  Cache hits: " << hits_run2 << "\n";
	Cout() << "  Cache misses: " << misses_run2 << "\n";
	Cout() << "  Cache entries: " << entries_run2 << "\n\n";

	// === VALIDATION ===
	Cout() << "--- Verification ---\n";

	if(is_synthetic) {
		// For synthetic sessions, verify specific expected values
		if(entries_run2 != 4) {
			Fail(Format("Expected 4 cache entries, got %d", entries_run2));
			return;
		}
		Cout() << "  Cache entry count: " << entries_run2 << " OK\n";

		// Verify that hits increased from run 1 to run 2
		if(hits_run2 <= hits_run1) {
			Fail(Format("Expected hits to increase (run1=%d, run2=%d)", hits_run1, hits_run2));
			return;
		}
		Cout() << "  Cache hits increased: " << hits_run1 << " -> " << hits_run2 << " OK\n";

		// Verify that cache was actually used (hits in run 2 should match lookups)
		if(hits_run2 - hits_run1 != 4) {
			Fail(Format("Expected 4 new hits in run 2 (got %d)", hits_run2 - hits_run1));
			return;
		}
		Cout() << "  Cache hits on second run: " << (hits_run2 - hits_run1) << " OK\n";
	} else {
		// For real sessions, just verify basic cache behavior
		Cout() << "  Cache entries created: " << entries_run2 << "\n";
		Cout() << "  Cache hits increased: " << hits_run1 << " -> " << hits_run2 << "\n";
		if(hits_run2 > hits_run1) {
			Cout() << "  Cache hit improvement: OK\n";
		} else {
			Cout() << "  WARNING: No cache hit improvement between runs\n";
		}
	}

	Cout() << "\n=== Cache statistics demo completed successfully ===\n";
	Cout() << "Summary:\n";
	Cout() << "  Run 1 - Hits: " << hits_run1 << ", Misses: " << misses_run1 << ", Entries: " << entries_run1 << "\n";
	Cout() << "  Run 2 - Hits: " << hits_run2 << ", Misses: " << misses_run2 << ", Entries: " << entries_run2 << "\n";

	// Cleanup (only synthetic sessions)
	if(is_synthetic) {
		DeleteFolderDeep(root);
	}
}
