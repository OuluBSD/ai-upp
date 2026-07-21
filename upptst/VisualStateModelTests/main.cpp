#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// M04-06 (task 0117): automated regression test suite for the M04 layout-
// model pipeline (FormLayout / LayoutProfile / RegionAssign), asserting
// against values already independently hand-verified in tasks 0112-0116's
// own evidence sections (Manager repo,
// VisualStateModel/011{2,3,4,5,6}_*.md) — every CHECK below cites, in its
// own description string, which task's evidence the expected number traces
// back to. None of the expected numbers below were invented for this task;
// see this task's own evidence section (0117_m04_automated_test_suite.md)
// for the full cross-reference.
//
// This is a CHECK-accumulate-and-report harness, NOT a raw assert()-aborts-
// on-first-failure demo (see upptst/GeometryTests for that style, kept as
// deliberately different precedent — this is a CI-style regression suite
// meant to be re-run, where one bad assertion must not hide the rest of the
// run's results): every CHECK() call records pass/fail and prints
// PASS/FAIL immediately, and CONSOLE_APP_MAIN's final summary calls
// SetExitCode(0) iff every check passed, SetExitCode(1) otherwise.

struct VsmTestHarness {
	int total  = 0;
	int passed = 0;

	void Check(bool cond, const String& desc)
	{
		total++;
		if(cond) {
			passed++;
			Cout() << "PASS: " << desc << "\n";
		}
		else {
			Cout() << "FAIL: " << desc << "\n";
		}
	}
};

static VsmTestHarness g_h;
#define CHECK(cond, desc) g_h.Check((cond), (desc))

static void RunShaderTemplateChecks()
{
	Cout() << "\n--- M0295 shader template reference ---\n";
	VsmImageBuffer window_a = VsmImageBuffer::MakeSolid(4, 2, 10, 1);
	VsmImageBuffer window_b = VsmImageBuffer::MakeSolid(3, 2, 20, 1);
	VsmImageBuffer packed;
	Vector<VsmPackedWindow> windows;
	String pack_error;
	CHECK(VsmPackTwoWindows(window_a, window_b, packed, windows, pack_error),
	      "0295a packs exactly two windows without desktop pixels");
	CHECK(packed.width == 7 && packed.height == 2 && windows.GetCount() == 2 &&
	      windows[1].source_x == 4 && packed.Get(4, 0) == 20,
	      "0295a preserves window order and source offset metadata");
	VsmImageBuffer frame = VsmImageBuffer::MakeSolid(12, 8, 0, 1);
	VsmImageBuffer crops = VsmImageBuffer::MakeSolid(8, 4, 0, 1);
	// Two templates deliberately have different dimensions. The second is
	// placed at (4,1), proving that the matcher does not pad to a common size.
	for(int y = 0; y < 2; y++)
		for(int x = 0; x < 2; x++) {
			byte value = (byte)((x + y) ? 180 : 255);
			crops.Set(x, y, value);
			frame.Set(3 + x, 2 + y, value);
		}
	for(int x = 0; x < 3; x++) {
		crops.Set(4 + x, 1, (byte)(40 + x * 80));
		frame.Set(4 + x, 1, (byte)(40 + x * 80));
	}

	VsmShaderTemplateManifest manifest;
	manifest.crop_map_width = crops.width;
	manifest.crop_map_height = crops.height;
	VsmShaderTemplate& a = manifest.templates.Add();
	a.id = "digit-a"; a.label = "digit-a"; a.x = 0; a.y = 0; a.w = 2; a.h = 2;
	a.hotspot_x = 1; a.hotspot_y = 1;
	VsmShaderTemplate& b = manifest.templates.Add();
	b.id = "digit-b"; b.label = "digit-b"; b.x = 4; b.y = 1; b.w = 3; b.h = 1;
	b.hotspot_x = 1; b.hotspot_y = 0;
	String error;
	CHECK(manifest.Validate(error), "0295a manifest accepts independently sized crop-map entries");
	CHECK(error.IsEmpty(), "0295a valid manifest produces no validation error");

	VsmCpuShaderTemplateMatcher matcher;
	VsmShaderEvidence evidence;
	CHECK(matcher.Match(frame, crops, manifest, evidence, error),
	      "0295b CPU reference matcher completes on compact grayscale input");
	CHECK(evidence.image.width == frame.width && evidence.image.height == frame.height &&
	      evidence.image.channels == 3,
	      "0295b evidence image preserves frame dimensions and RGB contract");
	CHECK(evidence.best_hits[0].x == 3 && evidence.best_hits[0].y == 2 &&
	      evidence.best_hits[0].score > 0.999,
	      "0295b exact 2x2 template match reports its hotspot-independent origin");
	CHECK(evidence.best_hits[1].x == 4 && evidence.best_hits[1].y == 1 &&
	      evidence.best_hits[1].score > 0.999,
	      "0295b exact 3x1 template match reports its own-size origin");
	CHECK(evidence.image.Get(3, 2, 0) == 255,
	      "0295b R channel stores a full-strength best score");
	CHECK(evidence.image.Get(3, 2, 1) == 0,
	      "0295b G channel stores the winning template index");

	VsmThresholdResult threshold = VsmAnalyzeEvidenceThreshold(evidence.image, 0);
	CHECK(threshold.accepted > 0 && threshold.rejected > 0,
	      "0295f evidence analysis separates accepted and rejected pixels");
	VsmOccupancyMask mask = VsmBuildOccupancyMask(evidence.image, 250);
	Rect bounds = VsmFindOccupancyBounds(mask);
	CHECK(!bounds.IsEmpty() && bounds.left <= 3 && bounds.top <= 1,
	      "0295g occupancy reduction preserves two-dimensional evidence bounds");
	VsmPackedOccupancyMask packed_mask = VsmBuildPackedOccupancyMask(evidence.image, 250);
	VsmTileOccupancyMask tile_mask = VsmBuildTileOccupancyMask(evidence.image, 250, 2);
	VsmTileOccupancyMask benchmark_tile_mask = VsmBuildTileOccupancyMask(evidence.image, 250);
	VsmOccupancyBenchmark benchmark = VsmBenchmarkOccupancy(evidence.image, 250, 2);
	CHECK(packed_mask.bits.GetCount() < evidence.image.width * evidence.image.height &&
	      tile_mask.tiles.GetCount() < evidence.image.width * evidence.image.height,
	      "0295g packed and tile masks reduce transfer size");
	CHECK(benchmark.packed_bytes == packed_mask.bits.GetCount() &&
	      benchmark.tile_bytes == benchmark_tile_mask.tiles.GetCount() && benchmark.xy_bytes == evidence.image.width * evidence.image.height,
	      "0295g benchmark reports deterministic transfer byte counts");
	Vector<VsmEvidenceTextRun> runs = VsmReconstructEvidence(evidence.image, manifest, 250);
	CHECK(runs.GetCount() >= 1 && runs[0].ambiguous,
	      "0295d marks non-digit template evidence as ambiguous instead of inventing text");
	CHECK(VsmCpuShaderTemplateMatcher::FragmentShaderSource().Find("crop_map") >= 0,
	      "0295c shader contract names the compact frame and crop-map inputs");

	String manifest_path = "tmp/m0295_template_manifest.json";
	CHECK(manifest.Save(manifest_path), "0295a manifest serializes to JSON");
	VsmShaderTemplateManifest loaded;
	CHECK(loaded.Load(manifest_path) && loaded.templates.GetCount() == 2,
	      "0295a manifest round-trips with both template sizes intact");
}

