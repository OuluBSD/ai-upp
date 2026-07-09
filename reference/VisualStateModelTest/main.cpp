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
// Test: Dynamic OCR expected_text templates (task 0070, gap #4)

static void TestDynamicOcrText()
{
	Cout() << "\n=== Dynamic OCR expected_text templates ===\n";

	Value state = ParseJSON("{\"round_scores\":[1,0,7,4],\"phase\":\"PLAYING\"}");

	// Scalar + indexed-array substitution
	String resolved = VsmResolveDynamicText("Score: {round_scores[2]}, phase={phase}", state);
	if(resolved != "Score: 7, phase=PLAYING") {
		Fail(("VsmResolveDynamicText basic substitution: got '" + resolved + "'"));
		return;
	}
	Cout() << "VsmResolveDynamicText basic substitution: OK ('" << resolved << "')\n";

	// Missing field: placeholder survives, is reported via missing_fields
	Vector<String> missing;
	String resolved2 = VsmResolveDynamicText("Hand count: {hand_count}", state, &missing);
	if(resolved2 != "Hand count: {hand_count}") {
		Fail(("VsmResolveDynamicText missing field should keep placeholder, got '" + resolved2 + "'"));
		return;
	}
	if(missing.GetCount() != 1 || missing[0] != "hand_count") {
		Fail("VsmResolveDynamicText missing field should report 'hand_count' via missing_fields");
		return;
	}
	Cout() << "VsmResolveDynamicText missing field: OK (placeholder kept, reported as missing)\n";

	// Out-of-range array index also counts as missing (placeholder kept)
	Vector<String> missing3;
	String resolved3 = VsmResolveDynamicText("Score[9]: {round_scores[9]}", state, &missing3);
	if(resolved3 != "Score[9]: {round_scores[9]}" || missing3.GetCount() != 1) {
		Fail("VsmResolveDynamicText out-of-range index should keep placeholder as missing");
		return;
	}
	Cout() << "VsmResolveDynamicText out-of-range index: OK\n";

	// VsmOcrExecutor::Compare() wiring
	AppLog log;
	log.SetForwardToUppLog(false);

	VsmFakeOcrEngine fake("Score: 7, phase=PLAYING", 0.9);
	VsmOcrExecutor exec;
	exec.SetLog(&log);
	exec.SetEngine(&fake);

	VsmOcrRequest req;
	req.rule_id = "ocr-dyn-001";
	VsmFrameImage img;
	FillSolidRGBA(img, 32, 32, 200, 200, 200);
	VsmOcrResult result = exec.RunRequest(img, req);

	VsmOcrRule rule;
	rule.rule_id                   = "ocr-dyn-001";
	rule.expectation.mode          = VSM_EXPECT_DYNAMIC;
	rule.expectation.template_text = "Score: {round_scores[2]}, phase={phase}";
	rule.confidence_threshold      = 0.5;

	VsmOcrComparison cmp = exec.Compare(result, rule, state);
	if(cmp.severity != VSM_OCR_OK) { Fail("Compare dynamic match should be OK"); return; }
	Cout() << "Compare dynamic match: OK\n";

	// Mismatch: OCR text no longer matches the dynamically-resolved text
	VsmFakeOcrEngine fake_stale("Score: 4, phase=PLAYING", 0.9);
	exec.SetEngine(&fake_stale);
	VsmOcrResult stale_result = exec.RunRequest(img, req);
	VsmOcrComparison cmp2 = exec.Compare(stale_result, rule, state);
	if(cmp2.severity != VSM_OCR_WARNING) { Fail("Compare dynamic mismatch should warn"); return; }
	Cout() << "Compare dynamic mismatch → warning: OK\n";

	// No live_state supplied for a dynamic-mode rule: rule-configuration error
	VsmOcrComparison cmp3 = exec.Compare(result, rule);
	if(cmp3.severity != VSM_OCR_ERROR_S) { Fail("Compare dynamic mode without live_state should error"); return; }
	Cout() << "Compare dynamic mode without live_state → error: OK\n";
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
// Test: Stepped frame source (VsmSteppedFrameSource + RunFromSteppedSource)

static void TestSteppedFrameSource()
{
	Cout() << "\n=== Stepped frame source ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	struct SyntheticSteppedSource : VsmSteppedFrameSource {
		int step_ = 0;
		enum { kSteps = 3 };
		bool Open(const String&) override { step_ = 0; return true; }
		void Close() override { step_ = 0; }
		bool IsReady()  const override { return true; }
		int  GetWidth() const override { return 16; }
		int  GetHeight()const override { return 16; }
		int  GetFPS()   const override { return 0; }
		bool HasMoreSteps() const override { return step_ < kSteps; }
		bool Step() override {
			if(step_ >= kSteps) return false;
			step_++;
			return true;
		}
		bool ReadFrame(VsmImageBuffer& out, int64& ts) override {
			out = VsmImageBuffer::MakeSolid(16, 16, (byte)(step_ * 50 + 50), 1);
			ts  = step_ * 33;
			return true;
		}
		String GetSourceInfo() const override { return "synthetic-stepped"; }
	};

	SyntheticSteppedSource src;
	if(!src.Open("")) { Fail("SteppedSource: Open"); return; }
	if(!src.IsReady()) { Fail("SteppedSource: IsReady"); return; }
	if(!src.HasMoreSteps()) { Fail("SteppedSource: expected HasMoreSteps at start"); return; }
	Cout() << "Open/IsReady/HasMoreSteps: OK\n";

	// Drive manually: Step() then ReadFrame(), 3 times, then HasMoreSteps() false
	int steps_taken = 0;
	VsmImageBuffer out; int64 ts = -1;
	while(src.HasMoreSteps()) {
		if(!src.Step()) { Fail("SteppedSource: Step failed while HasMoreSteps true"); return; }
		if(!src.ReadFrame(out, ts)) { Fail("SteppedSource: ReadFrame failed after Step"); return; }
		steps_taken++;
	}
	if(steps_taken != 3) { Fail("SteppedSource: expected 3 steps"); return; }
	if(src.HasMoreSteps()) { Fail("SteppedSource: HasMoreSteps should be false after 3 steps"); return; }
	Cout() << "Manual Step()+ReadFrame() loop: " << steps_taken << " steps OK\n";

	// Drive via VsmObservationPipeline::RunFromSteppedSource
	SyntheticSteppedSource src2;
	src2.Open("");

	VsmAnnotationLayer ann;
	ann.schema = 1; ann.session_id = "stepped-001";
	VsmRegionAnnotation& a = ann.annotations.Add();
	a.id = "ann-stepped"; a.name = "FullFrame"; a.x = 0; a.y = 0; a.w = 16; a.h = 16;

	VsmFakeOcrEngine fake_ocr("SteppedTest", 0.85);
	Vector<VsmOcrRule> ocr_rules;
	VsmOcrRule& r = ocr_rules.Add();
	r.rule_id = "ocr-stepped"; r.annotation_id = "ann-stepped";
	r.expectation.mode = VSM_EXPECT_EXACT;
	r.expectation.expected_text = "SteppedTest";
	r.confidence_threshold = 0.5;

	VsmObservationPipeline pipe;
	pipe.SetLog(&log);
	pipe.SetAnnotationLayer(&ann);
	pipe.SetOcrRules(&ocr_rules);
	pipe.SetOcrEngine(&fake_ocr);

	VsmPipelineRunSummary sum = pipe.RunFromSteppedSource(src2);
	if(!sum.success) { Fail("RunFromSteppedSource: not success"); return; }
	if(sum.frames_processed != 3) { Fail("RunFromSteppedSource: expected 3 frames processed"); return; }
	if(sum.observations_made == 0) { Fail("RunFromSteppedSource: no observations"); return; }
	Cout() << "RunFromSteppedSource: frames=" << sum.frames_processed
	       << " obs=" << sum.observations_made << " OK\n";
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
// Test: Region cardinality field

static void TestRegionCardinality()
{
	Cout() << "\n=== Region cardinality field ===\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Test 1: VsmRegionAnnotation round-trip with expected_child_count
	{
		VsmAnnotationLayer layer;
		layer.schema = 1;
		layer.session_id = "card-001";

		VsmRegionAnnotation& a = layer.annotations.Add();
		a.id = "ann-hand";
		a.name = "Hand Zone";
		a.x = 100; a.y = 300; a.w = 500; a.h = 100;
		a.expected_child_count = 13;

		// Serialize and deserialize
		String json = StoreAsJson(layer);
		VsmAnnotationLayer layer2;
		if(!LoadFromJson(layer2, json)) {
			Fail("RegionCardinality: annotation deserialization");
			return;
		}

		if(layer2.annotations.GetCount() != 1) {
			Fail("RegionCardinality: annotation count after round-trip");
			return;
		}
		if(layer2.annotations[0].expected_child_count != 13) {
			Fail(Format("RegionCardinality: expected_child_count=%d, expected 13",
			            layer2.annotations[0].expected_child_count));
			return;
		}
		Cout() << "Annotation round-trip with expected_child_count=13: OK\n";
	}

	// Test 2: Annotation without expected_child_count defaults to -1
	{
		String json = R"({
			"schema": 1,
			"session_id": "test-001",
			"annotations": [
				{
					"id": "ann-fixed",
					"parent_id": "",
					"name": "Fixed Region",
					"x": 0,
					"y": 0,
					"w": 100,
					"h": 100,
					"relative_to_parent": false,
					"binding": {"type": 0, "reference_id": ""},
					"anchors": [],
					"hotspots": [],
					"linked_region_ids": [],
					"linked_fingerprints": []
				}
			]
		})";

		VsmAnnotationLayer layer;
		if(!LoadFromJson(layer, json)) {
			Fail("RegionCardinality: backward compat annotation load");
			return;
		}
		if(layer.annotations[0].expected_child_count != -1) {
			Fail(Format("RegionCardinality: backward compat expected_child_count=%d, expected -1",
			            layer.annotations[0].expected_child_count));
			return;
		}
		Cout() << "Annotation backward compatibility (no field defaults to -1): OK\n";
	}

	// Test 3: Ground-truth region event with expected_child_count
	{
		String sample = R"({
			"schema": 1,
			"producer": {"name": "VisualStateModel", "version": "0.1.0"},
			"session": {
				"id": "card-game-001",
				"source_type": "game_export",
				"frame_width": 1024,
				"frame_height": 768,
				"started_at": "2026-01-15T14:00:00.000Z"
			},
			"events": [
				{
					"type": "region",
					"frame": 0,
					"ts": "2026-01-15T14:00:00.000Z",
					"region_id": "rgn-hand",
					"action": "created",
					"rect": {"x": 100, "y": 500, "w": 800, "h": 200},
					"parent_id": "",
					"fingerprint": "sha1:abc123",
					"expected_child_count": 13
				},
				{
					"type": "region",
					"frame": 5,
					"ts": "2026-01-15T14:00:00.166Z",
					"region_id": "rgn-hand",
					"action": "resized",
					"rect": {"x": 100, "y": 500, "w": 800, "h": 200},
					"parent_id": "",
					"fingerprint": "sha1:abc124",
					"expected_child_count": 12
				},
				{
					"type": "region",
					"frame": 10,
					"ts": "2026-01-15T14:00:00.333Z",
					"region_id": "rgn-fixed",
					"action": "created",
					"rect": {"x": 50, "y": 50, "w": 300, "h": 300},
					"parent_id": "",
					"fingerprint": "sha1:def456"
				}
			]
		})";

		String tmpPath = AppendFileName(GetTempPath(), "vsm_card_test.json");
		SaveFile(tmpPath, sample);

		VsmGroundTruthLoader loader;
		loader.SetLog(&log);
		VsmSession session;
		if(!loader.Load(tmpPath, session)) {
			Fail("RegionCardinality: ground truth load");
			FileDelete(tmpPath);
			return;
		}
		FileDelete(tmpPath);

		if(session.regions.GetCount() != 3) {
			Fail(Format("RegionCardinality: region count=%d, expected 3", session.regions.GetCount()));
			return;
		}

		// Check first region (with expected_child_count=13)
		if(session.regions[0].expected_child_count != 13) {
			Fail(Format("RegionCardinality: rgn-hand[0] expected_child_count=%d, expected 13",
			            session.regions[0].expected_child_count));
			return;
		}

		// Check second region update (with expected_child_count=12)
		if(session.regions[1].expected_child_count != 12) {
			Fail(Format("RegionCardinality: rgn-hand[1] expected_child_count=%d, expected 12",
			            session.regions[1].expected_child_count));
			return;
		}

		// Check third region (without expected_child_count, should default to -1)
		if(session.regions[2].expected_child_count != -1) {
			Fail(Format("RegionCardinality: rgn-fixed expected_child_count=%d, expected -1",
			            session.regions[2].expected_child_count));
			return;
		}

		Cout() << "Ground-truth region events with cardinality: OK\n";
	}

	// Test 4: VsmRegionNode round-trip via Jsonize
	{
		VsmRegionNode rn;
		rn.id = "rgn-test";
		rn.parent_id = "";
		rn.x = 10; rn.y = 20; rn.w = 100; rn.h = 80;
		rn.action = "created";
		rn.label = "TestZone";
		rn.frame = 5;
		rn.ts = "2026-01-15T14:00:00.166Z";
		rn.expected_child_count = 7;

		String json = StoreAsJson(rn);
		VsmRegionNode rn2;
		if(!LoadFromJson(rn2, json)) {
			Fail("RegionCardinality: VsmRegionNode deserialization");
			return;
		}

		if(rn2.expected_child_count != 7) {
			Fail(Format("RegionCardinality: VsmRegionNode expected_child_count=%d, expected 7",
			            rn2.expected_child_count));
			return;
		}
		Cout() << "VsmRegionNode round-trip with expected_child_count=7: OK\n";
	}
}

