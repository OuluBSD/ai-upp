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
	TestTemplateMatch();
	TestPreprocess();
	TestAnnotation();
	TestImageAssets();
	TestSessionStorage();

	if(GetExitCode() == 0)
		Cout() << "\nAll VisualStateModel checks passed.\n";
	else
		Cout() << "\nSome checks FAILED.\n";
}