// ---------------------------------------------------------------------------
// Small rect helpers shared by every check below. Every expected number in
// this file is written in the same "(x,y,w,h)" shape the task spec and prior
// tasks' evidence sections already use (top-left + width/height), not
// U++'s internal left/top/right/bottom Rect representation.

static Rect XYWH(int x, int y, int w, int h) { return Rect(x, y, x + w, y + h); }
static bool RectEq(const Rect& r, int x, int y, int w, int h) { return r == XYWH(x, y, w, h); }

static const VsmLayoutElementInfo* FindElementInfo(const VsmLayoutProfile& profile, const String& name)
{
	for(const VsmLayoutElementInfo& e : profile.elements)
		if(e.name == name)
			return &e;
	return NULL;
}

static const VsmFormSubSlot* FindSubSlot(const Vector<VsmFormSubSlot>& slots, const String& name)
{
	for(const VsmFormSubSlot& s : slots)
		if(s.name == name)
			return &s;
	return NULL;
}

// ---------------------------------------------------------------------------
// Region-to-element assignment against the real recorded fixture, using the
// now-shared uppsrc/VisualStateModel/RegionAssign.h logic (task 0117's
// extraction) — reproduces reference/VisualStateLayoutAssign/main.cpp's own
// frame-scan loop (same VsmChangeDetectParams, same frame range for an
// 8-frame session: transitions 1..7) but does not need VsmRegionMemory's
// region-id bookkeeping, since these checks only care about the matched
// element/overlap per detected region, not its stable id across frames.
//
// Also exercises the never-yet-exercised "unassigned" path (flagged as a
// real gap by both task 0115's and task 0116's evidence sections: real
// fixture data never produces an unassigned region, because
// GameTable_PS_6p.form's Board element spans the entire design canvas) —
// with a synthetic region rect placed far outside the decoded frame's
// bounds (1024x625 for this fixture), which cannot overlap ANY candidate,
// not even Board.
static void RunRegionAssignmentChecks(const VsmLayoutProfile& profile)
{
	String session_dir = "var/vsm_fixtures/texas_ps6p_sample";
	CHECK(DirectoryExists(session_dir), "texas_ps6p_sample fixture directory exists");

	VsmM01M02SessionInfo info;
	bool read_ok = VsmReadM01M02SessionInfo(session_dir, info);
	CHECK(read_ok, "VsmReadM01M02SessionInfo reads texas_ps6p_sample/metadata.json");
	if(!read_ok)
		return;

	VsmFrameImage probe;
	bool probe_ok = VsmLoadM01M02SessionFrame(session_dir, 0, probe);
	CHECK(probe_ok, "VsmLoadM01M02SessionFrame decodes texas_ps6p_sample frame 0");
	if(!probe_ok)
		return;

	// Same scaling as VisualStateLayoutAssign (task 0115): profile design-space
	// vs. actual decoded frame size (1024x648 vs. 1024x625 for this fixture).
	double sx = (double)probe.width  / profile.width;
	double sy = (double)probe.height / profile.height;
	Vector<VsmLayoutCandidate> candidates = VsmBuildCandidates(profile, sx, sy);

	// Same VsmChangeDetectParams as reference/VisualStateLayoutAssign/main.cpp
	// and reference/VisualStateRegionDump/main.cpp's M01/M02 session path.
	VsmChangeDetectParams params;
	params.pixel_threshold = 30;
	params.block_size = 8;
	params.block_min_score = 0.05;
	params.merge_gap = 16;
	params.min_region_area = 64;

	int total_regions    = 0;
	int assigned_regions = 0;
	bool found_player3_stack = false;
	double player3_stack_overlap = -1.0;

	VsmFrameImage prev;
	prev.Set(probe.width, probe.height, nullptr);
	memcpy(prev.data, probe.data, (size_t)probe.width * probe.height * 4);

	for(int fid = 1; fid <= info.frame_count - 1; fid++) {
		VsmFrameImage curr;
		if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr)) {
			CHECK(false, Format("VsmLoadM01M02SessionFrame decodes texas_ps6p_sample frame %d", fid));
			continue;
		}

		Vector<VsmChangedRect> changes = VsmDetectChanges(prev, curr, params);
		for(const VsmChangedRect& cr : changes) {
			total_regions++;
			Rect region_rect(cr.x, cr.y, cr.x + cr.w, cr.y + cr.h);
			VsmMatchResult m = VsmMatchRegion(region_rect, candidates);
			if(m.best) {
				assigned_regions++;
				if(m.best->label == "Player3.stack_text") {
					found_player3_stack = true;
					player3_stack_overlap = m.overlap;
				}
			}
		}

		if(prev.width != curr.width || prev.height != curr.height)
			prev.Set(curr.width, curr.height, nullptr);
		memcpy(prev.data, curr.data, (size_t)curr.width * curr.height * 4);
	}

	CHECK(total_regions == 39,
	      Format("texas_ps6p_sample: exactly 39 region observations (got %d) [task 0115/0116 evidence]",
	             total_regions));
	CHECK(assigned_regions == total_regions,
	      Format("texas_ps6p_sample: 100%% assigned (%d/%d) [task 0115/0116 evidence]",
	             assigned_regions, total_regions));
	CHECK(found_player3_stack, "texas_ps6p_sample: some region matched Player3.stack_text");
	// M05-05 (task 0123): this overlap ratio changed from the original
	// 0.9375 (task 0115's hand-verified value) to 0.53125 because Objective
	// 1 moved/resized Player3's own rect in GameTable_PS_6p.form (from
	// (400,10,282,179) to (380,10,240,180), to remove its overlaps with
	// Player2 and Player4) -- the SAME raw recorded region (416,120,32,16,
	// real pixel data from this fixture's frames, unaffected by this task)
	// now scores differently against Player3.stack_text's candidate rect,
	// which is derived from Player3's own (now different) rect. Re-verified
	// as the real, deterministic output of this build (not guessed) rather
	// than papering over a changed assertion.
	CHECK(found_player3_stack && fabs(player3_stack_overlap - 0.53125) < 1e-9,
	      Format("texas_ps6p_sample: the Player3.stack_text match's overlap == 0.53125 exactly "
	             "(got %s) [task 0123: changed from 0115's 0.9375 by the Player3 .form rect "
	             "redesign, region rect (416,120,32,16)]",
	             found_player3_stack ? DblStr(player3_stack_overlap) : String("n/a (no match found)")));

	// --- The never-yet-exercised "unassigned" path (task 0117 Objective) ---
	Rect far_rect(5000, 5000, 5040, 5040); // nowhere near the 1024x625 decoded frame
	VsmMatchResult unassigned = VsmMatchRegion(far_rect, candidates);
	CHECK(unassigned.best == NULL,
	      "VsmMatchRegion returns no match (best==NULL / \"unassigned\") for a region far "
	      "outside every candidate's rect -- exercises the gap flagged (but never hit by real "
	      "fixture data) in tasks 0115/0116's evidence sections");
	CHECK(unassigned.overlap == 0.0,
	      "VsmMatchRegion's overlap is 0.0 for the unassigned case");
}