// ---------------------------------------------------------------------------
// Test: Canonical JSON comparison

static void TestCanonicalJsonCompare()
{
	Cout() << "\n=== Canonical JSON compare ===\n";

	// Identical strings: trivially equal
	if(!VsmCanonicalJsonEqual("{\"a\":1}", "{\"a\":1}"))
		{ Fail("Identical strings should be equal"); return; }
	Cout() << "Identical strings: OK\n";

	// Reordered keys: equal
	if(!VsmCanonicalJsonEqual("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}"))
		{ Fail("Reordered keys should be equal"); return; }
	Cout() << "Reordered keys: OK\n";

	// Whitespace differences: equal
	if(!VsmCanonicalJsonEqual("{\"a\":1,\"b\":2}", "{ \"a\" : 1 , \"b\" : 2 }"))
		{ Fail("Whitespace differences should be equal"); return; }
	Cout() << "Whitespace differences: OK\n";

	// Nested objects with reordered keys: equal
	if(!VsmCanonicalJsonEqual(
		"{\"outer\":{\"x\":1,\"y\":2},\"z\":3}",
		"{\"z\":3,\"outer\":{\"y\":2,\"x\":1}}"))
		{ Fail("Nested reordered keys should be equal"); return; }
	Cout() << "Nested reordered keys: OK\n";

	// Arrays: order matters
	if(VsmCanonicalJsonEqual("{\"a\":[1,2,3]}", "{\"a\":[3,2,1]}"))
		{ Fail("Reordered array elements should not be equal"); return; }
	Cout() << "Array order matters: OK\n";

	// Arrays: same order equal
	if(!VsmCanonicalJsonEqual("{\"a\":[1,2,3]}", "{\"a\":[1,2,3]}"))
		{ Fail("Same-order arrays should be equal"); return; }
	Cout() << "Array same order: OK\n";

	// Differing values: not equal
	if(VsmCanonicalJsonEqual("{\"a\":1}", "{\"a\":2}"))
		{ Fail("Differing values should not be equal"); return; }
	Cout() << "Differing values: OK\n";

	// Differing key sets: not equal
	if(VsmCanonicalJsonEqual("{\"a\":1}", "{\"a\":1,\"b\":2}"))
		{ Fail("Differing key sets should not be equal"); return; }
	Cout() << "Differing key sets: OK\n";

	// Unparsable input: false, not a crash
	if(VsmCanonicalJsonEqual("not json", "{\"a\":1}"))
		{ Fail("Unparsable input should not be equal"); return; }
	Cout() << "Unparsable input: OK\n";

	// GroundTruthComparison uses canonical compare: a reordered expected_json
	// must still match, which would fail under raw string equality.
	VsmSession expected;
	expected.schema = 1;
	VsmDivergence& exp_div = expected.divergences.Add();
	exp_div.frame = 10;
	exp_div.expected_json = "{\"screen\":\"Dashboard\",\"error_visible\":false}";

	Vector<VsmDivergence> observed;
	VsmDivergence& obs_div = observed.Add();
	obs_div.frame = 11; // within +/-5 frames
	obs_div.expected_json = "{\"error_visible\":false,\"screen\":\"Dashboard\"}";

	VsmGroundTruthComparison cmp_engine;
	VsmComparisonResult result = cmp_engine.Compare(expected, observed);
	if(result.matched != 1 || result.missing != 0)
		{ Fail("GroundTruthComparison should match reordered expected_json"); return; }
	Cout() << "GroundTruthComparison canonical match: OK\n";
}

