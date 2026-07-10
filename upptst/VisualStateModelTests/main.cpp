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

			// --- VsmGetSubSlots/VsmResolveSubSlot against the PS_6p Board rect
			// [task 0113 evidence, bit-exact vs. the legacy renderer] ---
			if(board) {
				Rect board_rect = layout.GetAbsoluteRect(*board);
				CHECK(RectEq(board_rect, 0, 0, 1024, 648),
				      "GameTable_PS_6p.form: Board's own rect == (0,0,1024,648) "
				      "[task 0115 evidence]");

				Vector<VsmFormSubSlot> slots = VsmGetSubSlots("BoardCtrl");
				struct Expect { const char* name; int x, y, w, h; };
				static const Expect expects[] = {
					{ "board_card_0", 341, 209, 65, 85 },
					{ "board_card_1", 410, 209, 65, 85 },
					{ "board_card_2", 479, 209, 65, 85 },
					{ "board_card_3", 548, 209, 65, 85 },
					{ "board_card_4", 617, 209, 65, 85 },
				};
				for(const Expect& ex : expects) {
					const VsmFormSubSlot* slot = FindSubSlot(slots, ex.name);
					CHECK(slot != NULL, Format("BoardCtrl sub-slot '%s' is declared", ex.name));
					if(slot) {
						Rect r = VsmResolveSubSlot(*slot, board_rect);
						CHECK(RectEq(r, ex.x, ex.y, ex.w, ex.h),
						      Format("%s resolves to (%d,%d,%d,%d) "
						             "[task 0113 evidence, bit-exact against the legacy renderer]",
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