// ---------------------------------------------------------------------------
// M05-06 (task 0124): second, structurally-divergent `.form` fixture — a
// synthetic "PlayerCtrlMerged" seat type whose player-name and action-text
// controls are MERGED into one `.form`-declared child element (as opposed to
// real PlayerCtrl's two SEPARATE opaque C++-only children, `label_PlayerName`/
// `actionPic`), stress-testing whether the M04 "no fixed schema" architecture
// (FormLayout/LayoutProfile/RegionAssign) actually generalizes to a
// DIFFERENT widget composition, not just a different rect layout of the SAME
// PlayerCtrl vocabulary (both existing forms, GameTable.form and
// GameTable_PS_6p.form, declare the identical 38-element vocabulary — see
// this task's own evidence section for the full rationale). This fixture
// also has NO BoardCtrl and no action-button/human-input elements at all — a
// further stress case for "profile-building must not assume there is always
// a Board".
static void RunMergedNameActionFixtureChecks()
{
	String path = "upptst/VisualStateModelTests/testdata/GameTable_MergedNameAction.form";
	Vector<VsmFormLayout> layouts = VsmParseFormFile(path);
	CHECK(layouts.GetCount() >= 1,
	      "GameTable_MergedNameAction.form: VsmParseFormFile returns >= 1 layout");
	if(layouts.IsEmpty())
		return;

	const VsmFormLayout& layout = layouts[0];
	CHECK(layout.elements.GetCount() == 9,
	      "GameTable_MergedNameAction.form: VsmParseFormFile yields exactly 9 elements "
	      "(3 PlayerCtrlMerged seats + 3 nameaction children + 3 stacktext children) "
	      "[task 0124 fixture design]");

	// --- Central architectural point: VsmGetSubSlots("PlayerCtrlMerged") is
	// EMPTY -- this fixture's name/action + stack children are handled
	// ENTIRELY by the flat-element-list + Parent-nesting mechanism
	// (GetAbsoluteRect()'s parent-walk), NOT the per-type VsmFormSubSlot
	// synthetic sub-slot table real PlayerCtrl needs. ---
	Vector<VsmFormSubSlot> merged_subslots = VsmGetSubSlots("PlayerCtrlMerged");
	CHECK(merged_subslots.IsEmpty(),
	      "VsmGetSubSlots(\"PlayerCtrlMerged\") returns an EMPTY vector -- confirms this fixture's "
	      "merged name/action and stack children are real, .form-declared, Parent-nested top-level "
	      "elements resolved via the SAME generic GetAbsoluteRect() parent-walk every other nested "
	      "element (e.g. HumanButtons/BtnFold) already uses, not a new per-type VsmFormSubSlot table "
	      "entry [task 0124's central architectural point]");

	VsmLayoutProfile profile = VsmBuildLayoutProfile(layout);
	CHECK(profile.elements.GetCount() == 9,
	      "VsmBuildLayoutProfile(MergedNameAction): exactly 9 elements");
	CHECK(profile.subslots.IsEmpty(),
	      "VsmBuildLayoutProfile(MergedNameAction): subslots is EMPTY for every element of this "
	      "profile -- confirms no accidental fallback/inheritance from the real \"PlayerCtrl\" "
	      "sub-slot table [task 0124]");

	// --- 3 seats: role=="seat", correct seat_index, correct own rect ---
	struct SeatExpect { const char* elem_name; int seat_index; int x, y, w, h; };
	static const SeatExpect seats[] = {
		{ "Player0", 0, 50,  50, 200, 150 },
		{ "Player1", 1, 300, 50, 200, 150 },
		{ "Player2", 2, 550, 50, 200, 150 },
	};
	for(const SeatExpect& se : seats) {
		const VsmLayoutElementInfo* seat = FindElementInfo(profile, se.elem_name);
		CHECK(seat != NULL, Format("MergedNameAction: %s element found", se.elem_name));
		if(!seat)
			continue;
		CHECK(seat->role == "seat",
		      Format("MergedNameAction: %s -> role \"seat\" [task 0124 Objective 1]", se.elem_name));
		CHECK(seat->seat_index == se.seat_index,
		      Format("MergedNameAction: %s -> seat_index==%d (VsmParseSeatIndex unchanged, "
		             "Variable-pattern-based not Type-based -- confirmed, not assumed) [task 0124]",
		             se.elem_name, se.seat_index));
		CHECK(RectEq(seat->GetRect(), se.x, se.y, se.w, se.h),
		      Format("MergedNameAction: %s's own rect == (%d,%d,%d,%d)",
		             se.elem_name, se.x, se.y, se.w, se.h));
	}

	// --- Each seat's 2 Parent-nested children: the merged name/action child
	// classifies to the NEW "name_action" role (not "unknown", not
	// accidentally "player_name"/"action_icon" -- those roles' existing
	// meaning is specifically for the SEPARATE-control case and must not
	// leak here); the stack child reuses the EXISTING "stack_text" role. ---
	struct ChildExpect { const char* name; const char* expect_role; int x, y, w, h; };
	static const ChildExpect children[] = {
		{ "NameAction0", "name_action", 60,  150, 180, 20 },
		{ "StackText0",  "stack_text",  60,  175, 180, 20 },
		{ "NameAction1", "name_action", 310, 150, 180, 20 },
		{ "StackText1",  "stack_text",  310, 175, 180, 20 },
		{ "NameAction2", "name_action", 560, 150, 180, 20 },
		{ "StackText2",  "stack_text",  560, 175, 180, 20 },
	};
	for(const ChildExpect& ce : children) {
		const VsmLayoutElementInfo* el = FindElementInfo(profile, ce.name);
		CHECK(el != NULL, Format("MergedNameAction: %s element found", ce.name));
		if(!el)
			continue;
		CHECK(el->role == ce.expect_role,
		      Format("MergedNameAction: %s -> role \"%s\" (not \"unknown\", not \"player_name\"/"
		             "\"action_icon\") [task 0124 Objective 1]", ce.name, ce.expect_role));
		CHECK(RectEq(el->GetRect(), ce.x, ce.y, ce.w, ce.h),
		      Format("MergedNameAction: %s's absolute (Parent-resolved) rect == (%d,%d,%d,%d)",
		             ce.name, ce.x, ce.y, ce.w, ce.h));
	}

	// --- Region-match test via the shared RegionAssign.h logic (task 0117's
	// extraction), scale 1:1 (design space treated directly as "frame" space
	// here -- there is no recorded session for this synthetic fixture). ---
	Vector<VsmLayoutCandidate> candidates = VsmBuildCandidates(profile, 1.0, 1.0);

	// Region matching seat0's name/action child rect CLOSELY (its real,
	// computed absolute rect -- via GetAbsoluteRect/the profile above, not a
	// hand-guessed number) -- expect it to resolve to that element
	// specifically, kind=="element" (there are no "subslot"-kind candidates
	// at all for this fixture, since VsmGetSubSlots("PlayerCtrlMerged") is
	// empty), not fall through to the whole seat.
	const VsmLayoutElementInfo* na0 = FindElementInfo(profile, "NameAction0");
	CHECK(na0 != NULL, "MergedNameAction: NameAction0 found for region-match test");
	if(na0) {
		Rect region_rect = na0->GetRect();
		VsmMatchResult m = VsmMatchRegion(region_rect, candidates);
		CHECK(m.best != NULL,
		      "MergedNameAction: region matching NameAction0's exact rect resolves to something");
		if(m.best) {
			CHECK(m.best->label == "NameAction0",
			      Format("MergedNameAction: region matching NameAction0's rect resolves to "
			             "\"NameAction0\" specifically, not the whole seat (got \"%s\")",
			             ~m.best->label));
			CHECK(m.best->kind == "element",
			      Format("MergedNameAction: NameAction0's match kind == \"element\" (got \"%s\") -- "
			             "there are no subslot-kind candidates for this fixture", ~m.best->kind));
		}
	}

	// Region matching seat1's WHOLE seat rect but neither child's rect --
	// expect it to resolve to the seat element itself (neither child's rect
	// clears kOverlapThreshold against this much-larger region: NameAction1/
	// StackText1 are each 180x20=3600 vs. Player1's 200x150=30000, an overlap
	// ratio of 0.12, well under 0.5).
	const VsmLayoutElementInfo* player1 = FindElementInfo(profile, "Player1");
	CHECK(player1 != NULL, "MergedNameAction: Player1 found for region-match test");
	if(player1) {
		Rect region_rect = player1->GetRect();
		VsmMatchResult m = VsmMatchRegion(region_rect, candidates);
		CHECK(m.best != NULL,
		      "MergedNameAction: region matching Player1's whole rect resolves to something");
		if(m.best)
			CHECK(m.best->label == "Player1",
			      Format("MergedNameAction: region matching Player1's whole rect resolves to "
			             "\"Player1\" itself, not one of its children (got \"%s\")", ~m.best->label));
	}
}

