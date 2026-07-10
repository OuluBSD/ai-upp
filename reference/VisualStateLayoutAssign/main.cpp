#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// Task 0115 (M04-04): region-to-element assignment CLI.
//
// Combines two already-built M04 pieces:
//   - M03's changed-region detection (VsmDetectChanges/VsmRegionMemory, same
//     M01/M02 session-loading path as reference/VisualStateRegionDump) —
//     reused as library calls, not re-implemented or shelled out to.
//   - Task 0114's VsmLayoutProfile (uppsrc/VisualStateModel/LayoutProfile.h),
//     built directly from a parsed `.form` file via VsmBuildLayoutProfile()
//     (not by parsing FormLayoutDump's stdout — see 0114's "Status of this
//     spec" note: that demo CLI's --json output is not a clean JSON document,
//     it's plain-text dump then JSON on the same stream).
//
// For each detected changed region, finds the best-overlapping layout
// element/sub-slot (by intersection-area-over-region-area, see rationale
// below) and emits one "layout observation" JSONL record per region.
//
// IMPORTANT SCALING NOTE (see this task's spec, "Status of this spec"
// section): VsmLayoutProfile's rects are in the .form file's own
// design-space (profile.width/height). A recorded session's ACTUAL decoded
// frame size is not guaranteed to match that (task 0103 already found a
// pre-existing recorder quirk: GameTable_PS_6p.form declares 1024x648, but
// the M01/M02 sample session's decoded PNG frames are 1024x625). This tool
// reads the real decoded frame size (VsmLoadM01M02SessionFrame) for the
// session being matched against and scales every profile rect by
// (frame.width/profile.width, frame.height/profile.height) BEFORE computing
// any overlap — same per-axis "scale left/top/right/bottom separately, then
// truncate" pattern already used by game/Poker/TableLayoutProfile.cpp's
// Sx()/Sy() helpers, so this reproduces that codebase's existing scaling
// convention rather than inventing a new one.

// One scaled-to-frame-space layout candidate a region can be matched
// against: either a top-level element or a resolved sub-slot from
// VsmLayoutProfile, flattened into one list so both compete on equal
// footing for the best overlap.
struct VsmLayoutCandidate : Moveable<VsmLayoutCandidate> {
	String label;       // "Player0" (element) or "Player0.hole_card_0" (sub-slot)
	String kind;         // "element" or "subslot"
	String role;
	int    seat_index = -1;
	int    card_index = -1;
	Rect   rect;          // scaled to actual frame pixel space

	int Area() const { return max(0, rect.Width()) * max(0, rect.Height()); }
};

// Scales a design-space rect to frame pixel space, matching
// game/Poker/TableLayoutProfile.cpp's Sx()/Sy() convention: left/top/
// right/bottom are each scaled and truncated to int SEPARATELY (not
// width/height scaled and added to an untouched origin) so results are
// consistent with the rest of this codebase's scaling behavior.
static Rect VsmScaleRect(const Rect& r, double sx, double sy)
{
	return Rect((int)(r.left * sx), (int)(r.top * sy),
	            (int)(r.right * sx), (int)(r.bottom * sy));
}

// Builds the flattened, frame-space-scaled candidate list from one
// VsmLayoutProfile.
static Vector<VsmLayoutCandidate> VsmBuildCandidates(const VsmLayoutProfile& profile,
                                                      double sx, double sy)
{
	Vector<VsmLayoutCandidate> out;
	for(const VsmLayoutElementInfo& e : profile.elements) {
		VsmLayoutCandidate c;
		c.label = e.name;
		c.kind = "element";
		c.role = e.role;
		c.seat_index = e.seat_index;
		c.rect = VsmScaleRect(e.GetRect(), sx, sy);
		out.Add(c);
	}
	for(const VsmLayoutSubSlotInfo& s : profile.subslots) {
		VsmLayoutCandidate c;
		c.label = s.owner_name + "." + s.slot_name;
		c.kind = "subslot";
		c.role = s.role;
		c.seat_index = s.seat_index;
		c.card_index = s.card_index;
		c.rect = VsmScaleRect(s.GetRect(), sx, sy);
		out.Add(c);
	}
	return out;
}

