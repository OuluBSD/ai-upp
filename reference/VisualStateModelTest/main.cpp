#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

// ---------------------------------------------------------------------------
// Test: Types serialization round-trip

static void TestTypes()
{
	Cout() << "=== Types round-trip ===\n";

	VsmChangedRect cr;
	cr.x = 10; cr.y = 20; cr.w = 80; cr.h = 40; cr.score = 0.91;
	String json = StoreAsJson(cr);
	VsmChangedRect cr2;
	LoadFromJson(cr2, json);
	if(cr2.x != 10 || cr2.y != 20 || cr2.w != 80 || cr2.h != 40)
		Fail("ChangedRect round-trip");
	else
		Cout() << "ChangedRect: OK\n";

	VsmRegionFingerprint fp;
	fp.hash = "sha1:abcdef";
	fp.file = "crops/test.fp.bin";
	String fpJson = StoreAsJson(fp);
	VsmRegionFingerprint fp2;
	LoadFromJson(fp2, fpJson);
	if(fp2.hash != fp.hash || fp2.file != fp.file)
		Fail("Fingerprint round-trip");
	else
		Cout() << "RegionFingerprint: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Region memory

static void TestRegionMemory()
{
	Cout() << "\n=== Region memory ===\n";

	VsmRegionMemory mem;
	AppLog log;
	log.SetForwardToUppLog(false);
	mem.SetLog(&log);

	// Build two synthetic fingerprints
	VsmFingerprint32 a, b;
	memset(a.data, 128, sizeof(a.data));
	memset(b.data, 130, sizeof(b.data)); // very similar to a

	mem.Add("rgn-0001", a);
	mem.Add("rgn-0002", b);

	if(mem.GetCount() != 2)
		Fail("RegionMemory count");
	else
		Cout() << "RegionMemory count: OK\n";

	// Query with a fingerprint close to 'a'
	VsmFingerprint32 q;
	memset(q.data, 129, sizeof(q.data));
	VsmRegionMatch m = mem.FindNearest(q, 0.1);
	if(m.region_id.IsEmpty())
		Fail("FindNearest: no match found");
	else
		Cout() << "FindNearest: matched " << m.region_id
		       << " distance=" << m.distance << " OK\n";

	// Distance sanity: identical fingerprints → distance 0
	double d = VsmRegionMemory::Distance(a, a);
	if(d != 0.0)
		Fail("Distance(a,a) != 0");
	else
		Cout() << "Distance self: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Ground truth loader

static void TestGroundTruth()
{
	Cout() << "\n=== Ground truth loader ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	VsmGroundTruthLoader loader;
	loader.SetLog(&log);

	VsmSession session;
	String sample = VsmMakeSampleJson();
	String tmpPath = AppendFileName(GetTempPath(), "vsm_test.json");
	SaveFile(tmpPath, sample);

	if(!loader.Load(tmpPath, session)) {
		Fail("GroundTruth load");
		return;
	}
	FileDelete(tmpPath);

	if(session.schema != 1)               Fail("schema != 1");
	else if(session.frames.GetCount() != 2)   Fail("frame count != 2");
	else if(session.changes.GetCount() != 1)  Fail("change count != 1");
	else if(session.regions.GetCount() != 1)  Fail("region count != 1");
	else if(session.divergences.GetCount() != 1) Fail("divergence count != 1");
	else {
		Cout() << "Schema: " << session.schema << " OK\n";
		Cout() << "Frames: " << session.frames.GetCount() << " OK\n";
		Cout() << "Changes: " << session.changes.GetCount() << " — regions: "
		       << session.changes[0].regions.GetCount() << " OK\n";
		Cout() << "Regions: " << session.regions.GetCount() << " OK\n";
		Cout() << "Divergences: " << session.divergences.GetCount() << " OK\n";
	}
}

// ---------------------------------------------------------------------------
// Test: Replay session

static void TestReplay()
{
	Cout() << "\n=== Replay session ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);
	int record_count = 0;
	log.WhenRecord = [&](const AppLogRecord&) { record_count++; };

	VsmReplaySession replay;
	replay.SetLog(&log);

	String sample = VsmMakeSampleJson();
	String tmpPath = AppendFileName(GetTempPath(), "vsm_replay.json");
	SaveFile(tmpPath, sample);
	if(!replay.Load(tmpPath)) {
		Fail("Replay load");
		return;
	}
	FileDelete(tmpPath);

	int total = replay.GetTotalEvents();
	Cout() << "Total step events: " << total << "\n";

	replay.RunAll();

	if(replay.CanStep())
		Fail("CanStep after RunAll");
	else
		Cout() << "RunAll: OK\n";

	if(replay.GetDivergences().GetCount() != 1)
		Fail("Expected 1 divergence");
	else
		Cout() << "Divergences: 1 OK\n";

	Cout() << "AppLog records: " << record_count << "\n";
}

// ---------------------------------------------------------------------------
// Test: Change detection

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

static void FillRect(VsmFrameImage& img, int rx, int ry, int rw, int rh,
                     byte r, byte g, byte b)
{
	for(int y = ry; y < ry + rh; y++) {
		for(int x = rx; x < rx + rw; x++) {
			byte* p = img.data + (y * img.width + x) * 4;
			p[0] = r; p[1] = g; p[2] = b; p[3] = 255;
		}
	}
}

static void TestChangeDetect()
{
	Cout() << "\n=== Change detection ===\n";

	// Frame A: all gray (128)
	VsmFrameImage frameA;
	FillSolidRGBA(frameA, 320, 240, 128, 128, 128);

	// Frame B: gray with one white rectangle changed at (50,60,100,80)
	VsmFrameImage frameB;
	FillSolidRGBA(frameB, 320, 240, 128, 128, 128);
	FillRect(frameB, 50, 60, 100, 80, 255, 255, 255);

	VsmChangeDetectParams params;
	params.pixel_threshold = 30;

	Vector<VsmChangedRect> changes = VsmDetectChanges(frameA, frameB, params);
	if(changes.IsEmpty()) {
		Fail("DetectChanges: no changes found");
		return;
	}
	Cout() << "Detected " << changes.GetCount() << " changed region(s)\n";
	Cout() << "First region: (" << changes[0].x << "," << changes[0].y
	       << ") " << changes[0].w << "x" << changes[0].h
	       << " score=" << changes[0].score << "\n";
	if(changes[0].score < 0.1)
		Fail("DetectChanges: score too low");
	else
		Cout() << "DetectChanges: OK\n";

	// Test VsmCompareFrames
	VsmChangeEvent ev = VsmCompareFrames(frameA, frameB, 1, "2026-01-15T14:23:00.033Z");
	if(ev.frame != 1 || ev.regions.IsEmpty())
		Fail("VsmCompareFrames: unexpected result");
	else
		Cout() << "VsmCompareFrames: OK\n";

	// Identical frames → no changes
	Vector<VsmChangedRect> nochanges = VsmDetectChanges(frameA, frameA, params);
	if(!nochanges.IsEmpty())
		Fail("Identical frames should produce no changes");
	else
		Cout() << "Identical frames: no changes OK\n";

	// Fingerprint extraction from a changed region
	VsmFingerprint32 fp;
	bool ok = VsmRegionMemory::ExtractFingerprint(frameB, 50, 60, 100, 80, fp);
	if(!ok)
		Fail("ExtractFingerprint failed");
	else {
		String hash = fp.ComputeHash();
		Cout() << "Fingerprint hash: " << hash << " OK\n";
	}
}

// ---------------------------------------------------------------------------
// Test: Application model runtime

static void TestModelRuntime()
{
	Cout() << "\n=== Application model runtime ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	VsmModelRuntime rt;
	rt.SetLog(&log);

	// Rule 1: set 'screen' property from OCR result of rule 'ocr-001'
	VsmModelRule r1;
	r1.rule_id        = "mr-001";
	r1.type           = VSM_MR_SET_PROP_FROM_OCR;
	r1.object_id      = "app-screen";
	r1.property_key   = "screen";
	r1.source_rule_id = "ocr-001";
	rt.AddRule(r1);

	// Rule 2: validate 'screen' == "Login" (should pass)
	VsmModelRule r2;
	r2.rule_id        = "mr-002";
	r2.type           = VSM_MR_VALIDATE_PROP;
	r2.object_id      = "app-screen";
	r2.property_key   = "screen";
	r2.expected_value = "\"Login\"";
	r2.source_rule_id = "ocr-001";
	rt.AddRule(r2);

	// Event 1: OCR sees "Login" → set + validate (should succeed)
	VsmModelEvent ev1;
	ev1.type           = "ocr_result";
	ev1.source_rule_id = "ocr-001";
	ev1.data_json      = "\"Login\"";
	ev1.ts             = "2026-01-15T14:23:00.000Z";

	VsmModelRuntimeResult res1 = rt.ApplyEvent(ev1);
	if(res1.transitions.IsEmpty()) { Fail("Expected transition from ocr_result"); return; }
	Cout() << "Transition: " << res1.transitions[0].property_key
	       << " = " << res1.transitions[0].to_value << " OK\n";
	if(!res1.divergences.IsEmpty()) { Fail("No divergence expected for matching value"); return; }
	Cout() << "ValidateProp match: OK\n";

	// Event 2: OCR sees "Dashboard" → set property, then validate expects "Login" → divergence
	VsmModelRule r3 = r2;
	r3.rule_id        = "mr-003";
	r3.expected_value = "\"Login\""; // still expects Login
	rt.AddRule(r3);

	VsmModelEvent ev2;
	ev2.type           = "ocr_result";
	ev2.source_rule_id = "ocr-001";
	ev2.data_json      = "\"Dashboard\"";
	ev2.ts             = "2026-01-15T14:23:01.000Z";

	VsmModelRuntimeResult res2 = rt.ApplyEvent(ev2);
	// r2 validates "Login" == "Dashboard" → divergence; r3 same
	bool found_div = !res2.divergences.IsEmpty() || !rt.GetDivergences().IsEmpty();
	if(!found_div) { Fail("Expected divergence when property doesn't match"); return; }
	Cout() << "ValidateProp divergence: " << rt.GetDivergences().Top().message << " OK\n";

	// History check
	if(rt.GetHistory().GetCount() < 2) { Fail("Expected at least 2 history entries"); return; }
	Cout() << "History entries: " << rt.GetHistory().GetCount() << " OK\n";

	// Rule round-trip
	String json = StoreAsJson(r1);
	VsmModelRule r1_rt;
	LoadFromJson(r1_rt, json);
	if(r1_rt.rule_id != r1.rule_id) { Fail("ModelRule round-trip"); return; }
	Cout() << "ModelRule round-trip: OK\n";
}

// ---------------------------------------------------------------------------
// Test: OCR observation layer

static void TestOcrLayer()
{
	Cout() << "\n=== OCR observation layer ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	VsmFakeOcrEngine fake("Login Button", 0.97);
	VsmOcrEngineInfo info = fake.GetInfo();
	if(info.name != "FakeOCR" || !info.available) { Fail("FakeOcrEngine GetInfo"); return; }
	Cout() << "FakeOcrEngine: " << info.name << " available=" << info.available << " OK\n";

	VsmOcrExecutor exec;
	exec.SetLog(&log);
	exec.SetEngine(&fake);

	VsmOcrRequest req;
	req.rule_id   = "ocr-001";
	req.region_id = "rgn-0001";
	req.frame     = 1;
	req.status    = VSM_OCR_PENDING;

	VsmFrameImage img;
	FillSolidRGBA(img, 32, 32, 200, 200, 200);

	VsmOcrResult result = exec.RunRequest(img, req);
	if(result.text != "Login Button") { Fail("RunRequest text mismatch"); return; }
	Cout() << "RunRequest: text='" << result.text << "' confidence=" << result.confidence << " OK\n";

	// Exact match OK
	VsmOcrRule rule;
	rule.rule_id                  = "ocr-001";
	rule.expectation.mode         = VSM_EXPECT_EXACT;
	rule.expectation.expected_text= "Login Button";
	rule.confidence_threshold     = 0.5;
	VsmOcrComparison cmp = exec.Compare(result, rule);
	if(cmp.severity != VSM_OCR_OK) { Fail("Compare exact match"); return; }
	Cout() << "Compare exact match: OK\n";

	// Contains mode
	VsmOcrRule rule2 = rule;
	rule2.expectation.mode         = VSM_EXPECT_CONTAINS;
	rule2.expectation.expected_text= "Login";
	VsmOcrComparison cmp2 = exec.Compare(result, rule2);
	if(cmp2.severity != VSM_OCR_OK) { Fail("Compare contains"); return; }
	Cout() << "Compare contains: OK\n";

	// Mismatch
	VsmOcrRule rule3 = rule;
	rule3.expectation.expected_text = "Dashboard";
	VsmOcrComparison cmp3 = exec.Compare(result, rule3);
	if(cmp3.severity != VSM_OCR_WARNING) { Fail("Expected warning for mismatch"); return; }
	Cout() << "Compare mismatch → warning: OK\n";

	// Round-trip
	String json = StoreAsJson(rule);
	VsmOcrRule rule_rt;
	LoadFromJson(rule_rt, json);
	if(rule_rt.rule_id != rule.rule_id) { Fail("OcrRule round-trip"); return; }
	Cout() << "OcrRule round-trip: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Template matching

static void TestTemplateMatch()
{
	Cout() << "\n=== Template matching ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build two distinct fingerprints
	VsmFingerprint32 fp_a, fp_b;
	memset(fp_a.data, 50,  sizeof(fp_a.data));
	memset(fp_b.data, 200, sizeof(fp_b.data));

	VsmTemplateMatcher matcher;
	matcher.SetLog(&log);
	matcher.AddSyntheticAsset("asset-a", fp_a);
	matcher.AddSyntheticAsset("asset-b", fp_b);

	// Build a rule — presence mode, threshold 0.9
	VsmTemplateRule rule;
	rule.rule_id       = "rule-001";
	rule.annotation_id = "ann-001";
	rule.mode          = VSM_TM_PRESENCE;
	rule.threshold     = 0.9;
	rule.requirement   = VSM_TMR_OPTIONAL;
	VsmTemplateCandidate& ca = rule.candidates.Add();
	ca.asset_id = "asset-a"; ca.label = "LightGray";
	VsmTemplateCandidate& cb = rule.candidates.Add();
	cb.asset_id = "asset-b"; cb.label = "DarkGray";

	// Region image that matches fp_a exactly (luma~50)
	VsmFrameImage img;
	FillSolidRGBA(img, 32, 32, 50, 50, 50);

	VsmTemplateMatchResult res = matcher.Match(img, rule);
	Cout() << "Match: matched=" << (res.matched?"yes":"no")
	       << " score=" << res.score
	       << " label=" << res.matched_label << "\n";
	if(!res.matched) { Fail("Expected match for identical image"); return; }
	Cout() << "Presence match: OK\n";

	// Required-no-match failure: empty candidates
	VsmTemplateRule rule_empty;
	rule_empty.rule_id     = "rule-002";
	rule_empty.requirement = VSM_TMR_REQUIRED_ONE;
	VsmTemplateMatchResult res2 = matcher.Match(img, rule_empty);
	if(!res2.is_required_failure) { Fail("Expected required_failure for empty candidates"); return; }
	Cout() << "Required-failure: OK\n";

	// Round-trip
	String json = StoreAsJson(rule);
	VsmTemplateRule rule2;
	LoadFromJson(rule2, json);
	if(rule2.candidates.GetCount() != 2) { Fail("Rule round-trip"); return; }
	Cout() << "Rule round-trip: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Preprocessing pipeline

static void TestPreprocess()
{
	Cout() << "\n=== Preprocessing pipeline ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build a solid gray 64x64 image
	VsmFrameImage img;
	FillSolidRGBA(img, 64, 64, 180, 90, 60);

	// Pipeline: grayscale → invert → threshold → normalize 32x32
	VsmPreprocessPipeline pipeline;
	pipeline.id   = "pipe-001";
	pipeline.name = "Test pipeline";

	VsmPreprocessStep s1; s1.type = VSM_PREP_GRAYSCALE;
	VsmPreprocessStep s2; s2.type = VSM_PREP_INVERT;
	VsmPreprocessStep s3; s3.type = VSM_PREP_THRESHOLD; s3.params.threshold_value = 128;
	VsmPreprocessStep s4; s4.type = VSM_PREP_NORMALIZE_32;
	VsmPreprocessStep s5; s5.type = VSM_PREP_OTSU;       // deferred
	pipeline.steps.Add(s1);
	pipeline.steps.Add(s2);
	pipeline.steps.Add(s3);
	pipeline.steps.Add(s4);
	pipeline.steps.Add(s5);

	VsmPreprocessExecutor exec;
	exec.SetLog(&log);
	VsmFrameImage out;
	VsmPreprocessResultRef result = exec.Execute(img, pipeline, out);

	if(!result.success) { Fail("Preprocess Execute failed"); return; }
	Cout() << "Execute: " << result.steps_run << " steps run OK\n";
	if(result.warnings.IsEmpty()) { Fail("Expected warning for Otsu step"); return; }
	Cout() << "Deferred step warning: OK\n";
	if(out.width != 32 || out.height != 32) { Fail("Normalize output size wrong"); return; }
	Cout() << "Normalize 32x32: OK\n";

	// Round-trip
	String json = StoreAsJson(pipeline);
	VsmPreprocessPipeline p2;
	LoadFromJson(p2, json);
	if(p2.steps.GetCount() != pipeline.steps.GetCount()) {
		Fail("Pipeline round-trip step count");
		return;
	}
	Cout() << "Pipeline round-trip: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Annotation layer

static void TestAnnotation()
{
	Cout() << "\n=== Annotation layer ===\n";

	VsmAnnotationLayer layer;
	layer.schema     = 1;
	layer.session_id = "test-session-001";

	// Add a valid annotation
	VsmRegionAnnotation& a = layer.annotations.Add();
	a.id   = "ann-001";
	a.name = "Login Button";
	a.x = 10; a.y = 20; a.w = 80; a.h = 40;
	VsmAnchorPoint& ap = a.anchors.Add();
	ap.name = "center"; ap.x = 40; ap.y = 20;

	// Add child
	VsmRegionAnnotation& b = layer.annotations.Add();
	b.id        = "ann-002";
	b.parent_id = "ann-001";
	b.name      = "Button Label";
	b.x = 5; b.y = 10; b.w = 70; b.h = 20;
	b.relative_to_parent = true;

	// Validate clean layer
	auto errs = layer.Validate();
	if(!errs.IsEmpty()) {
		Fail("Validate clean layer: unexpected errors");
		return;
	}
	Cout() << "Validate clean layer: OK\n";

	// Save and reload
	String path = AppendFileName(GetTempPath(), "vsm_test_annotations.json");
	if(!layer.Save(path)) { Fail("Save annotations"); return; }
	VsmAnnotationLayer layer2;
	if(!layer2.Load(path)) { Fail("Load annotations"); return; }
	FileDelete(path);
	if(layer2.annotations.GetCount() != 2) { Fail("Load count"); return; }
	if(layer2.annotations[0].name != "Login Button") { Fail("Load name"); return; }
	Cout() << "Annotation round-trip: count=" << layer2.annotations.GetCount() << " OK\n";

	// Missing parent error
	VsmRegionAnnotation& c = layer2.annotations.Add();
	c.id        = "ann-003";
	c.parent_id = "non-existent-parent";
	c.name      = "Orphan";
	c.x = 1; c.y = 1; c.w = 10; c.h = 10;
	auto errs2 = layer2.Validate();
	bool found_missing = false;
	for(const auto& e : errs2)
		if(e.message.Find("Parent not found") >= 0) found_missing = true;
	if(!found_missing) { Fail("Validate missing parent"); return; }
	Cout() << "Validate missing parent: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Pipeline cache

static void TestPipelineCache()
{
	Cout() << "\n=== Pipeline cache ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	String cache_dir = AppendFileName(GetTempPath(), "vsm_cache_test");
	if(DirectoryExists(cache_dir)) DeleteFolderDeep(cache_dir);

	VsmPipelineCache cache;
	cache.SetLog(&log);

	if(!cache.Open(cache_dir)) { Fail("Cache Open"); return; }
	if(!cache.IsOpen()) { Fail("Cache IsOpen"); return; }
	Cout() << "Cache open: OK\n";

	// Build two keys
	VsmCacheKey k1;
	k1.asset_id = "frames/00000000.vsm"; k1.pipeline_id = "pipe-001";
	k1.rule_id  = "rule-001";            k1.rule_type   = "fingerprint";

	VsmCacheKey k2;
	k2.asset_id = "frames/00000001.vsm"; k2.pipeline_id = "pipe-001";
	k2.rule_id  = "rule-001";            k2.rule_type   = "fingerprint";

	// Verify different keys produce different hashes
	if(k1.Compute() == k2.Compute()) { Fail("Different keys produced same hash"); return; }
	Cout() << "Key hashing: OK\n";

	// Miss on empty cache
	String out;
	if(cache.Get(k1, out)) { Fail("Expected miss on empty cache"); return; }
	if(cache.GetMisses() != 1) { Fail("Miss counter"); return; }
	Cout() << "Miss on empty: OK\n";

	// Put then hit
	cache.Put(k1, "{\"score\":0.95}");
	if(!cache.Get(k1, out)) { Fail("Expected hit after Put"); return; }
	if(out != "{\"score\":0.95}") { Fail("Hit data mismatch"); return; }
	if(cache.GetHits() != 1) { Fail("Hit counter"); return; }
	Cout() << "Put/Get: OK (hits=" << cache.GetHits() << " misses=" << cache.GetMisses() << ")\n";

	// k2 still misses
	cache.ResetStats();
	if(cache.Get(k2, out)) { Fail("k2 should miss"); return; }
	Cout() << "Distinct key miss: OK\n";

	// Save and reload
	if(!cache.Save()) { Fail("Cache Save"); return; }
	VsmPipelineCache cache2;
	cache2.SetLog(&log);
	if(!cache2.Open(cache_dir)) { Fail("Cache2 Open"); return; }
	String out2;
	if(!cache2.Get(k1, out2)) { Fail("Cache2: expected hit after reload"); return; }
	if(out2 != "{\"score\":0.95}") { Fail("Cache2: data mismatch after reload"); return; }
	Cout() << "Save/reload: OK\n";

	// Clear
	if(!cache2.Clear()) { Fail("Cache Clear"); return; }
	if(cache2.GetCount() != 0) { Fail("Count != 0 after clear"); return; }
	Cout() << "Clear: OK\n";

	DeleteFolderDeep(cache_dir);
}

// ---------------------------------------------------------------------------
// Test: Observation pipeline runner

static void TestPipelineRunner()
{
	Cout() << "\n=== Observation pipeline runner ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build session from sample JSON
	VsmSession session;
	VsmGroundTruthLoader loader;
	loader.SetLog(&log);
	String sample = VsmMakeSampleJson();
	String tmpJson = AppendFileName(GetTempPath(), "vsm_pipe_test.json");
	SaveFile(tmpJson, sample);
	if(!loader.Load(tmpJson, session)) { Fail("PipelineRunner: load session"); return; }
	FileDelete(tmpJson);

	// Set up annotation layer matching sample region
	VsmAnnotationLayer ann;
	ann.schema = 1; ann.session_id = session.session_id;
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-001"; a.name = "Login"; a.x = 0; a.y = 0; a.w = 320; a.h = 240;

	// Set up preprocessing pipeline
	VsmPreprocessPipeline pipeline;
	pipeline.id = "pipe-001"; pipeline.name = "Test";
	VsmPreprocessStep gs; gs.type = VSM_PREP_GRAYSCALE;
	VsmPreprocessStep n32; n32.type = VSM_PREP_NORMALIZE_32;
	pipeline.steps.Add(gs); pipeline.steps.Add(n32);

	// Template matcher with synthetic asset
	VsmFingerprint32 fp; memset(fp.data, 128, sizeof(fp.data));
	VsmTemplateMatcher matcher; matcher.SetLog(&log);
	matcher.AddSyntheticAsset("asset-sample", fp);

	Vector<VsmTemplateRule> tmpl_rules;
	VsmTemplateRule& tr = tmpl_rules.Add();
	tr.rule_id = "rule-001"; tr.annotation_id = "ann-001";
	tr.mode = VSM_TM_PRESENCE; tr.threshold = 0.5; tr.requirement = VSM_TMR_OPTIONAL;
	VsmTemplateCandidate& tc = tr.candidates.Add();
	tc.asset_id = "asset-sample"; tc.label = "SampleRegion";

	// OCR: fake engine returns "Login"
	VsmFakeOcrEngine fake_ocr("Login", 0.90);
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& ocr_r = ocr_rules.Add();
	ocr_r.rule_id = "ocr-001"; ocr_r.annotation_id = "ann-001";
	ocr_r.expectation.mode = VSM_EXPECT_EXACT;
	ocr_r.expectation.expected_text = "Login";
	ocr_r.confidence_threshold = 0.5;

	// Model runtime: one rule
	VsmModelRuntime model_rt;
	model_rt.SetLog(&log);
	VsmModelRule mr;
	mr.rule_id = "mr-001"; mr.type = VSM_MR_SET_PROP_FROM_OCR;
	mr.object_id = "app-screen"; mr.property_key = "screen";
	mr.source_rule_id = "ocr-001";
	model_rt.AddRule(mr);

	// Wire up pipeline
	VsmObservationPipeline pipe;
	pipe.SetLog(&log);
	pipe.SetSession(&session);
	pipe.SetAnnotationLayer(&ann);
	pipe.SetPreprocessPipeline(&pipeline);
	pipe.SetTemplateRules(&tmpl_rules);
	pipe.SetTemplateMatcher(&matcher);
	pipe.SetOcrRules(&ocr_rules);
	pipe.SetOcrEngine(&fake_ocr);
	pipe.SetModelRuntime(&model_rt);

	VsmPipelineRunSummary summary = pipe.Run();

	if(!summary.success) { Fail("PipelineRunner: Run() not success"); return; }
	Cout() << "Run: OK\n";
	Cout() << "Frames processed: " << summary.frames_processed << "\n";
	Cout() << "Observations: " << summary.observations_made << "\n";
	Cout() << "Transitions: " << summary.transitions << "\n";

	if(summary.observations_made == 0)
		{ Fail("Expected at least 1 observation"); return; }
	Cout() << "Observations > 0: OK\n";

	if(summary.transitions == 0)
		{ Fail("Expected at least 1 model transition"); return; }
	Cout() << "Transitions > 0: OK\n";

	// Save outputs to temp dir
	String root = AppendFileName(GetTempPath(), "vsm_pipe_out");
	if(DirectoryExists(root)) DeleteFolderDeep(root);
	RealizeDirectory(root);

	if(!pipe.SaveOutputs(root, summary.run_id))
		{ Fail("SaveOutputs failed"); return; }
	String obs_path = AppendFileName(AppendFileName(AppendFileName(root, "runs"),
	                                                 summary.run_id), "observations.json");
	if(!FileExists(obs_path)) { Fail("observations.json not created"); return; }
	Cout() << "SaveOutputs: OK\n";

	DeleteFolderDeep(root);
}

// ---------------------------------------------------------------------------
// Test: Image buffer (headless pixel buffer, .vsm save/load)

static void TestImageAssets()
{
	Cout() << "\n=== Image buffer ===\n";

	// Solid gray
	VsmImageBuffer gray = VsmImageBuffer::MakeSolid(16, 16, 128, 1);
	if(gray.width != 16 || gray.height != 16 || gray.channels != 1)
		{ Fail("MakeSolid dimensions"); return; }
	if(gray.Get(0, 0) != 128 || gray.Get(15, 15) != 128)
		{ Fail("MakeSolid pixel value"); return; }
	Cout() << "MakeSolid: OK\n";

	// Gradient
	VsmImageBuffer grad = VsmImageBuffer::MakeGradient(32, 8);
	if(grad.Get(0, 0) != 0 || grad.Get(31, 0) != 255)
		{ Fail("MakeGradient edge values"); return; }
	Cout() << "MakeGradient: OK\n";

	// Checkerboard
	VsmImageBuffer cb = VsmImageBuffer::MakeCheckerboard(16, 16, 4);
	if(cb.Get(0, 0) != 255 || cb.Get(4, 0) != 0)
		{ Fail("MakeCheckerboard cells"); return; }
	Cout() << "MakeCheckerboard: OK\n";

	// Save / Load round-trip
	String path = AppendFileName(GetTempPath(), "vsm_test_img.vsm");
	if(!gray.Save(path)) { Fail("VsmImageBuffer::Save"); return; }

	VsmImageBuffer loaded;
	if(!loaded.Load(path)) { Fail("VsmImageBuffer::Load"); return; }
	FileDelete(path);

	if(loaded.width != 16 || loaded.height != 16 || loaded.channels != 1)
		{ Fail("Load dimensions mismatch"); return; }
	if(loaded.Get(7, 7) != 128)
		{ Fail("Load pixel value mismatch"); return; }
	Cout() << "Save/Load round-trip: " << loaded.Info() << " OK\n";

	// Session store image I/O
	String root = AppendFileName(GetTempPath(), "vsm_img_session_test");
	if(DirectoryExists(root)) DeleteFolderDeep(root);

	AppLog log;
	log.SetForwardToUppLog(false);
	VsmSessionStore store;
	store.SetLog(&log);
	if(!store.Create(root, "img-test-001", 32, 32)) { Fail("store Create for images"); return; }

	VsmImageBuffer frame0 = VsmImageBuffer::MakeSolid(32, 32, 200, 1);
	VsmAssetRef fr = store.SaveFrameImage(0, frame0);
	if(fr.IsEmpty()) { Fail("SaveFrameImage returned empty ref"); return; }
	if(fr.relative_path.Find(".vsm") < 0) { Fail("SaveFrameImage wrong extension"); return; }
	Cout() << "SaveFrameImage: " << fr.relative_path << " OK\n";

	VsmImageBuffer frame_loaded;
	if(!store.LoadFrameImage(0, frame_loaded)) { Fail("LoadFrameImage failed"); return; }
	if(frame_loaded.Get(0, 0) != 200) { Fail("LoadFrameImage pixel value"); return; }
	Cout() << "LoadFrameImage: " << frame_loaded.Info() << " OK\n";

	VsmImageBuffer crop0 = VsmImageBuffer::MakeSolid(8, 8, 100, 1);
	VsmAssetRef cr = store.SaveCropImage("rgn-0001", crop0);
	if(cr.IsEmpty()) { Fail("SaveCropImage returned empty ref"); return; }

	VsmImageBuffer crop_loaded;
	if(!store.LoadCropImage("rgn-0001", crop_loaded)) { Fail("LoadCropImage failed"); return; }
	if(crop_loaded.Get(0, 0) != 100) { Fail("LoadCropImage pixel value"); return; }
	Cout() << "SaveCropImage/LoadCropImage: OK\n";

	// Manifest records .vsm format
	if(!store.SaveManifest()) { Fail("SaveManifest after image"); return; }
	const VsmSessionManifest& m = store.GetManifest();
	if(m.image_format != "vsm") { Fail("Manifest image_format not updated"); return; }
	Cout() << "Manifest image_format: " << m.image_format << " OK\n";

	DeleteFolderDeep(root);
}

// ---------------------------------------------------------------------------
// Test: Session storage

static void TestSessionStorage()
{
	Cout() << "\n=== Session storage ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	String root = AppendFileName(GetTempPath(), "vsm_session_test");
	// Remove leftover from previous run
	if(DirectoryExists(root))
		DeleteFolderDeep(root);

	VsmSessionStore store;
	store.SetLog(&log);

	// Create
	if(!store.Create(root, "test-session-001", 640, 480)) {
		Fail("SessionStore Create");
		return;
	}
	Cout() << "Create: OK\n";

	// Allocate assets
	VsmAssetRef f0 = store.AllocateFrame(0);
	VsmAssetRef f1 = store.AllocateFrame(1);
	VsmAssetRef cr = store.AllocateCrop("rgn-0001");
	if(f0.IsEmpty() || f1.IsEmpty() || cr.IsEmpty()) {
		Fail("AllocateFrame/Crop returned empty ref");
		return;
	}
	Cout() << "AllocateFrame(0): " << f0.relative_path << " OK\n";
	Cout() << "AllocateCrop: " << cr.relative_path << " OK\n";

	// Resolve
	String abs = store.Resolve(f0);
	if(!FileExists(abs)) {
		Fail("Placeholder file not created");
		return;
	}
	Cout() << "Resolve + placeholder exists: OK\n";

	// Save manifest
	if(!store.SaveManifest()) { Fail("SaveManifest"); return; }

	// Manifest round-trip: open fresh store
	VsmSessionStore store2;
	store2.SetLog(&log);
	if(!store2.Open(root)) { Fail("SessionStore Open"); return; }

	const VsmSessionManifest& m = store2.GetManifest();
	if(m.session_id != "test-session-001")  { Fail("manifest session_id"); return; }
	if(m.frame_width != 640)                { Fail("manifest frame_width"); return; }
	if(m.frames.GetCount() != 2)            { Fail("manifest frames count"); return; }
	if(m.crops.GetCount() != 1)             { Fail("manifest crops count"); return; }
	Cout() << "Manifest round-trip: session_id=" << m.session_id
	       << " frames=" << m.frames.GetCount() << " crops=" << m.crops.GetCount() << " OK\n";

	// Cleanup
	DeleteFolderDeep(root);
}

// ---------------------------------------------------------------------------
// Test: VsmSessionStoreSource and RunFromSource

static void TestFrameSource()
{
	Cout() << "\n=== Frame source (VsmSessionStoreSource) ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build a session with 3 real vsm frames
	String root = AppendFileName(GetTempPath(), "vsm_framesrc_test");
	if(DirectoryExists(root)) DeleteFolderDeep(root);

	VsmSessionStore store;
	VsmSyntheticSessionOptions opts;
	opts.output_dir = root;
	opts.session_id = "framesrc-001";
	opts.frame_count = 3;
	opts.width = 32;
	opts.height = 32;
	if(!VsmBuildSyntheticSession(opts, store)) { Fail("FrameSource: store Create"); return; }

	// Open with VsmSessionStoreSource
	VsmSessionStoreSource src;
	src.SetLog(&log);
	if(!src.Open(root)) { Fail("FrameSource: Open"); return; }
	if(!src.IsReady()) { Fail("FrameSource: IsReady"); return; }
	if(src.GetWidth() != 32 || src.GetHeight() != 32) { Fail("FrameSource: dimensions"); return; }
	if(src.GetFrameCount() != 3) { Fail("FrameSource: GetFrameCount"); return; }
	Cout() << "Open: OK — " << src.GetSourceInfo() << "\n";
	Cout() << "Dimensions: " << src.GetWidth() << "x" << src.GetHeight() << " OK\n";

	// Read all frames
	VsmImageBuffer out; int64 ts_ms = 0;
	int frames_read = 0;
	while(src.ReadFrame(out, ts_ms)) {
		if(out.width != 32 || out.height != 32) { Fail("FrameSource: frame dimensions"); return; }
		frames_read++;
	}
	if(frames_read != 3) { Fail("FrameSource: expected 3 frames"); return; }
	Cout() << "ReadFrame: " << frames_read << " frames OK\n";

	// End-of-stream: ReadFrame should return false
	if(src.ReadFrame(out, ts_ms)) { Fail("FrameSource: expected end-of-stream"); return; }
	Cout() << "End-of-stream: OK\n";

	// Close / reopen
	src.Close();
	if(src.IsReady()) { Fail("FrameSource: IsReady after Close"); return; }
	if(!src.Open(root)) { Fail("FrameSource: reopen after Close"); return; }
	Cout() << "Close/reopen: OK\n";

	// Run pipeline from source
	VsmAnnotationLayer ann;
	ann.schema = 1; ann.session_id = "framesrc-001";
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-fs"; a.name = "FullFrame"; a.x = 0; a.y = 0; a.w = 32; a.h = 32;

	VsmFakeOcrEngine fake_ocr("FrameTest", 0.88);
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& r = ocr_rules.Add();
	r.rule_id = "ocr-fs"; r.annotation_id = "ann-fs";
	r.expectation.mode = VSM_EXPECT_EXACT;
	r.expectation.expected_text = "FrameTest";
	r.confidence_threshold = 0.5;

	VsmObservationPipeline pipe;
	pipe.SetLog(&log);
	pipe.SetAnnotationLayer(&ann);
	pipe.SetOcrRules(&ocr_rules);
	pipe.SetOcrEngine(&fake_ocr);

	VsmPipelineRunSummary sum = pipe.RunFromSource(src);
	if(!sum.success) { Fail("FrameSource: RunFromSource not success"); return; }
	if(sum.frames_processed != 3) { Fail("FrameSource: RunFromSource frames processed"); return; }
	if(sum.observations_made == 0) { Fail("FrameSource: RunFromSource no observations"); return; }
	Cout() << "RunFromSource: frames=" << sum.frames_processed
	       << " obs=" << sum.observations_made << " OK\n";

	DeleteFolderDeep(root);

	// Sub-test: timestamp round-trip via VsmCaptureSink → VsmSessionStoreSource
	Cout() << "Timestamp round-trip:\n";
	{
		struct SyntheticTsSource : VsmFrameSource {
			int step_ = 0;
			bool Open(const String&) override { step_ = 0; return true; }
			void Close() override { step_ = 0; }
			bool IsReady()  const override { return true; }
			int  GetWidth() const override { return 16; }
			int  GetHeight()const override { return 16; }
			int  GetFPS()   const override { return 0; }
			bool ReadFrame(VsmImageBuffer& out, int64& ts) override {
				if(step_ >= 3) return false;
				static const int64 kTs[] = { 0, 33, 100 };
				out = VsmImageBuffer::MakeSolid(16, 16, (byte)(step_ * 50 + 100), 1);
				ts  = kTs[step_++];
				return true;
			}
			String GetSourceInfo() const override { return "synthetic-ts"; }
		};

		String caproot = AppendFileName(GetTempPath(), "vsm_ts_round_trip_test");
		if(DirectoryExists(caproot)) DeleteFolderDeep(caproot);

		AppLog log2;
		log2.SetForwardToUppLog(false);

		SyntheticTsSource ssrc;
		VsmCaptureSink sink;
		sink.SetLog(&log2);
		VsmCaptureSinkOptions opts;
		opts.output_dir = caproot;
		opts.session_id = "ts-trip-001";

		VsmCaptureSummary cap = sink.Record(ssrc, opts);
		if(!cap.success || cap.frames_recorded != 3) {
			Fail("FrameSource ts: capture failed");
		} else {
			VsmSessionStoreSource replay;
			replay.SetLog(&log2);
			if(!replay.Open(caproot)) {
				Fail("FrameSource ts: replay open");
			} else {
				static const int64 kExpected[] = { 0, 33, 100 };
				int fi = 0;
				VsmImageBuffer rout; int64 rts = -1;
				bool ts_ok = true;
				while(replay.ReadFrame(rout, rts)) {
					if(fi >= 3) { Fail("FrameSource ts: too many frames"); ts_ok = false; break; }
					if(rts != kExpected[fi]) {
						Fail(Format("FrameSource ts: frame %d ts_ms=%lld expected %lld",
						            fi, (long long)rts, (long long)kExpected[fi]));
						ts_ok = false;
						break;
					}
					fi++;
				}
				if(ts_ok) {
					if(fi != 3)
						Fail("FrameSource ts: expected 3 replay frames");
					else
						Cout() << "  ts_ms round-trip: 0, 33, 100 ms OK\n";
				}
			}
		}
		DeleteFolderDeep(caproot);
	}
}

// ---------------------------------------------------------------------------
// Test: Manifest backward compatibility

static void TestManifestBackwardCompat()
{
	Cout() << "\n=== Manifest backward compatibility ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Create temp directory for old-shaped session
	String root = AppendFileName(GetTempPath(), "vsm_manifest_compat_test");
	if(DirectoryExists(root))
		DeleteFolderDeep(root);
	RealizeDirectory(root);
	String frames_dir = AppendFileName(root, "frames");
	RealizeDirectory(frames_dir);

	// Create two .vsm frame images
	VsmImageBuffer frame0 = VsmImageBuffer::MakeSolid(32, 32, 100, 1);
	VsmImageBuffer frame1 = VsmImageBuffer::MakeSolid(32, 32, 150, 1);
	String frame0_path = AppendFileName(frames_dir, "00000000.vsm");
	String frame1_path = AppendFileName(frames_dir, "00000001.vsm");
	if(!frame0.Save(frame0_path)) {
		Fail("TestManifestBackwardCompat: cannot save frame 0");
		DeleteFolderDeep(root);
		return;
	}
	if(!frame1.Save(frame1_path)) {
		Fail("TestManifestBackwardCompat: cannot save frame 1");
		DeleteFolderDeep(root);
		return;
	}

	// Hand-construct old-shaped manifest.json (no ts_ms field, no divergences array)
	// This represents the format from before task 0029 added ts_ms
	String oldManifest = R"({
	"schema": 1,
	"session_id": "compat-test-001",
	"source_type": "synthetic",
	"created_at": "2026-01-15T14:23:00.000Z",
	"frame_width": 32,
	"frame_height": 32,
	"image_format": "vsm",
	"frames": [
		{
			"frame_index": 0,
			"relative_path": "frames/00000000.vsm",
			"format": "vsm"
		},
		{
			"frame_index": 1,
			"relative_path": "frames/00000001.vsm",
			"format": "vsm"
		}
	],
	"crops": []
})";

	// Write manifest.json
	String manifest_path = AppendFileName(root, "manifest.json");
	if(!SaveFile(manifest_path, oldManifest)) {
		Fail("TestManifestBackwardCompat: cannot write manifest");
		DeleteFolderDeep(root);
		return;
	}

	// Open with VsmSessionStoreSource
	VsmSessionStoreSource src;
	src.SetLog(&log);
	if(!src.Open(root)) {
		Fail("TestManifestBackwardCompat: Open failed");
		DeleteFolderDeep(root);
		return;
	}

	// Verify frame count
	if(src.GetFrameCount() != 2) {
		Fail(Format("TestManifestBackwardCompat: expected 2 frames, got %d", src.GetFrameCount()));
		src.Close();
		DeleteFolderDeep(root);
		return;
	}
	Cout() << "Frame count: 2 OK\n";

	// Read frames and verify ts_ms fallback (frame_index * 33)
	VsmImageBuffer out;
	int64 ts_ms = 0;
	int frame_count = 0;
	static const int64 expected_ts[] = { 0, 33 };
	while(src.ReadFrame(out, ts_ms)) {
		if(frame_count >= 2) {
			Fail("TestManifestBackwardCompat: too many frames read");
			src.Close();
			DeleteFolderDeep(root);
			return;
		}
		if(ts_ms != expected_ts[frame_count]) {
			Fail(Format("TestManifestBackwardCompat: frame %d ts_ms=%lld expected %lld",
			            frame_count, (long long)ts_ms, (long long)expected_ts[frame_count]));
			src.Close();
			DeleteFolderDeep(root);
			return;
		}
		frame_count++;
	}

	if(frame_count != 2) {
		Fail(Format("TestManifestBackwardCompat: read %d frames, expected 2", frame_count));
		src.Close();
		DeleteFolderDeep(root);
		return;
	}
	Cout() << "ReadFrame ts_ms fallback (frame_index * 33): OK\n";

	// Validate with VsmSessionValidator
	VsmSessionValidator validator;
	validator.SetLog(&log);
	VsmValidationResult val_result = validator.Validate(root);
	bool has_errors = false;
	for(const auto& issue : val_result.issues) {
		if(issue.severity == "error") {
			has_errors = true;
			Fail(Format("TestManifestBackwardCompat: validation error: %s", issue.message));
			break;
		}
	}
	if(has_errors) {
		src.Close();
		DeleteFolderDeep(root);
		return;
	}
	Cout() << "SessionValidator: OK (ok=" << (val_result.ok ? "true" : "false") << ")\n";

	src.Close();
	DeleteFolderDeep(root);

	Cout() << "Manifest backward compat: OK\n";
}

// ---------------------------------------------------------------------------
// Test: MJPEG boundary parser

static void TestMjpegParser()
{
	Cout() << "\n=== MJPEG boundary parser ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build a synthetic 3-frame MJPEG stream
	String boundary = "myboundary";
	String fake_jpeg = "FAKE_JPEG_DATA_0123456789";
	String stream = VsmMakeSyntheticMjpeg(boundary, fake_jpeg, 3);
	if(stream.IsEmpty()) { Fail("VsmMakeSyntheticMjpeg returned empty"); return; }
	Cout() << "Synthetic stream: " << stream.GetCount() << " bytes OK\n";

	VsmMjpegParser parser;
	parser.SetLog(&log);
	parser.Reset(boundary);

	// Feed in one chunk
	parser.Feed(stream);

	// Extract all payloads
	int count = 0;
	String payload;
	VsmMjpegPartHeader header;
	while(parser.ExtractNextPayload(payload, header)) {
		count++;
		if(payload != fake_jpeg) { Fail("Payload content mismatch"); return; }
		if(header.content_type != "image/jpeg") { Fail("Content-Type mismatch"); return; }
		if(header.content_length != (int)fake_jpeg.GetCount())
			{ Fail("Content-Length mismatch"); return; }
	}
	if(count != 3) { Fail("Expected 3 payloads"); return; }
	Cout() << "Extracted 3 payloads, all correct OK\n";
	Cout() << "PayloadCount: " << parser.GetPayloadCount() << " OK\n";

	// Incremental feed test: split stream into 10-byte chunks
	parser.Reset(boundary);
	int ichunk = 0;
	int total = stream.GetCount();
	while(ichunk < total) {
		int n = min(10, total - ichunk);
		parser.Feed(stream.Mid(ichunk, n));
		ichunk += n;
	}
	int count2 = 0;
	while(parser.ExtractNextPayload(payload, header)) count2++;
	if(count2 != 3) { Fail("Incremental feed: expected 3 payloads"); return; }
	Cout() << "Incremental feed (10-byte chunks): " << count2 << " payloads OK\n";

	// Empty boundary reset
	parser.Reset("otherbnd");
	if(parser.GetPayloadCount() != 0) { Fail("Reset did not clear payload count"); return; }
	Cout() << "Reset: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Deterministic replay

static void TestDeterministicReplay()
{
	Cout() << "\n=== Deterministic replay ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Build a small session with 2 frames
	String root = AppendFileName(GetTempPath(), "vsm_det_replay_test");
	if(DirectoryExists(root)) DeleteFolderDeep(root);

	VsmSessionStore store;
	store.SetLog(&log);
	if(!store.Create(root, "det-001", 32, 32)) {
		Fail("DeterministicReplay: store Create");
		return;
	}

	// Add frames
	VsmImageBuffer f0 = VsmImageBuffer::MakeSolid(32, 32, 100, 1);
	VsmImageBuffer f1 = VsmImageBuffer::MakeSolid(32, 32, 150, 1);
	store.SaveFrameImage(0, f0);
	store.SaveFrameImage(1, f1);
	store.SaveManifest();

	// Shared annotation layer
	VsmAnnotationLayer ann;
	ann.schema = 1;
	ann.session_id = "det-001";
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-det";
	a.name = "TestRegion";
	a.x = 0; a.y = 0; a.w = 32; a.h = 32;

	// OCR engine that returns "ActualText"
	VsmFakeOcrEngine fake_ocr("ActualText", 0.95);

	// OCR rule expecting "ExpectedText" (mismatch produces divergence)
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& ocr_rule = ocr_rules.Add();
	ocr_rule.rule_id = "ocr-det";
	ocr_rule.annotation_id = "ann-det";
	ocr_rule.expectation.mode = VSM_EXPECT_EXACT;
	ocr_rule.expectation.expected_text = "ExpectedText";
	ocr_rule.confidence_threshold = 0.5;

	// First run: open fresh source, run pipeline, collect divergences
	String json1;
	int frames1 = 0, obs1 = 0;
	{
		VsmModelRuntime rt1;
		rt1.SetLog(&log);
		VsmModelRule mr;
		mr.rule_id = "mr-val-det";
		mr.type = VSM_MR_VALIDATE_PROP;
		mr.object_id = "app";
		mr.property_key = "screen";
		mr.expected_value = "\"ExpectedText\"";  // Validate against expected value
		mr.source_rule_id = "ocr-det";
		rt1.AddRule(mr);

		VsmSessionStoreSource src1;
		src1.SetLog(&log);
		if(!src1.Open(root)) {
			Fail("DeterministicReplay: Open run1");
			return;
		}

		VsmObservationPipeline pipe1;
		pipe1.SetLog(&log);
		pipe1.SetAnnotationLayer(&ann);
		pipe1.SetOcrRules(&ocr_rules);
		pipe1.SetOcrEngine(&fake_ocr);
		pipe1.SetModelRuntime(&rt1);

		VsmPipelineRunSummary sum1 = pipe1.RunFromSource(src1);
		if(!sum1.success) {
			Fail("DeterministicReplay: Run run1");
			return;
		}

		frames1 = sum1.frames_processed;
		obs1 = sum1.observations_made;
		json1 = StoreAsJson(rt1.GetDivergences(), true);
	}

	// Second run: fresh instances on same session directory
	String json2;
	int frames2 = 0, obs2 = 0;
	{
		VsmModelRuntime rt2;
		rt2.SetLog(&log);
		VsmModelRule mr;
		mr.rule_id = "mr-val-det";
		mr.type = VSM_MR_VALIDATE_PROP;
		mr.object_id = "app";
		mr.property_key = "screen";
		mr.expected_value = "\"ExpectedText\"";
		mr.source_rule_id = "ocr-det";
		rt2.AddRule(mr);

		VsmSessionStoreSource src2;
		src2.SetLog(&log);
		if(!src2.Open(root)) {
			Fail("DeterministicReplay: Open run2");
			return;
		}

		VsmObservationPipeline pipe2;
		pipe2.SetLog(&log);
		pipe2.SetAnnotationLayer(&ann);
		pipe2.SetOcrRules(&ocr_rules);
		pipe2.SetOcrEngine(&fake_ocr);
		pipe2.SetModelRuntime(&rt2);

		VsmPipelineRunSummary sum2 = pipe2.RunFromSource(src2);
		if(!sum2.success) {
			Fail("DeterministicReplay: Run run2");
			return;
		}

		frames2 = sum2.frames_processed;
		obs2 = sum2.observations_made;
		json2 = StoreAsJson(rt2.GetDivergences(), true);
	}

	// Verify determinism: frames and observations must match
	if(frames1 != frames2) {
		Fail(Format("DeterministicReplay: frames mismatch %d vs %d", frames1, frames2));
		return;
	}
	if(obs1 != obs2) {
		Fail(Format("DeterministicReplay: observations mismatch %d vs %d", obs1, obs2));
		return;
	}

	// Verify byte-identical JSON
	if(json1 != json2) {
		Fail("DeterministicReplay: divergence JSON mismatch");
		return;
	}

	Cout() << "Deterministic replay: OK\n";

	DeleteFolderDeep(root);
}

// ---------------------------------------------------------------------------

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	TestTypes();
	TestRegionMemory();
	TestChangeDetect();
	TestGroundTruth();
	TestReplay();
	TestModelRuntime();
	TestOcrLayer();
	TestPipelineCache();
	TestPipelineRunner();
	TestTemplateMatch();
	TestPreprocess();
	TestAnnotation();
	TestImageAssets();
	TestSessionStorage();
	TestFrameSource();
	TestManifestBackwardCompat();
	TestMjpegParser();
	TestDeterministicReplay();

	if(GetExitCode() == 0)
		Cout() << "\nAll VisualStateModel checks passed.\n";
	else
		Cout() << "\nSome checks FAILED.\n";
}