// ---------------------------------------------------------------------------
// M07-01 (task 0137): live-tailing frame source regression.
//
// Proves VsmLiveM01M02SessionSource genuinely OBSERVES an M01/M02 session
// directory being written incrementally by a concurrently-running
// TexasHoldem.exe --record-session process -- not just re-reading a directory
// that already happens to be complete. Two parts:
//
//   (A) A deterministic, no-subprocess check that an OLD-format completed
//       fixture (no "status" marker at all -- var/vsm_fixtures/texas_ps6p_sample,
//       frozen, recorded before this task) opens as "already complete" and
//       yields all N frames immediately via the base ReadFrame() one-shot path.
//       This guards the backward-compat requirement that absence of the marker
//       is read as "complete", never "recording forever".
//
//   (B) The genuine live-tailing part: launch TexasHoldem.exe --record-session
//       into a scratch dir as a BACKGROUND process, open the live source while
//       recording is still in progress, and poll ReadFrameLive() concurrently.
//       Asserts (a) metadata.frame_count is discoverable up-front before the
//       recording finishes, (b) frames are observed strictly in order, (c) at
//       least one ReadFrameLive() call returned LIVE_PENDING (had to wait for a
//       frame that wasn't on disk yet) -- the check that would FAIL if the
//       source only worked against an already-finished directory, (d) all N
//       frames are eventually read, (e) genuine LIVE_EOS at the end once the
//       recorder marks the session complete, (f) per-frame ground truth becomes
//       readable. Scratch recording is deleted afterward.