// ---------------------------------------------------------------------------
// Overlap-matching algorithm choice (documented here + in this task's
// evidence section):
//
// Plain IoU (intersection-area / union-area) was tried conceptually first,
// but rejected: many detected regions are SMALL partial-content updates
// (e.g. a stack/bet text repaint, a card-back flip) that occupy only a
// fraction of their owning element's/sub-slot's full rect, while several
// layout candidates (whole seats, tab panes, the board) are large. A small
// region against a large candidate it is fully contained in scores a tiny
// IoU (union dominated by the large candidate's area) even though the
// region is a perfect semantic match — IoU would systematically starve
// exactly the "small region inside a bigger owning element" case this tool
// most needs to handle well.
//
// Instead this tool scores intersection-area / REGION-area (how much of
// the DETECTED region falls inside the candidate) — a region entirely
// contained in a candidate scores 1.0 regardless of the candidate's own
// size. Threshold: 0.5 (a strict majority of the region's pixels must fall
// inside the candidate).
//
// Selection is deliberately NOT "highest overlap wins across all
// candidates": because every sub-slot's rect is a geometric SUBSET of its
// owning element's rect, any region that scores highly against a sub-slot
// (e.g. `hole_card_0`) ALSO scores at least as highly against that
// sub-slot's owning element (the whole seat) — and often the coarse seat
// reaches a clean 1.0 (whole region inside the seat's bounding box) even
// when the region is really "about" one specific sub-slot within it. A
// global argmax over overlap would then almost always report the coarse
// container and never the specific sub-slot, which defeats the purpose of
// having sub-slots as match targets at all (verified empirically: an
// earlier version of this tool that DID do a flat global argmax matched
// 100% of this fixture's regions to top-level seat/board elements and 0%
// to any sub-slot — see this task's evidence section).
//
// Instead this tool matches in two specificity TIERS, most-specific first:
//   1. sub-slots (hole cards, player name/stack/bet text, action icon,
//      dealer button, timeout bar, board cards) — the highest-overlap
//      sub-slot clearing the threshold wins, if any does.
//   2. top-level elements — tried ONLY if no sub-slot cleared the
//      threshold (this is also the only tier for element types that have
//      no sub-slots at all, e.g. PotTitle/ActionButton/TabCtrl — those are
//      correctly matched at the element level since there is nothing more
//      specific to offer for them).
// This directly encodes "prefer the most specific true match" without an
// area-based heuristic that could let an incidentally-tiny but poorly
// overlapping candidate outrank a well-matched larger one.
static const double kOverlapThreshold = 0.5;

struct VsmMatchResult {
	const VsmLayoutCandidate* best = NULL;
	double overlap = 0.0;
};

// Finds the highest-overlap candidate of one specificity tier (kind ==
// "subslot" or "element") that clears kOverlapThreshold; ties broken by
// preferring the smaller-area candidate (deterministic, and mildly prefers
// the more specific of two equally-good same-tier matches).
static VsmMatchResult VsmMatchTier(const Rect& region_rect, double region_area,
                                    const Vector<VsmLayoutCandidate>& candidates,
                                    const char* tier_kind)
{
	VsmMatchResult result;
	for(const VsmLayoutCandidate& c : candidates) {
		if(c.kind != tier_kind)
			continue;
		Rect inter = region_rect & c.rect;
		if(inter.IsEmpty())
			continue;
		double overlap = ((double)inter.Width() * inter.Height()) / region_area;
		if(overlap < kOverlapThreshold)
			continue;
		if(!result.best || overlap > result.overlap + 1e-9 ||
		   (fabs(overlap - result.overlap) <= 1e-9 && c.Area() < result.best->Area())) {
			result.best = &c;
			result.overlap = overlap;
		}
	}
	return result;
}

