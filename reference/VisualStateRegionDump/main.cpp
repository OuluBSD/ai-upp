#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
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

// Helper: Fill a rectangle with solid color
static void FillRect(VsmFrameImage& img, int rx, int ry, int rw, int rh,
                     byte r, byte g, byte b)
{
	for(int y = ry; y < ry + rh; y++) {
		for(int x = rx; x < rx + rw; x++) {
			if(x >= 0 && x < img.width && y >= 0 && y < img.height) {
				byte* p = img.data + (y * img.width + x) * 4;
				p[0] = r; p[1] = g; p[2] = b; p[3] = 255;
			}
		}
	}
}

// Helper: Print a short fingerprint hash (first 16 chars of MD5)
static String ShortHash(const VsmFingerprint32& fp)
{
	String hash = fp.ComputeHash(); // "md5:..."
	if(hash.GetLength() > 16)
		return hash.Mid(0, 16);
	return hash;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	Cout() << "=== VisualStateModel Region Fingerprint Dump ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// Create synthetic 3-frame session
	Cout() << "Building synthetic session...\n";

	// Frame 0: baseline gray (no change detection comparison, just reference)
	VsmFrameImage frame0;
	FillSolidRGBA(frame0, 320, 240, 128, 128, 128);

	// Frame 1: gray + white rectangle at position (100, 80) with size 60x50
	// This represents a "changed region" compared to frame 0
	VsmFrameImage frame1;
	FillSolidRGBA(frame1, 320, 240, 128, 128, 128);
	FillRect(frame1, 100, 80, 60, 50, 255, 255, 255);

	// Frame 2: gray + white rectangle at position (105, 85) with size 60x50
	// Same rectangle but shifted by (5, 5). Should be detected as changed but
	// should match the fingerprint from frame 1 and get the same region ID.
	VsmFrameImage frame2;
	FillSolidRGBA(frame2, 320, 240, 128, 128, 128);
	FillRect(frame2, 105, 85, 60, 50, 255, 255, 255);

	// Configure change detection
	VsmChangeDetectParams params;
	params.pixel_threshold = 30;
	params.block_size = 8;
	params.block_min_score = 0.05;
	params.merge_gap = 16;
	params.min_region_area = 64;

	Cout() << "Detecting changes between frames...\n\n";

	// Initialize region memory
	VsmRegionMemory mem;
	mem.SetLog(&log);

	// Compare frame 0 → frame 1
	Vector<VsmChangedRect> changes_0_to_1 = VsmDetectChanges(frame0, frame1, params);
	Cout() << "Frame 0→1: detected " << changes_0_to_1.GetCount() << " changed region(s)\n";

	// Map to store region assignments: region ID → (frame, rect, fingerprint)
	struct RegionInfo : Moveable<RegionInfo> {
		String region_id;
		int frame;
		VsmChangedRect rect;
		VsmFingerprint32 fingerprint;
	};
	Vector<RegionInfo> region_log;

	// Process changes from frame 0→1: these are "new" regions
	Cout() << "\nFrame 1 regions:\n";
	VsmRegionId assigned_id_1;
	for(const VsmChangedRect& cr : changes_0_to_1) {
		VsmFingerprint32 fp;
		if(!VsmRegionMemory::ExtractFingerprint(frame1, cr.x, cr.y, cr.w, cr.h, fp)) {
			Fail("ExtractFingerprint frame 1");
			return;
		}

		// Assign a new region ID (this is the "created" region)
		VsmRegionId rid = "rgn-0001";
		mem.Add(rid, fp);
		assigned_id_1 = rid;

		RegionInfo info;
		info.region_id = rid;
		info.frame = 1;
		info.rect = cr;
		info.fingerprint = fp;
		region_log.Add(info);

		Cout() << "  Frame 1: region_id=" << rid
		       << " rect=(" << cr.x << "," << cr.y << "," << cr.w << "x" << cr.h << ")"
		       << " hash=" << ShortHash(fp) << "\n";
	}

	// Compare frame 1 → frame 2
	Vector<VsmChangedRect> changes_1_to_2 = VsmDetectChanges(frame1, frame2, params);
	Cout() << "\nFrame 1→2: detected " << changes_1_to_2.GetCount() << " changed region(s)\n";

	// Process changes from frame 1→2: these should match against existing regions
	Cout() << "\nFrame 2 regions:\n";
	VsmRegionId assigned_id_2;
	bool identity_stable = false;
	for(const VsmChangedRect& cr : changes_1_to_2) {
		VsmFingerprint32 fp;
		if(!VsmRegionMemory::ExtractFingerprint(frame2, cr.x, cr.y, cr.w, cr.h, fp)) {
			Fail("ExtractFingerprint frame 2");
			return;
		}

		// Query region memory for a match
		VsmRegionMatch match = mem.FindNearest(fp, 0.3);
		VsmRegionId rid;
		if(!match.region_id.IsEmpty()) {
			// Matched existing region
			rid = match.region_id;
		} else {
			// New region
			rid = "rgn-0002";
			mem.Add(rid, fp);
		}
		assigned_id_2 = rid;

		RegionInfo info;
		info.region_id = rid;
		info.frame = 2;
		info.rect = cr;
		info.fingerprint = fp;
		region_log.Add(info);

		Cout() << "  Frame 2: region_id=" << rid
		       << " rect=(" << cr.x << "," << cr.y << "," << cr.w << "x" << cr.h << ")"
		       << " hash=" << ShortHash(fp);
		if(!match.region_id.IsEmpty()) {
			Cout() << " [matched, distance=" << DblStr(match.distance) << "]";
		}
		Cout() << "\n";

		// Check if the ID is the same as frame 1 (identity stability)
		if(rid == assigned_id_1) {
			identity_stable = true;
		}
	}

	// Print summary
	Cout() << "\n=== Region Identity Stability Check ===\n";
	Cout() << "Frame 1 region ID: " << assigned_id_1 << "\n";
	Cout() << "Frame 2 region ID: " << assigned_id_2 << "\n";
	Cout() << "Frame 1 position: (100, 80)\n";
	Cout() << "Frame 2 position: (105, 85)\n";
	Cout() << "Position shift: (5, 5)\n\n";

	if(identity_stable && assigned_id_1 == assigned_id_2) {
		Cout() << "Region identity stability: OK\n";
		Cout() << "✓ Shifted region kept the same stable ID across frames\n";
	} else {
		Cout() << "Region identity stability: FAIL\n";
		Cout() << "✗ Shifted region did NOT keep the same stable ID\n";
		Cout() << "  Frame 1 got ID: " << assigned_id_1 << "\n";
		Cout() << "  Frame 2 got ID: " << assigned_id_2 << "\n";
		SetExitCode(1);
	}

	Cout() << "\n=== Full Region Log ===\n";
	for(const RegionInfo& info : region_log) {
		Cout() << "Frame " << info.frame << ": " << info.region_id
		       << " @ (" << info.rect.x << "," << info.rect.y
		       << ") " << info.rect.w << "x" << info.rect.h
		       << " hash=" << ShortHash(info.fingerprint) << "\n";
	}

	Cout() << "\nRegion memory final count: " << mem.GetCount() << "\n";
}