static void RunLiveTailingCompletedFixtureCheck()
{
	// (A) old-format completed fixture, no subprocess.
	String session = "var/vsm_fixtures/texas_ps6p_sample";
	CHECK(DirectoryExists(session), "texas_ps6p_sample fixture exists for live-source completed-read check");
	if(!DirectoryExists(session))
		return;

	VsmLiveM01M02SessionSource src;
	bool opened = src.Open(session);
	CHECK(opened, "VsmLiveM01M02SessionSource opens the old-format completed texas_ps6p_sample");
	if(!opened)
		return;
	CHECK(src.IsRecordingComplete(),
	      "old-format session with NO \"status\" field reads as COMPLETE (absence != \"recording forever\") "
	      "[task 0137 backward-compat guardrail]");
	CHECK(src.GetTargetFrameCount() == 8,
	      Format("live source reads frame_count==8 from texas_ps6p_sample metadata (got %d)",
	             src.GetTargetFrameCount()));

	int read = 0;
	bool order_ok = true, images_ok = true;
	VsmImageBuffer buf; int64 ts = 0;
	while(src.ReadFrame(buf, ts)) {   // base one-shot contract on a complete session
		if(buf.width <= 0 || buf.height <= 0 || buf.channels != 4)
			images_ok = false;
		if(src.GetCursor() != read + 1)
			order_ok = false;
		read++;
	}
	CHECK(read == 8,
	      Format("base ReadFrame() drains all 8 frames of a completed session like a one-shot reader (got %d)", read));
	CHECK(order_ok, "completed-session frames read strictly in order via the live source");
	CHECK(images_ok, "completed-session frames decode to non-empty RGBA (channels==4) buffers");

	// EOS is genuine end-of-stream on a complete session (not a wait).
	VsmImageBuffer b2; int64 t2 = 0;
	CHECK(src.ReadFrameLive(b2, t2) == VsmLiveM01M02SessionSource::LIVE_EOS,
	      "after draining a completed session, ReadFrameLive() reports LIVE_EOS (genuine done, not PENDING)");
}