static VsmMatchResult VsmMatchRegion(const Rect& region_rect,
                                      const Vector<VsmLayoutCandidate>& candidates)
{
	double region_area = (double)max(1, region_rect.Width()) * max(1, region_rect.Height());

	VsmMatchResult subslot_match = VsmMatchTier(region_rect, region_area, candidates, "subslot");
	if(subslot_match.best)
		return subslot_match;
	return VsmMatchTier(region_rect, region_area, candidates, "element");
}

// ---------------------------------------------------------------------------
// One layout-observation output record: the original region geometry (for
// traceability, same fields as VisualStateRegionDump's --jsonl-out records)
// plus the matched element/sub-slot (or "unassigned").
struct VsmLayoutObservationOut : Moveable<VsmLayoutObservationOut> {
	int    frame_prev = -1;
	int    frame      = -1;
	int    x = 0, y = 0, w = 0, h = 0;
	double score      = 0.0;
	String region_id;

	String assigned;     // matched element/sub-slot label, or "unassigned"
	String kind;          // "element", "subslot", or "unassigned"
	String role;           // matched candidate's role, or "unassigned"
	int    seat_index = -1;
	int    card_index = -1;
	double overlap    = 0.0;

	void Jsonize(JsonIO& json)
	{
		json
			("region_id", region_id)
			("x", x)("y", y)("w", w)("h", h)
			("score", score)
			("frame_prev", frame_prev)("frame", frame)
			("assigned", assigned)("kind", kind)("role", role)
			("seat_index", seat_index)("card_index", card_index)
			("overlap", overlap)
		;
	}
};

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	String session_dir;
	String form_path;
	int frame_start = -1;
	int frame_end   = -1;
	String jsonl_out;
	String crop_report_out;

	Vector<String> positional;
	for(int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if(arg == "--help") {
			Cout() << "Usage: VisualStateLayoutAssign <m01m02_session_dir> <path-to-.form> "
			          "[--frame-start N] [--frame-end M] [--jsonl-out <path>] "
			          "[--crop-report-out <dir>]\n"
			          "  <m01m02_session_dir>  M01/M02 session directory (metadata.json +\n"
			          "                        groundtruth.jsonl + frames/%08d.png), e.g.\n"
			          "                        var/vsm_fixtures/texas_ps6p_sample\n"
			          "  <path-to-.form>       .form layout file, e.g.\n"
			          "                        game/TexasHoldem/GameTable_PS_6p.form\n"
			          "  --frame-start N       restrict to transitions ending at frame >= N (default 0)\n"
			          "  --frame-end M         restrict to transitions ending at frame <= M (default last frame)\n"
			          "  --jsonl-out <path>    write one JSON layout-observation record per line to <path>\n"
			          "  --crop-report-out <dir>  for each transition with >=1 changed region,\n"
			          "                        write one small cropped PNG per region\n"
			          "                        (crop_<prev>_<curr>_<idx>.png, same shared crop\n"
			          "                        helper VisualStateRegionDump uses) plus one markdown\n"
			          "                        file (report_<prev>_<curr>.md) embedding the crop(s)\n"
			          "                        and a data table (region_id, x, y, w, h, score,\n"
			          "                        frame_prev, frame, assigned, role, overlap), plus a\n"
			          "                        final summary.md listing assigned/unassigned counts\n"
			          "                        and every unassigned region. Composable with\n"
			          "                        --jsonl-out.\n";
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
		else if(arg == "--crop-report-out") {
			if(i + 1 >= args.GetCount()) { Fail("--crop-report-out requires a value"); return; }
			crop_report_out = args[++i];
		}
		else {
			positional.Add(arg);
		}
	}

	if(positional.GetCount() < 2) {
		Cout() << "Usage: VisualStateLayoutAssign <m01m02_session_dir> <path-to-.form> "
		          "[--frame-start N] [--frame-end M] [--jsonl-out <path>] "
		          "[--crop-report-out <dir>]\n"
		          "(pass --help for details)\n";
		SetExitCode(1);
		return;
	}
	session_dir = positional[0];
	form_path   = positional[1];

	Cout() << "=== VisualStateModel Layout Assignment ===\n\n";

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
	       << " metadata size=" << info.table_width << "x" << info.table_height
	       << " frame_count=" << info.frame_count << "\n";

	if(info.frame_count < 2) {
		Fail("Session has fewer than 2 frames - no transitions to detect");
		return;
	}

	// --- Load the .form layout profile ---
	Vector<VsmFormLayout> layouts = VsmParseFormFile(form_path);
	if(layouts.IsEmpty()) {
		Fail(Format("Failed to parse any <layouts><item> from: %s", form_path));
		return;
	}
	const VsmFormLayout& layout = layouts[0];
	VsmLayoutProfile profile = VsmBuildLayoutProfile(layout);
	Cout() << "Layout profile: \"" << profile.name << "\" design-space "
	       << profile.width << "x" << profile.height
	       << " elements=" << profile.elements.GetCount()
	       << " subslots=" << profile.subslots.GetCount() << "\n";

	// --- Resolve the frame range, then decode frame 0 to learn the ACTUAL
	// decoded frame size (do not trust metadata.json's table_width/height -
	// see this task's scaling note above). ---
	int fs = frame_start >= 0 ? frame_start : 0;
	int fe = frame_end   >= 0 ? frame_end   : info.frame_count - 1;
	if(fs < 1) fs = 1;
	if(fe > info.frame_count - 1) fe = info.frame_count - 1;
	if(fs > fe) {
		Fail(Format("Invalid frame range: --frame-start resolves to %d > --frame-end %d", fs, fe));
		return;
	}
	int load_start = fs - 1;

	VsmFrameImage probe_frame;
	if(!VsmLoadM01M02SessionFrame(session_dir, load_start, probe_frame)) {
		Fail(Format("Failed to decode frame %d", load_start));
		return;
	}
	Cout() << "Actual decoded frame size: " << probe_frame.width << "x" << probe_frame.height << "\n";

	if(profile.width <= 0 || profile.height <= 0) {
		Fail("Layout profile has zero/negative width or height - cannot compute scale");
		return;
	}
	double sx = (double)probe_frame.width  / profile.width;
	double sy = (double)probe_frame.height / profile.height;
	Cout() << "Scale factor (frame / profile design-space): sx=" << DblStr(sx)
	       << " sy=" << DblStr(sy) << "\n";
	if(fabs(sx - 1.0) > 1e-6 || fabs(sy - 1.0) > 1e-6)
		Cout() << "  -> NOT 1.0: profile design-space size differs from the actual decoded "
		          "frame size for this session; rects were scaled before matching.\n";
	else
		Cout() << "  -> 1.0: profile design-space size matches the decoded frame exactly.\n";
	Cout() << "\n";

	Vector<VsmLayoutCandidate> candidates = VsmBuildCandidates(profile, sx, sy);
	Cout() << "Built " << candidates.GetCount() << " match candidate(s) "
	       << "(" << profile.elements.GetCount() << " element(s) + "
	       << profile.subslots.GetCount() << " sub-slot(s))\n\n";

	// --- Region detection: same VsmDetectChanges/VsmRegionMemory library
	// calls, same params, as reference/VisualStateRegionDump's M01/M02
	// session path - reused as library code, not re-implemented. ---
	VsmChangeDetectParams params;
	params.pixel_threshold = 30;
	params.block_size = 8;
	params.block_min_score = 0.05;
	params.merge_gap = 16;
	params.min_region_area = 64;

	AppLog log;
	log.SetForwardToUppLog(false);
	VsmRegionMemory mem;
	mem.SetLog(&log);
	int rgn_counter = 0;

	Vector<VsmLayoutObservationOut> observations;

	VsmFrameImage prev_frame;
	prev_frame.Set(probe_frame.width, probe_frame.height, nullptr);
	memcpy(prev_frame.data, probe_frame.data, (size_t)probe_frame.width * probe_frame.height * 4);

	for(int fid = fs; fid <= fe; fid++) {
		VsmFrameImage curr_frame;
		if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr_frame)) {
			Fail(Format("Failed to decode frame %d", fid));
			return;
		}

		Vector<VsmChangedRect> changes = VsmDetectChanges(prev_frame, curr_frame, params);

		// This transition's observations only (same order as `changes`), used
		// below to build the --crop-report-out markdown table without
		// re-deriving them from `observations` (the whole-run vector) — same
		// pattern reference/VisualStateRegionDump's --crop-report-out uses.
		Vector<VsmLayoutObservationOut> transition_obs;

		for(const VsmChangedRect& cr : changes) {
			VsmFingerprint32 fp;
			if(!VsmRegionMemory::ExtractFingerprint(curr_frame, cr.x, cr.y, cr.w, cr.h, fp)) {
				Fail(Format("ExtractFingerprint frame %d", fid));
				return;
			}
			VsmRegionMatch match = mem.FindNearest(fp, 0.3);
			VsmRegionId rid;
			if(!match.region_id.IsEmpty())
				rid = match.region_id;
			else {
				rid = Format("rgn-%04d", ++rgn_counter);
				mem.Add(rid, fp);
			}

			Rect region_rect(cr.x, cr.y, cr.x + cr.w, cr.y + cr.h);
			VsmMatchResult m = VsmMatchRegion(region_rect, candidates);

			VsmLayoutObservationOut obs;
			obs.frame_prev = fid - 1;
			obs.frame      = fid;
			obs.x = cr.x; obs.y = cr.y; obs.w = cr.w; obs.h = cr.h;
			obs.score      = cr.score;
			obs.region_id  = rid;
			if(m.best) {
				obs.assigned    = m.best->label;
				obs.kind        = m.best->kind;
				obs.role        = m.best->role;
				obs.seat_index  = m.best->seat_index;
				obs.card_index  = m.best->card_index;
				obs.overlap     = m.overlap;
			}
			else {
				obs.assigned = "unassigned";
				obs.kind     = "unassigned";
				obs.role     = "unassigned";
				obs.overlap  = 0.0;
			}
			observations.Add(obs);
			transition_obs.Add(obs);
		}

		// --crop-report-out (task 0116): mirrors reference/VisualStateRegionDump's
		// task-0110 --crop-report-out (per-region debug crop PNGs + one markdown
		// file per transition), extended with the assigned/role/overlap columns
		// this tool already computed above. Uses the shared crop helper
		// (uppsrc/VisualStateModel/FrameCrop.h) so the crop PNG rendering itself
		// is byte-for-byte the same code VisualStateRegionDump uses, not a
		// second copy of it.
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
				const VsmLayoutObservationOut& o = transition_obs[i];
				md << "## Region " << o.region_id << "\n\n";
				md << "![" << o.region_id << "](" << crop_names[i] << ")\n\n";
			}
			md << "## Region Data\n\n";
			md << "| region_id | x | y | w | h | score | frame_prev | frame "
			      "| assigned | role | overlap |\n";
			md << "| --- | --- | --- | --- | --- | --- | --- | --- "
			      "| --- | --- | --- |\n";
			for(int i = 0; i < changes.GetCount(); i++) {
				const VsmLayoutObservationOut& o = transition_obs[i];
				bool is_unassigned = (o.kind == "unassigned");
				String assigned_cell = is_unassigned ? "**UNASSIGNED**" : o.assigned;
				String overlap_cell  = is_unassigned ? "-" : DblStr(o.overlap * 100.0) + "%";
				md << "| " << o.region_id
				   << " | " << o.x << " | " << o.y << " | " << o.w << " | " << o.h
				   << " | " << DblStr(o.score)
				   << " | " << o.frame_prev << " | " << o.frame
				   << " | " << assigned_cell
				   << " | " << o.role
				   << " | " << overlap_cell << " |\n";
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

		if(prev_frame.width != curr_frame.width || prev_frame.height != curr_frame.height)
			prev_frame.Set(curr_frame.width, curr_frame.height, nullptr);
		memcpy(prev_frame.data, curr_frame.data, (size_t)curr_frame.width * curr_frame.height * 4);
	}

	int assigned_count = 0;
	for(const VsmLayoutObservationOut& o : observations)
		if(o.kind != "unassigned")
			assigned_count++;
	int total = observations.GetCount();
	double rate = total > 0 ? (double)assigned_count / total : 0.0;

	Cout() << "=== Assignment Summary ===\n";
	Cout() << "Total region observations: " << total << "\n";
	Cout() << "Assigned: " << assigned_count << " (" << DblStr(rate * 100.0) << "%)\n";
	Cout() << "Unassigned: " << (total - assigned_count)
	       << " (" << DblStr((1.0 - rate) * 100.0) << "%)\n\n";

	// --crop-report-out (task 0116): session-level summary.md, separate from
	// the per-transition report_<prev>_<curr>.md files above so a human (or
	// script) always finds it at one fixed path rather than having to scan
	// for "whichever transition file was written last" — lists the same
	// assigned/unassigned counts already printed to stdout, plus the full
	// list of unassigned regions (Done Criterion #2: "distinguish raw
	// changed regions from assigned table elements").
	if(!crop_report_out.IsEmpty()) {
		String md;
		md << "# Layout Assignment Summary\n\n";
		md << "| metric | value |\n";
		md << "| --- | --- |\n";
		md << "| Total region observations | " << total << " |\n";
		md << "| Assigned | " << assigned_count << " (" << DblStr(rate * 100.0) << "%) |\n";
		md << "| Unassigned | " << (total - assigned_count)
		   << " (" << DblStr((1.0 - rate) * 100.0) << "%) |\n\n";

		int unassigned_total = total - assigned_count;
		if(unassigned_total > 0) {
			md << "## Unassigned Regions\n\n";
			md << "These are raw detected changed regions with no matching layout "
			      "element/sub-slot at overlap >= " << DblStr(kOverlapThreshold)
			   << " — layout-model gaps to investigate.\n\n";
			md << "| region_id | frame_prev | frame | x | y | w | h | score |\n";
			md << "| --- | --- | --- | --- | --- | --- | --- | --- |\n";
			for(const VsmLayoutObservationOut& o : observations) {
				if(o.kind != "unassigned")
					continue;
				md << "| " << o.region_id
				   << " | " << o.frame_prev << " | " << o.frame
				   << " | " << o.x << " | " << o.y << " | " << o.w << " | " << o.h
				   << " | " << DblStr(o.score) << " |\n";
			}
		}
		else {
			md << "No unassigned regions — every detected changed region matched a "
			      "layout element or sub-slot.\n";
		}

		String summary_path = AppendFileName(crop_report_out, "summary.md");
		RealizeDirectory(GetFileFolder(summary_path));
		if(!SaveFile(summary_path, md)) {
			Fail(Format("Failed to write --crop-report-out summary file: %s", summary_path));
			return;
		}
		Cout() << "Wrote crop report summary: " << summary_path << "\n\n";
	}

	if(!jsonl_out.IsEmpty()) {
		String jsonl;
		for(const VsmLayoutObservationOut& o : observations)
			jsonl << StoreAsJson(o) << "\n";
		if(!SaveFile(jsonl_out, jsonl)) {
			Fail(Format("Failed to write --jsonl-out file: %s", jsonl_out));
			return;
		}
		Cout() << "Wrote " << observations.GetCount() << " layout observation record(s) to "
		       << jsonl_out << "\n";
	}
	else {
		Cout() << "=== JSONL layout observation records (stdout; use --jsonl-out <path> to save) ===\n";
		for(const VsmLayoutObservationOut& o : observations)
			Cout() << StoreAsJson(o) << "\n";
	}

	SetExitCode(0);
}
