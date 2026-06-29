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
	TestSessionStorage();

	if(GetExitCode() == 0)
		Cout() << "\nAll VisualStateModel checks passed.\n";
	else
		Cout() << "\nSome checks FAILED.\n";
}