// ---------------------------------------------------------------------------
// Test: Card-game ground-truth self-consistency validator (task 0072)

static String CgcCardPlay(int round_number, const char* phase, int turn, int trick_number,
                           const char* leading_suit, bool hearts_broken, int player, const char* card)
{
	ValueMap v;
	v.Add("tier", "card_play");
	v.Add("round_number", round_number);
	v.Add("phase", phase);
	v.Add("turn", turn);
	v.Add("trick_number", trick_number);
	v.Add("leading_suit", leading_suit);
	v.Add("hearts_broken", hearts_broken);
	v.Add("player", player);
	v.Add("card_played", card);
	return AsJSON(v);
}

static String CgcTrick(int round_number, int trick_number, int trick_winner, int trick_points,
                        int s0, int s1, int s2, int s3)
{
	ValueMap v;
	v.Add("tier", "trick");
	v.Add("round_number", round_number);
	v.Add("trick_number", trick_number);
	v.Add("trick_winner", trick_winner);
	v.Add("trick_points", trick_points);
	ValueArray scores;
	scores.Add(s0); scores.Add(s1); scores.Add(s2); scores.Add(s3);
	v.Add("round_scores", scores);
	return AsJSON(v);
}

static String CgcRound(int round_number, int s0, int s1, int s2, int s3,
                        int moon_shooter, bool game_over)
{
	ValueMap v;
	v.Add("tier", "round");
	v.Add("round_number", round_number);
	ValueArray round_scores;
	round_scores.Add(s0); round_scores.Add(s1); round_scores.Add(s2); round_scores.Add(s3);
	v.Add("round_scores", round_scores);
	ValueArray scores; // cumulative game score -- same as round_scores for a fresh game in these tests
	scores.Add(s0); scores.Add(s1); scores.Add(s2); scores.Add(s3);
	v.Add("scores", scores);
	v.Add("moon_shooter", moon_shooter);
	v.Add("game_over", game_over);
	return AsJSON(v);
}