static void RunLiveTailingIncrementalCheck()
{
	// (B) genuine incremental observation against a live recorder subprocess.
	String exe = GetExeDirFile("TexasHoldem.exe");
	CHECK(FileExists(exe),
	      Format("TexasHoldem.exe present next to the test exe (%s) for live-tailing subprocess", ~exe));
	if(!FileExists(exe))
		return;

	String cwd = GetCurrentDirectory();
	String scratch = AppendFileName(cwd, "var/vsm_fixtures/task0137_livetail_scratch");
	if(DirectoryExists(scratch))
		DeleteFolderDeep(scratch);
	RealizeDirectory(scratch);

	const int kFrames = 24;
	Vector<String> args;
	args << "--record-session" << "--provider" << "PS_6p" << "--seed" << "1"
	     << "--frames" << IntStr(kFrames) << "--out" << scratch;

	LocalProcess proc;
	bool started = proc.Start(exe, args, NULL, cwd);   // cd = repo root so assets resolve
	CHECK(started, "launched background TexasHoldem.exe --record-session for live-tailing");
	if(!started) {
		DeleteFolderDeep(scratch);
		return;
	}

	String drain;
	VsmLiveM01M02SessionSource src;

	// Open as soon as metadata.json exists (written up-front, before frames). Retry
	// because the recorder needs a moment to start and write metadata.
	bool opened = false;
	int open_retries = 0;
	{
		TimeStop ts;
		while(ts.Seconds() < 30) {           // Elapsed() is microseconds; use Seconds()
			proc.Read(drain);               // keep the pipe from filling / blocking the recorder
			if(src.Open(scratch)) { opened = true; break; }
			open_retries++;
			Sleep(5);
		}
	}
	CHECK(opened,
	      Format("live source Open() succeeded on a still-recording session (metadata up-front, %d retries)",
	             open_retries));
	if(!opened) {
		proc.Kill();
		DeleteFolderDeep(scratch);
		return;
	}
	CHECK(src.GetTargetFrameCount() == kFrames,
	      Format("metadata frame_count==%d discoverable while recording still in progress (got %d)",
	             kFrames, src.GetTargetFrameCount()));

	int  frames_read   = 0;
	int  pending_waits  = 0;
	int  last_index    = -1;
	bool order_ok      = true;
	bool images_ok     = true;
	VsmLiveM01M02SessionSource::LiveResult final_r = VsmLiveM01M02SessionSource::LIVE_PENDING;

	TimeStop run;
	while(run.Seconds() < 90) {
		proc.Read(drain);
		VsmImageBuffer buf; int64 ts = 0;
		VsmLiveM01M02SessionSource::LiveResult r = src.ReadFrameLive(buf, ts);
		if(r == VsmLiveM01M02SessionSource::LIVE_OK) {
			int idx = src.GetCursor() - 1;
			if(idx != last_index + 1) order_ok = false;
			last_index = idx;
			if(buf.width <= 0 || buf.height <= 0 || buf.channels != 4) images_ok = false;
			frames_read++;
		}
		else if(r == VsmLiveM01M02SessionSource::LIVE_PENDING) {
			pending_waits++;
			Sleep(3);
		}
		else { // LIVE_EOS
			final_r = r;
			break;
		}
	}

	// Let the recorder finish and drain remaining output.
	{
		TimeStop ts;
		while(proc.IsRunning() && ts.Seconds() < 15) {
			proc.Read(drain);
			Sleep(5);
		}
	}

	CHECK(frames_read == kFrames,
	      Format("live source observed all %d frames incrementally (got %d)", kFrames, frames_read));
	CHECK(order_ok, "live source observed frames strictly in ascending order 0..N-1");
	CHECK(images_ok, "every live-observed frame decoded to a non-empty RGBA (channels==4) buffer");
	CHECK(pending_waits > 0,
	      Format("live source had to WAIT (LIVE_PENDING) at least once before a frame was on disk "
	             "(got %d pending polls) -- proves genuine incremental tailing, NOT reading an "
	             "already-complete directory", pending_waits));
	CHECK(final_r == VsmLiveM01M02SessionSource::LIVE_EOS,
	      "live source reported genuine LIVE_EOS after all frames consumed + recorder marked complete");
	CHECK(src.IsRecordingComplete(),
	      "recorder's final \"status\":\"complete\" rewrite was observed by the live source");

	// Per-frame ground truth became available for every recorded frame.
	int gt_ok = 0;
	for(int i = 0; i < kFrames; i++) {
		String line;
		if(src.TryReadGroundTruth(i, line))
			gt_ok++;
	}
	CHECK(gt_ok == kFrames,
	      Format("opportunistic per-frame ground truth readable for all %d frames after completion (got %d)",
	             kFrames, gt_ok));

	DeleteFolderDeep(scratch);
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT);
	Cout() << "=== VisualStateModel M04 Automated Test Suite (task 0117) ===\n\n";

	// --- VsmParseFormFile on GameTable_PS_6p.form ---
	{
		Vector<VsmFormLayout> layouts = VsmParseFormFile("game/TexasHoldem/GameTable_PS_6p.form");
		CHECK(layouts.GetCount() >= 1, "VsmParseFormFile(GameTable_PS_6p.form) returns >= 1 layout");

		if(layouts.GetCount() >= 1) {
			const VsmFormLayout& layout = layouts[0];

			CHECK(layout.elements.GetCount() == 38,
			      "GameTable_PS_6p.form: VsmParseFormFile yields exactly 38 elements "
			      "[task 0112/0115 evidence]");

			const VsmFormElement* player0 = layout.Find("Player0");
			CHECK(player0 != NULL, "GameTable_PS_6p.form: Player0 element found");
			if(player0)
				CHECK(RectEq(player0->GetRect(), 375, 364, 255, 162),
				      "GameTable_PS_6p.form: Player0's own rect == (375,364,255,162) "
				      "[task 0117 spec, independently cross-checked against the raw .form XML]");

			const VsmFormElement* btnfold = layout.Find("BtnFold");
			CHECK(btnfold != NULL, "GameTable_PS_6p.form: BtnFold element found");
			if(btnfold) {
				Rect abs_rect = layout.GetAbsoluteRect(*btnfold);
				CHECK(RectEq(abs_rect, 399, 616, 239, 26),
				      "GameTable_PS_6p.form: BtnFold.GetAbsoluteRect() == (399,616,239,26), "
				      "resolved through its HumanButtons parent [task 0117 spec]");
			}

			const VsmFormElement* board = layout.Find("Board");
			CHECK(board != NULL, "GameTable_PS_6p.form: Board element found");

			// --- VsmGetSubSlots/VsmResolveSubSlot against Player0's PS_6p rect
			// [task 0113/0114 evidence] ---
			if(player0) {
				Rect owner_rect = layout.GetAbsoluteRect(*player0);
				Vector<VsmFormSubSlot> slots = VsmGetSubSlots("PlayerCtrl");

				struct Expect { const char* name; int x, y, w, h; };
				static const Expect expects[] = {
					{ "hole_card_0",  482, 375, 64, 68 },
					{ "hole_card_1",  522, 375, 64, 68 },
					// M05-05 (task 0123): button_puck's SIZE is now aspect-
					// locked to a single uniform scale factor (square,
					// min(fcx*ow, fcy*oh) = min(42.94, 36.56) = 36 for
					// Player0's unchanged 255x162 rect), down from the old
					// independent-fx/fy 42x36. Position (562,449) is
					// unaffected -- it still uses independent fx/fy.
					{ "button_puck",  562, 449, 36, 36 },
					{ "avatar",       388, 375, 80, 68 },
					{ "player_name",  388, 449, 228, 17 },
				};
				for(const Expect& ex : expects) {
					const VsmFormSubSlot* slot = FindSubSlot(slots, ex.name);
					CHECK(slot != NULL, Format("PlayerCtrl sub-slot '%s' is declared", ex.name));
					if(slot) {
						Rect r = VsmResolveSubSlot(*slot, owner_rect);
						CHECK(RectEq(r, ex.x, ex.y, ex.w, ex.h),
						      Format("Player0.%s resolves to (%d,%d,%d,%d) "
						             "[task 0113/0114 evidence]", ex.name, ex.x, ex.y, ex.w, ex.h));
					}
				}
			}

			// --- VsmGetSubSlots/VsmResolveSubSlot against the PS_6p Board rect ---
			// Task 0129 (M05-11): expected values reflect the ACTUAL profile the
			// PS_6p recorder renders board cards with,
			// "texas-holdem-legacy-pokertable" (board_x=680, board_y=293,
			// board_step=113, board_w=94, board_h=133), NOT the "ps-6p" profile
			// (641/349/131/122/143) tasks 0113-0128 mistakenly assumed. Fresh
			// PaintBoard instrumentation proved the active profile is
			// legacy-pokertable (see FormLayout.cpp's ROOT-CAUSE NOTE and Manager
			// task 0129's evidence). Numbers below are legacy-pokertable resolved
			// against the .form design-space (0,0,1024,648) Board rect:
			//   x0=(int)(680/1920*1024)=362, step=(int)(113/1920*1024)=60,
			//   y =(int)(293/1080*648) =175, w=(int)(94/1920*1024)=50,
			//   h =(int)(133/1080*648) =79.
			if(board) {
				Rect board_rect = layout.GetAbsoluteRect(*board);
				CHECK(RectEq(board_rect, 0, 0, 1024, 648),
				      "GameTable_PS_6p.form: Board's own rect == (0,0,1024,648) "
				      "[task 0115 evidence]");

				Vector<VsmFormSubSlot> slots = VsmGetSubSlots("BoardCtrl");
				struct Expect { const char* name; int x, y, w, h; };
				static const Expect expects[] = {
					{ "board_card_0", 362, 175, 50, 79 },
					{ "board_card_1", 422, 175, 50, 79 },
					{ "board_card_2", 482, 175, 50, 79 },
					{ "board_card_3", 542, 175, 50, 79 },
					{ "board_card_4", 602, 175, 50, 79 },
				};
				for(const Expect& ex : expects) {
					const VsmFormSubSlot* slot = FindSubSlot(slots, ex.name);
					CHECK(slot != NULL, Format("BoardCtrl sub-slot '%s' is declared", ex.name));
					if(slot) {
						Rect r = VsmResolveSubSlot(*slot, board_rect);
						CHECK(RectEq(r, ex.x, ex.y, ex.w, ex.h),
						      Format("%s resolves to (%d,%d,%d,%d) "
						             "[task 0129: bit-exact against the legacy-pokertable renderer]",
						             ex.name, ex.x, ex.y, ex.w, ex.h));
					}
				}
			}

			// --- VsmBuildLayoutProfile on the PS_6p form [task 0114 evidence] ---
			VsmLayoutProfile profile = VsmBuildLayoutProfile(layout);
			CHECK(profile.elements.GetCount() == 38,
			      "VsmBuildLayoutProfile(PS_6p): exactly 38 elements [task 0114 evidence]");
			CHECK(profile.subslots.GetCount() == 95,
			      "VsmBuildLayoutProfile(PS_6p): exactly 95 sub-slots [task 0114 evidence]");

			int unknown_count = 0;
			String unknown_name;
			for(const VsmLayoutElementInfo& e : profile.elements)
				if(e.role == "unknown") {
					unknown_count++;
					unknown_name = e.name;
				}
			CHECK(unknown_count == 1,
			      Format("VsmBuildLayoutProfile(PS_6p): exactly one role==\"unknown\" element "
			             "(got %d) [task 0114 evidence]", unknown_count));
			CHECK(unknown_name == "HumanButtons",
			      Format("VsmBuildLayoutProfile(PS_6p): the sole unknown-role element is "
			             "HumanButtons (got '%s') [task 0114 evidence]", unknown_name));

			const VsmLayoutElementInfo* pot_total  = FindElementInfo(profile, "PotTotal");
			const VsmLayoutElementInfo* pot_title  = FindElementInfo(profile, "PotTitle");
			const VsmLayoutElementInfo* turn_title = FindElementInfo(profile, "TurnTitle");
			const VsmLayoutElementInfo* player0_i  = FindElementInfo(profile, "Player0");

			CHECK(pot_total != NULL && pot_total->role == "pot_amount",
			      "PotTotal -> role \"pot_amount\" [task 0114 evidence]");
			CHECK(pot_title != NULL && pot_title->role == "pot_amount",
			      "PotTitle -> role \"pot_amount\" [task 0114 evidence]");
			CHECK(turn_title != NULL && turn_title->role == "turn_label",
			      "TurnTitle -> role \"turn_label\" [task 0114 evidence]");
			CHECK(player0_i != NULL && player0_i->role == "seat" && player0_i->seat_index == 0,
			      "Player0 -> role \"seat\", seat_index==0 [task 0114 evidence]");

			// --- Region-to-element assignment + the unassigned path ---
			RunRegionAssignmentChecks(profile);
		}
	}

	// --- VsmParseFormFile on GameTable.form (the default, non-PS_6p form) ---
	{
		Vector<VsmFormLayout> layouts = VsmParseFormFile("game/TexasHoldem/GameTable.form");
		CHECK(layouts.GetCount() >= 1, "VsmParseFormFile(GameTable.form) returns >= 1 layout");

		if(layouts.GetCount() >= 1) {
			const VsmFormLayout& layout = layouts[0];
			CHECK(layout.elements.GetCount() == 38,
			      "GameTable.form: VsmParseFormFile yields exactly 38 elements "
			      "[task 0117 spec: proves the parser isn't PS_6p-count-specific]");

			const VsmFormElement* player6 = layout.Find("Player6");
			CHECK(player6 != NULL, "GameTable.form: Player6 element found");
			if(player6)
				CHECK(!RectEq(player6->GetRect(), 1, 1, 1, 1),
				      "GameTable.form: Player6's rect is NOT degenerate (1,1,1,1) like PS_6p's "
				      "-- proves the parser isn't hardcoded to PS_6p's specific geometry "
				      "[task 0117 Objective]");
		}
	}

	// --- M05-06 (task 0124): second, structurally-divergent .form fixture
	// (synthetic "PlayerCtrlMerged" seat type, merged name+action control) ---
	RunMergedNameActionFixtureChecks();

	// --- M07-01 (task 0137): live-tailing frame source ---
	Cout() << "\n--- M07-01 live-tailing frame source (task 0137) ---\n";
	RunLiveTailingCompletedFixtureCheck();
	RunLiveTailingIncrementalCheck();
	RunShaderTemplateChecks();

	// --- Error path: nonexistent .form path ---
	{
		Vector<VsmFormLayout> layouts = VsmParseFormFile("game/TexasHoldem/DoesNotExist_0117.form");
		CHECK(layouts.IsEmpty(),
		      "VsmParseFormFile on a nonexistent path returns an empty Vector<VsmFormLayout>, "
		      "no crash [task 0117 Objective]");
	}

	Cout() << "\n=== Summary ===\n";
	Cout() << g_h.passed << "/" << g_h.total << " checks passed\n";
	if(g_h.passed == g_h.total) {
		Cout() << "ALL CHECKS PASSED\n";
		SetExitCode(0);
	}
	else {
		Cout() << (g_h.total - g_h.passed) << " CHECK(S) FAILED\n";
		SetExitCode(1);
	}
}
