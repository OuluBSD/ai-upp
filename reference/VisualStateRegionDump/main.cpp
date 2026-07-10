#include <VisualStateModel/VisualStateModel.h>
#include <plugin/png/png.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Headless overlay-PNG rendering (task 0105): renders a decoded M01/M02 frame
// plus its detected changed-region rectangles into a PNG file, reusing the
// exact same VsmDrawRegionOverlay drawing logic FrameCanvas uses in the GUI
// Workbench (uppsrc/VisualStateModel/RegionOverlay.h) — no second copy of the
// overlay-drawing code exists in this tool. Rendering itself is done with
// SImageDraw (uppsrc/Draw/SDraw.h), which is part of the Draw package (not
// CtrlCore's platform-native ImageDraw), so this stays headless-linkable.

// VsmFrameImageToImage()/kCropPadding/VsmSaveRegionCropPng() (task 0110) were
// extracted into uppsrc/VisualStateModel/FrameCrop.h/.cpp as of task 0116 so
// VisualStateLayoutAssign can reuse them too — see that header for the RGBA
// -> Image conversion and crop/pad geometry, both unchanged by the move.

static bool VsmSaveOverlayPng(const VsmFrameImage& frame, const Vector<VsmChangedRect>& regions,
                              const String& path)
{
	if(frame.IsEmpty())
		return false;

	int w = frame.width, h = frame.height;
	Image frame_img = VsmFrameImageToImage(frame);

	SImageDraw sw(w, h);
	sw.DrawImage(0, 0, frame_img);
	VsmDrawRegionOverlay(sw, regions, Point(0, 0)); // no selection concept headlessly
	Image out = sw;

	RealizeDirectory(GetFileFolder(path));
	return PNGEncoder().SaveFile(path, out);
}

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

	const Vector<String>& args = CommandLine();
	String session_dir;
	int frame_start = -1; // sentinel: unset -> default to 0
	int frame_end   = -1; // sentinel: unset -> default to frame_count - 1
	String jsonl_out;
	String overlay_out;
	String crop_report_out;

	// Parse command-line arguments
	for(int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if(arg == "--help") {
			Cout() << "Usage: VisualStateRegionDump [<m01m02_session_dir>] "
			          "[--frame-start N] [--frame-end M] [--jsonl-out <path>] "
			          "[--overlay-out <dir>] [--crop-report-out <dir>]\n"
			          "  <m01m02_session_dir>  M01/M02 session directory (metadata.json +\n"
			          "                        groundtruth.jsonl + frames/%08d.png), e.g.\n"
			          "                        var/vsm_fixtures/texas_ps6p_sample\n"
			          "  --frame-start N       restrict to transitions ending at frame >= N (default 0)\n"
			          "  --frame-end M         restrict to transitions ending at frame <= M (default last frame)\n"
			          "  --jsonl-out <path>    write one JSON region record per line to <path>\n"
			          "  --overlay-out <dir>   write one overlay PNG per transition that has >=1\n"
			          "                        detected changed region (frame pixels + region\n"
			          "                        rectangles drawn via the shared VsmDrawRegionOverlay\n"
			          "                        helper), named overlay_<prev>_<curr>.png\n"
			          "  --crop-report-out <dir>  for each transition with >=1 changed region,\n"
			          "                        write one small cropped PNG per region\n"
			          "                        (crop_<prev>_<curr>_<idx>.png, just the region rect\n"
			          "                        + padding, not the whole frame) plus one markdown\n"
			          "                        file (report_<prev>_<curr>.md) embedding the crop(s)\n"
			          "                        and a data table (region_id, x, y, w, h, score,\n"
			          "                        frame_prev, frame). Composable with --overlay-out.\n"
			          "  (no session dir)      run the synthetic self-test path instead\n";
			SetExitCode(0);
			return;
		}
		else if(arg == "--frame-start") {
			if(i + 1 >= args.GetCount()) { Fail("--frame-start requires a value"); return; }
			frame_start = ScanInt(args[++i]);
		}
		else if(arg == "--frame-end") {
			if(i + 1 >= args.GetCount()) { Fail("--frame-end requires a value"); return; }
			frame_end = ScanInt(args[++i]);
		}
		else if(arg == "--jsonl-out") {
			if(i + 1 >= args.GetCount()) { Fail("--jsonl-out requires a value"); return; }
			jsonl_out = args[++i];
		}
		else if(arg == "--overlay-out") {
			if(i + 1 >= args.GetCount()) { Fail("--overlay-out requires a value"); return; }
			overlay_out = args[++i];
		}
		else if(arg == "--crop-report-out") {
			if(i + 1 >= args.GetCount()) { Fail("--crop-report-out requires a value"); return; }
			crop_report_out = args[++i];
		}
		else {
			session_dir = arg;
		}
	}

	Cout() << "=== VisualStateModel Region Fingerprint Dump ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	if(session_dir.IsEmpty()) {
		// ========== SYNTHETIC SESSION PATH ==========
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

		// Map to store region assignments: region ID → (frame, rect, fingerprint)
		struct RegionInfo : Moveable<RegionInfo> {
			String region_id;
			int frame;
			VsmChangedRect rect;
			VsmFingerprint32 fingerprint;
		};
		Vector<RegionInfo> region_log;

		// Compare frame 0 → frame 1
		Vector<VsmChangedRect> changes_0_to_1 = VsmDetectChanges(frame0, frame1, params);
		Cout() << "Frame 0→1: detected " << changes_0_to_1.GetCount() << " changed region(s)\n";

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

	} else {
		// ========== M01/M02 SESSION PATH ==========
		// Supersedes the old VsmSessionStoreSource/.vsm real-session path (task 0104):
		// M01/M02 sessions (game/TexasHoldem/TexasHoldemSessionContract) are
		// metadata.json + groundtruth.jsonl + frames/%08d.png, not the OLD
		// VsmSessionStore .vsm binary format. Loaded via the 0103 PNG bridge
		// (VsmReadM01M02SessionInfo / VsmLoadM01M02SessionFrame).
		Cout() << "Loading M01/M02 session from: " << session_dir << "\n";

		if(!DirectoryExists(session_dir)) {
			Fail(Format("Session directory not found: %s", session_dir));
			return;
		}

		VsmM01M02SessionInfo info;
		if(!VsmReadM01M02SessionInfo(session_dir, info)) {
			Fail(Format("Failed to read M01/M02 session metadata.json under: %s", session_dir));
			return;
		}

		Cout() << "Session: provider=" << info.provider
		       << " session_id=" << info.session_id
		       << " size=" << info.table_width << "x" << info.table_height
		       << " frame_count=" << info.frame_count << "\n\n";

		if(info.frame_count < 2) {
			Fail("Session has fewer than 2 frames — no transitions to detect");
			return;
		}

		// Resolve --frame-start/--frame-end against the session's frame count.
		// frame_start/frame_end restrict which *target* frame ids (the "curr"
		// side of a prev->curr transition) are processed, supporting
		// MILESTONE_03's "focused reruns" on a frame range.
		int fs = frame_start >= 0 ? frame_start : 0;
		int fe = frame_end   >= 0 ? frame_end   : info.frame_count - 1;
		if(fs < 1) fs = 1; // frame 0 has no predecessor; first possible transition target is 1
		if(fe > info.frame_count - 1) fe = info.frame_count - 1;
		if(fs > fe) {
			Fail(Format("Invalid frame range: --frame-start resolves to %d > --frame-end %d", fs, fe));
			return;
		}
		int load_start = fs - 1;

		Cout() << "Frame range: transitions " << load_start << "->" << fs
		       << " .. " << (fe - 1) << "->" << fe << "\n\n";

		// Configure change detection (same parameters as the synthetic path)
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

		// Deterministic per-region-per-transition output record.
		struct VsmRegionRecordOut : Moveable<VsmRegionRecordOut> {
			int    frame_prev = -1;
			int    frame      = -1;
			int    x = 0, y = 0, w = 0, h = 0;
			double score      = 0.0;
			String region_id;

			void Jsonize(JsonIO& json)
			{
				json("frame_prev", frame_prev)
				    ("frame",      frame)
				    ("x", x)("y", y)("w", w)("h", h)
				    ("score",      score)
				    ("region_id",  region_id);
			}
		};
		Vector<VsmRegionRecordOut> records;
		int rgn_counter = 0;

		VsmFrameImage prev_frame;
		if(!VsmLoadM01M02SessionFrame(session_dir, load_start, prev_frame)) {
			Fail(Format("Failed to decode frame %d", load_start));
			return;
		}

		for(int fid = fs; fid <= fe; fid++) {
			VsmFrameImage curr_frame;
			if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr_frame)) {
				Fail(Format("Failed to decode frame %d", fid));
				return;
			}

			Vector<VsmChangedRect> changes = VsmDetectChanges(prev_frame, curr_frame, params);
			Cout() << "Frame " << (fid - 1) << "->" << fid
			       << ": detected " << changes.GetCount() << " changed region(s)\n";

			// This transition's records only (same fields/order as `changes`),
			// used below to build the --crop-report-out markdown table without
			// re-deriving region_id/score from `records` (the whole-run vector).
			Vector<VsmRegionRecordOut> transition_records;

			for(const VsmChangedRect& cr : changes) {
				VsmFingerprint32 fp;
				if(!VsmRegionMemory::ExtractFingerprint(curr_frame, cr.x, cr.y, cr.w, cr.h, fp)) {
					Fail(Format("ExtractFingerprint frame %d", fid));
					return;
				}

				// Query region memory for a match
				VsmRegionMatch match = mem.FindNearest(fp, 0.3);
				VsmRegionId rid;
				if(!match.region_id.IsEmpty()) {
					rid = match.region_id;
				} else {
					rid = Format("rgn-%04d", ++rgn_counter);
					mem.Add(rid, fp);
				}

				VsmRegionRecordOut rec;
				rec.frame_prev = fid - 1;
				rec.frame      = fid;
				rec.x = cr.x; rec.y = cr.y; rec.w = cr.w; rec.h = cr.h;
				rec.score      = cr.score;
				rec.region_id  = rid;
				records.Add(rec);
				transition_records.Add(rec);

				Cout() << "  Frame " << fid << ": region_id=" << rid
				       << " rect=(" << cr.x << "," << cr.y << "," << cr.w << "x" << cr.h << ")"
				       << " score=" << DblStr(cr.score)
				       << " hash=" << ShortHash(fp);
				if(!match.region_id.IsEmpty()) {
					Cout() << " [matched, distance=" << DblStr(match.distance) << "]";
				}
				Cout() << "\n";
			}
			Cout() << "\n";

			if(!overlay_out.IsEmpty() && changes.GetCount() > 0) {
				String out_path = AppendFileName(overlay_out,
				    Format("overlay_%04d_%04d.png", fid - 1, fid));
				if(VsmSaveOverlayPng(curr_frame, changes, out_path))
					Cout() << "Wrote overlay PNG: " << out_path << "\n\n";
				else {
					Fail(Format("Failed to write overlay PNG: %s", out_path));
					return;
				}
			}

			// --crop-report-out (task 0110): per changed-region debug crop PNGs
			// + one markdown file per transition embedding them and a data table
			// mirroring the --jsonl-out record fields. Independent of and
			// composable with --overlay-out (full-frame) above.
			if(!crop_report_out.IsEmpty() && changes.GetCount() > 0) {
				Vector<String> crop_names;
				for(int i = 0; i < changes.GetCount(); i++) {
					String crop_name = Format("crop_%04d_%04d_%02d.png", fid - 1, fid, i);
					String crop_path = AppendFileName(crop_report_out, crop_name);
					if(!VsmSaveRegionCropPng(curr_frame, changes[i], crop_path)) {
						Fail(Format("Failed to write crop PNG: %s", crop_path));
						return;
					}
					crop_names.Add(crop_name);
				}

				String md;
				md << "# Frame transition " << Format("%04d", fid - 1)
				   << " -> " << Format("%04d", fid) << "\n\n";
				md << changes.GetCount() << " changed region(s) detected.\n\n";
				for(int i = 0; i < changes.GetCount(); i++) {
					const VsmRegionRecordOut& r = transition_records[i];
					md << "## Region " << r.region_id << "\n\n";
					md << "![" << r.region_id << "](" << crop_names[i] << ")\n\n";
				}
				md << "## Region Data\n\n";
				md << "| region_id | x | y | w | h | score | frame_prev | frame |\n";
				md << "| --- | --- | --- | --- | --- | --- | --- | --- |\n";
				for(int i = 0; i < changes.GetCount(); i++) {
					const VsmRegionRecordOut& r = transition_records[i];
					md << "| " << r.region_id
					   << " | " << r.x << " | " << r.y << " | " << r.w << " | " << r.h
					   << " | " << DblStr(r.score)
					   << " | " << r.frame_prev << " | " << r.frame << " |\n";
				}

				String md_path = AppendFileName(crop_report_out,
				    Format("report_%04d_%04d.md", fid - 1, fid));
				RealizeDirectory(GetFileFolder(md_path));
				if(!SaveFile(md_path, md)) {
					Fail(Format("Failed to write --crop-report-out markdown file: %s", md_path));
					return;
				}
				Cout() << "Wrote crop report: " << md_path
				       << " (" << changes.GetCount() << " crop PNG(s))\n\n";
			}

			// Copy curr_frame data to prev_frame for next iteration (VsmFrameImage
			// wraps a non-assignable Buffer<byte>, so copy the raw bytes instead).
			if(prev_frame.width != curr_frame.width || prev_frame.height != curr_frame.height) {
				prev_frame.Set(curr_frame.width, curr_frame.height, nullptr);
			}
			memcpy(prev_frame.data, curr_frame.data, (size_t)curr_frame.width * curr_frame.height * 4);
		}

		Cout() << "\n=== Region Detection Summary ===\n";
		Cout() << "Total distinct regions: " << mem.GetCount() << "\n";
		Cout() << "Total region records: " << records.GetCount() << "\n";
		Cout() << "Transitions processed: " << load_start << "->" << fs
		       << " .. " << (fe - 1) << "->" << fe << "\n";

		// Emit deterministic JSON/JSONL region records: one JSON object per
		// changed region per frame transition (frame id, rect, score, stable
		// region id), suitable for regression diffing.
		if(!jsonl_out.IsEmpty()) {
			String jsonl;
			for(const VsmRegionRecordOut& r : records)
				jsonl << StoreAsJson(r) << "\n";
			if(!SaveFile(jsonl_out, jsonl)) {
				Fail(Format("Failed to write --jsonl-out file: %s", jsonl_out));
				return;
			}
			Cout() << "\nWrote " << records.GetCount() << " region record(s) to " << jsonl_out << "\n";
		}
		else {
			Cout() << "\n=== JSONL region records (stdout; use --jsonl-out <path> to save) ===\n";
			for(const VsmRegionRecordOut& r : records)
				Cout() << StoreAsJson(r) << "\n";
		}
	}
}