// Builds a small, valid 2-trick synthetic round:
//   trick 1: players 0,1,2,3 play 2C,3C,4C,5C -- player 2 wins, 5 points.
//   trick 2: players 0,1,2,3 play 6C,7C,8C,9C -- player 0 wins, 3 points.
//   round:   round_scores [3,0,5,0], no moon shooter.
static void BuildValidTwoTrickSequence(Vector<VsmCardGameEvent>& events)
{
	events.Clear();
	auto Add = [&](const char* tier, const String& json) {
		VsmCardGameEvent& e = events.Add();
		e.tier = tier;
		e.state_json = json;
	};

	Add("card_play", CgcCardPlay(1, "PLAYING", 1, 1, "clubs", false, 0, "2C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 2, 1, "clubs", false, 1, "3C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 3, 1, "clubs", false, 2, "4C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 2, 1, "clubs", false, 3, "5C"));
	Add("trick",     CgcTrick(1, 1, 2, 5, 0, 0, 5, 0));

	Add("card_play", CgcCardPlay(1, "PLAYING", 3, 2, "clubs", false, 2, "6C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 0, 2, "clubs", false, 3, "7C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 1, 2, "clubs", false, 0, "8C"));
	Add("card_play", CgcCardPlay(1, "PLAYING", 0, 2, "clubs", false, 1, "9C"));
	Add("trick",     CgcTrick(1, 2, 0, 3, 3, 0, 5, 0));

	Add("round",     CgcRound(1, 3, 0, 5, 0, -1, false));
}

static void TestCardGameConsistency()
{
	Cout() << "\n=== Card-game ground-truth self-consistency validator ===\n";

	// --- Valid sequence: all four checks pass ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(!result.ok) {
			Fail("CardGameConsistency: valid sequence should pass");
			for(const VsmValidationIssue& issue : result.issues)
				Cout() << "  [" << issue.severity << "] " << issue.message << "\n";
			return;
		}
		if(result.issues.GetCount() != 0) {
			Fail(Format("CardGameConsistency: valid sequence should have zero issues, got %d",
			            result.issues.GetCount()));
			return;
		}
		Cout() << "Valid 2-trick sequence: OK (no issues)\n";
	}

	// --- Broken variant 1: duplicate card ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);
		// Trick 2's first card_play ("6C") is index 5; overwrite with "2C",
		// a duplicate of trick 1's very first card.
		events[5].state_json = CgcCardPlay(1, "PLAYING", 3, 2, "clubs", false, 2, "2C");

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(result.ok) {
			Fail("CardGameConsistency: duplicate card should fail");
			return;
		}
		bool found = false;
		for(const VsmValidationIssue& issue : result.issues)
			if(issue.severity == "error" && issue.message.Find("duplicate card_played") >= 0)
				found = true;
		if(!found) {
			Fail("CardGameConsistency: duplicate card should report a 'duplicate card_played' error");
			return;
		}
		Cout() << "Duplicate card detected: OK\n";
	}

	// --- Broken variant 2: wrong round_scores delta on a trick event ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);
		// Trick 2's round_scores should be [3,0,5,0] (player 0 credited 3);
		// corrupt it to [4,0,5,0] -- wrong delta for the credited winner.
		events[9].state_json = CgcTrick(1, 2, 0, 3, 4, 0, 5, 0);
		// Keep the round event consistent with what the (broken) trick
		// reconciliation will actually accumulate (player 2's trick 1 win
		// only, since trick 2's row fails to reconcile and is not
		// credited) so this variant isolates the single broken invariant.
		events[10].state_json = CgcRound(1, 0, 0, 5, 0, -1, false);

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(result.ok) {
			Fail("CardGameConsistency: wrong round_scores delta should fail");
			return;
		}
		bool found = false;
		for(const VsmValidationIssue& issue : result.issues)
			if(issue.severity == "error" && issue.message.Find("changed by") >= 0)
				found = true;
		if(!found) {
			Fail("CardGameConsistency: wrong round_scores delta should report a round_scores delta error");
			return;
		}
		Cout() << "Wrong trick round_scores delta detected: OK\n";
	}

	// --- Broken variant 3: wrong round total ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);
		// Final round event should be [3,0,5,0]; corrupt to all-zero.
		events[10].state_json = CgcRound(1, 0, 0, 0, 0, -1, false);

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(result.ok) {
			Fail("CardGameConsistency: wrong round total should fail");
			return;
		}
		bool found = false;
		for(const VsmValidationIssue& issue : result.issues)
			if(issue.severity == "error" && issue.message.Find("(round): round_scores[") >= 0)
				found = true;
		if(!found) {
			Fail("CardGameConsistency: wrong round total should report a round-total error");
			return;
		}
		Cout() << "Wrong round total detected: OK\n";
	}

	// --- Broken variant 4: wrong card count ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);
		// Drop one card_play event (trick 2's last play) -- 7 card_play
		// events for 2 resolved tricks instead of the expected 8.
		events.Remove(8);

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(result.ok) {
			Fail("CardGameConsistency: wrong card count should fail");
			return;
		}
		bool found = false;
		for(const VsmValidationIssue& issue : result.issues)
			if(issue.severity == "error" && issue.message.Find("card_play event count") >= 0)
				found = true;
		if(!found) {
			Fail("CardGameConsistency: wrong card count should report a card_play event count error");
			return;
		}
		Cout() << "Wrong card count detected: OK\n";
	}

	// --- Informational note: position-based vs field-based trick_number
	// mismatch (the documented CARD_GAME_ADAPTER.md limitation) is flagged
	// as "info", not treated as a validation failure. ---
	{
		Vector<VsmCardGameEvent> events;
		BuildValidTwoTrickSequence(events);
		// Trick 2's own trick_number field wrongly still says 1 (as if the
		// adapter's same-winner-twice-in-a-row undercount had struck) --
		// the checker must still validate reconciliation by position and
		// must not fail outright over the field mismatch.
		events[9].state_json = CgcTrick(1, /*trick_number field*/ 1, 0, 3, 3, 0, 5, 0);

		VsmValidationResult result = VsmCheckCardGameConsistency(events);
		if(!result.ok) {
			Fail("CardGameConsistency: trick_number field/position mismatch alone should not fail validation");
			for(const VsmValidationIssue& issue : result.issues)
				Cout() << "  [" << issue.severity << "] " << issue.message << "\n";
			return;
		}
		bool found_info = false;
		for(const VsmValidationIssue& issue : result.issues)
			if(issue.severity == "info" && issue.message.Find("known limitation") >= 0)
				found_info = true;
		if(!found_info) {
			Fail("CardGameConsistency: trick_number field/position mismatch should report an 'info' note");
			return;
		}
		Cout() << "trick_number field/position mismatch: flagged as info, validation still OK\n";
	}
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
// Test: PNG frame decode (task 0103 — headless Draw-gated bridge into
// VsmFrameImage) against the persistent M03 fixture session, if present.

static void TestPngFrameDecode()
{
	Cout() << "\n=== PNG frame decode (M03 bridge) ===\n";

	String fixture_root = "var/vsm_fixtures/texas_ps6p_sample";
	String frame_path = AppendFileName(AppendFileName(fixture_root, "frames"), "00000000.png");

	if(!FileExists(frame_path)) {
		Cout() << "SKIP: fixture not found at " << frame_path
		       << " (regenerate with the M03 fixture command in "
		          "Manager/2-plan/ai-upp/root/VisualStateModel/docs/TEXAS_HOLDEM_SOURCE_CONTRACT.md)\n";
		return;
	}

	VsmFrameImage img;
	if(!VsmLoadPngFrame(frame_path, img)) {
		Fail("VsmLoadPngFrame: decode failed");
		return;
	}
	Cout() << "VsmLoadPngFrame: " << frame_path << " -> "
	       << img.width << "x" << img.height << "\n";
	if(img.IsEmpty()) { Fail("VsmLoadPngFrame: empty result"); return; }
	byte r, g, b, a;
	img.GetPixel(0, 0, r, g, b, a); // sanity: must not crash / read garbage pointer
	img.GetPixel(img.width - 1, img.height - 1, r, g, b, a);
	Cout() << "VsmLoadPngFrame: non-empty RGBA data OK\n";

	VsmM01M02SessionInfo info;
	if(!VsmReadM01M02SessionInfo(fixture_root, info)) {
		Fail("VsmReadM01M02SessionInfo: failed to read metadata.json");
		return;
	}
	Cout() << "Session info: provider=" << info.provider
	       << " size=" << info.table_width << "x" << info.table_height
	       << " frame_count=" << info.frame_count << "\n";

	// NOTE: the decoded PNG's pixel height does not necessarily equal
	// metadata.json's table_height. This is a pre-existing, previously
	// documented quirk of GameTable's M02 recorder (task 0099 observed
	// "window [0, 0] - [1024, 648] : (1024, 648) size=(1024, 625)" —
	// GameTable::DumpSnapshot() rasterizes GetSize() *after* Layout(),
	// which can differ from the SetRect() size recorded as table_height/
	// table_width in metadata.json). It is not a defect in this PNG bridge
	// and is out of scope for task 0103 to fix (game/TexasHoldem is not
	// touched by this task). Width matching is still asserted since it has
	// been observed stable; height is reported, not asserted.
	if(img.width != info.table_width) {
		Fail("VsmLoadPngFrame: decoded width does not match session metadata");
		return;
	}
	if(img.height != info.table_height) {
		Cout() << "NOTE: decoded height (" << img.height << ") != metadata table_height ("
		       << info.table_height << ") — known GameTable recorder quirk, not a bridge defect\n";
	}
	else {
		Cout() << "Decoded size matches session metadata: OK\n";
	}

	VsmFrameImage img2;
	if(!VsmLoadM01M02SessionFrame(fixture_root, 0, img2)) {
		Fail("VsmLoadM01M02SessionFrame: failed for frame 0");
		return;
	}
	if(img2.width != img.width || img2.height != img.height) {
		Fail("VsmLoadM01M02SessionFrame: dimensions differ from direct decode");
		return;
	}
	Cout() << "VsmLoadM01M02SessionFrame(frame 0): " << img2.width << "x" << img2.height << " OK\n";
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
	TestDynamicOcrText();
	TestPipelineCache();
	TestPipelineRunner();
	TestTemplateMatch();
	TestPreprocess();
	TestAnnotation();
	TestImageAssets();
	TestSessionStorage();
	TestFrameSource();
	TestSteppedFrameSource();
	TestManifestBackwardCompat();
	TestMjpegParser();
	TestRegionCardinality();
	TestCanonicalJsonCompare();
	TestCardGameConsistency();
	TestDeterministicReplay();
	TestPngFrameDecode();

	if(GetExitCode() == 0)
		Cout() << "\nAll VisualStateModel checks passed.\n";
	else
		Cout() << "\nSome checks FAILED.\n";
}
