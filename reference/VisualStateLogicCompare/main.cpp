#include <CtrlLib/CtrlLib.h>
#include <TexasHoldem/TexasHoldemSessionContract.h>
#include <TexasHoldem/TexasHoldemLogicState.h>
#include <VisualStateModel/VisualStateModel.h>
#include <CardRender/CardRender.h>
#include <VisualStateLogic/LogicCompare.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// M06-03 (task 0133): the scoring/derivation logic this file used to define
// as file-local `static` functions (tasks 0119-0128: VsmScorePuckRoles,
// VsmSearchPuckRecovery, ApplyDealerButtonObservations/
// DeriveDealerSeatPerFrame, VsmScoreCardSlot/VsmEmpiricalBoardCardRect/
// DeriveBoardCardsPerFrame, VsmScoreActionIcon/VsmActionIconRowParityOffset/
// DeriveActionIconsPerFrame, VsmScoreHoleCardSlot/VsmHoleCardRowParityOffset/
// DeriveHoleCardsPerFrame, and their supporting observation/probe-attempt
// structs + threshold constants) has been extracted VERBATIM into the shared
// `uppsrc/VisualStateLogic/LogicCompare.h/.cpp` library (see that header's
// own comment for why it is a new sibling package rather than merged into
// `uppsrc/VisualStateModel` — a real dependency-layering reason, not
// arbitrary), so `reference/VisualStateWorkbench`'s logic-state timeline
// panel can call the exact same recognition/derivation code this CLI uses,
// without duplicating it — the same "shared library, single implementation"
// shape task 0117 established for M04's RegionAssign.h. This file now only
// calls those library functions; every line of orchestration/printing below
// (the per-transition loop, the disambiguation/acceptance-gate tables, the
// self-test drivers) is UNCHANGED from before the extraction — proven
// byte-for-byte identical output on the six kept M05 fixtures, see task
// 0133's evidence section.

// ---------------------------------------------------------------------------
// Task 0119 (M05-01): logic-state schema scaffold + dealer-button ground-
// truth comparator.
//
// Combines three already-built pieces, as library calls (not by shelling out
// to another CLI and parsing its stdout):
//   - TexasHoldemGroundTruthRecord::Jsonize (game/TexasHoldem/
//     TexasHoldemSessionContract.h) to load groundtruth.jsonl - the real
//     M01-M04 format, NOT the older uppsrc/VisualStateModel/GroundTruth.h
//     `.vsm.json` prototype (see this task's Manager task file "Reuse
//     decision" section for why that prototype is deliberately not used).
//   - uppsrc/VisualStateModel's VsmDetectChanges/VsmRegionMemory (M03) +
//     VsmBuildLayoutProfile/VsmMatchRegion/VsmBuildCandidates (M04, task
//     0117's RegionAssign.h extraction) - the exact same region-to-layout-
//     element matching path reference/VisualStateLayoutAssign/main.cpp uses.
//   - A new dealer-seat derivation + comparator layer (this task).
//
// Dealer-seat derivation: game/TexasHoldem/GameTable.cpp:1570-1581 draws a
// puck image on the "button_puck" sub-slot (LayoutProfile.cpp:164-165 role
// "dealer_button") only for a seat whose GBUTTON_DEALER/SMALL_BLIND/BIG_BLIND
// state is non-zero; ground truth carries the same fact per-frame as
// TexasHoldemPlayerSnapshot::button (1 == GBUTTON_DEALER, game/GameRules/
// GameDefs.h:280). So: whenever a changed region gets matched to some
// Player<N>.button_puck sub-slot between frame_prev and frame, this tool
// treats seat N as the dealer from `frame` onward, until a different seat's
// button_puck sub-slot changes.
//
// IMPORTANT EMPIRICAL FINDING (see this task's evidence section in the
// Manager task file for the full pixel-level investigation): across BOTH
// var/vsm_fixtures/texas_ps6p_sample and .../texas_ps6p_seed5_frames20 -
// spanning 7 and 19 frame transitions respectively, including 1 and 3 hand
// boundaries where ground truth's dealer seat actually moves - NOT ONE
// button_puck sub-slot ever shows a nonzero pixel change. A dedicated pixel-
// diff probe (compares the exact button_puck rect between every consecutive
// frame pair directly, independent of change-detection thresholds) confirmed
// zero difference in every channel, every pixel, every transition. So in the
// CURRENT recorded fixtures the dealer-button vision signal this task
// derives is real, correctly wired, and dormant: `derived_dealer_seat_known`
// is false for every single frame in both fixtures, not because of a bug in
// the matching/derivation code but because the puck graphic itself never
// renders a visually distinguishing pixel in this headless recording
// environment (most likely explanation: GameTable.cpp:1571-1578's
// `StreamRaster::LoadFileAny(dataDir + "gfx/...png")` fallback silently
// fails to a Null image when those asset files aren't present in the
// recorder's working directory, and stays Null/identical every frame either
// way). This is a genuine finding about the CURRENT fixtures, not a
// limitation of the approach - see --self-test below for direct proof the
// derivation logic itself is correct, independent of whether today's
// fixtures happen to exercise it.
//
// Because zero real button_puck observations exist in the current fixture
// data, "appear vs. disappear" could not be empirically distinguished here
// either (see the same evidence section) - the code below treats ANY
// changed region matched to a seat's button_puck sub-slot (append or
// disappear alike - VsmChangedRect carries no such distinction, only "this
// region's content changed") as "the dealer moved to this seat", taking the
// LAST such observation within one transition if more than one seat's
// button_puck sub-slot changes in the same transition (e.g. a full dealer/
// SB/BB rotation) - see the code comment at ApplyDealerButtonObservations
// for why this specific tie-break can't be improved on without OCR/template-
// matching (explicitly out of scope, see this task's guardrails).


static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}





// Fixture-independent correctness check of DeriveActionIconsPerFrame's sticky/
// reset state machine, exercised with SYNTHETIC observations (mirroring
// RunSelfTest's own dealer-seat synthetic check below) - needed because, per
// this task's evidence section, no genuine real-action recognition is
// possible in THIS checkout (no action-icon PNG assets exist), so this is the
// only way to verify the state machine's own logic (not the vision scoring,
// which is separately, honestly reported as untestable here) is correct.
static bool RunActionIconSelfTest()
{
	Cout() << "=== --self-test: synthetic action-icon sticky/reset check ===\n\n";
	// Seat 0: observed CALL at frame 2 (sticky through frame 4), then an
	// empty/background match at frame 5 (reset - frames 5+ unknown until the
	// next real observation), then FOLD at frame 8 (sticky to the end).
	// Seat 1: observed WINNER(9) at frame 6 (sticky, but must never compare
	// to ground truth - verified by the caller, this function only checks
	// the sticky VALUE/known flag are correct).
	Vector<VsmActionIconObservation> synthetic;
	{ VsmActionIconObservation o; o.frame = 2; o.seat = 0; o.value = 2; synthetic.Add(o); } // CALL
	{ VsmActionIconObservation o; o.frame = 5; o.seat = 0; o.value = kActionIconEmptyWinnerValue; synthetic.Add(o); }
	{ VsmActionIconObservation o; o.frame = 8; o.seat = 0; o.value = 5; synthetic.Add(o); } // FOLD
	{ VsmActionIconObservation o; o.frame = 6; o.seat = 1; o.value = kActionIconVocabWinner; synthetic.Add(o); }

	Vector<int> seats; seats.Add(0); seats.Add(1);
	VectorMap<int, Vector<bool>> known;
	VectorMap<int, Vector<int>>  value;
	DeriveActionIconsPerFrame(synthetic, 0, 9, seats, known, value);

	bool ok = true;
	auto Check = [&](int seat, int fid, bool exp_known, int exp_value) {
		bool got_known = known.Get(seat)[fid];
		int  got_value = value.Get(seat)[fid];
		bool row_ok = (got_known == exp_known) && (!exp_known || got_value == exp_value);
		if(!row_ok) ok = false;
		Cout() << "  seat " << seat << " frame " << fid
		       << " expected=" << (exp_known ? IntStr(exp_value) : String("unknown"))
		       << " got=" << (got_known ? IntStr(got_value) : String("unknown"))
		       << (row_ok ? "  OK" : "  MISMATCH") << "\n";
	};
	for(int fid = 0; fid <= 9; fid++) {
		bool exp_known = (fid >= 2 && fid < 5) || fid >= 8;
		int  exp_value = fid < 5 ? 2 : 5;
		Check(0, fid, exp_known, exp_value);
	}
	for(int fid = 0; fid <= 9; fid++)
		Check(1, fid, fid >= 6, kActionIconVocabWinner);

	Cout() << "\n--self-test (action-icon) " << (ok ? "PASS" : "FAIL") << "\n";
	return ok;
}

// Fixture-independent correctness check of DeriveHoleCardsPerFrame's own
// sticky/reset state machine (mirrors RunActionIconSelfTest's own synthetic
// check, task 0127) - added per this task's guardrail ("follow 0127's
// precedent of adding a synthetic self-test case if real recognition data
// alone doesn't exercise every path, e.g. the reset-across-hand-boundary
// transition"): even though this task DID get real positive-match data for
// both vocabulary categories (see evidence section), a real fixture is not
// guaranteed to exercise a full reveal-then-next-hand-hidden-again round
// trip for a NON-hero seat within a short 30-frame recording, so this
// synthetic check exercises that transition directly and deterministically.
static bool RunHoleCardSelfTest()
{
	Cout() << "=== --self-test: synthetic hole-card sticky/reset check ===\n\n";
	// Seat 2, slot 0: back at frame 1 (dealt face-down), revealed as card 10
	// at frame 6 (showdown), reset back to -1 (back) at frame 9 (next hand's
	// preflop deal) - the reveal-then-next-hand-hidden-again round trip.
	// Seat 0 (hero), slot 1: goes straight from "not yet observed" to a real
	// card at frame 3 (never shows "back" - by construction, no special-case
	// code needed, see this section's header comment), then a NEW real card
	// at frame 8 (next hand) - hero's slot just keeps updating card-to-card,
	// no reset step in between, matching real play.
	Vector<VsmHoleCardObservation> synthetic;
	{ VsmHoleCardObservation o; o.frame = 1; o.seat = 2; o.card_index = 0; o.value = -1; synthetic.Add(o); }
	{ VsmHoleCardObservation o; o.frame = 6; o.seat = 2; o.card_index = 0; o.value = 10; synthetic.Add(o); }
	{ VsmHoleCardObservation o; o.frame = 9; o.seat = 2; o.card_index = 0; o.value = -1; synthetic.Add(o); }
	{ VsmHoleCardObservation o; o.frame = 3; o.seat = 0; o.card_index = 1; o.value = 22; synthetic.Add(o); }
	{ VsmHoleCardObservation o; o.frame = 8; o.seat = 0; o.card_index = 1; o.value = 41; synthetic.Add(o); }

	Vector<int> seats; seats.Add(0); seats.Add(2);
	VectorMap<int, Vector<bool>> known[2];
	VectorMap<int, Vector<int>>  value[2];
	DeriveHoleCardsPerFrame(synthetic, 0, 11, seats, known, value);

	bool ok = true;
	auto Check = [&](int slot, int seat, int fid, bool exp_known, int exp_value) {
		bool got_known = known[slot].Get(seat)[fid];
		int  got_value = value[slot].Get(seat)[fid];
		bool row_ok = (got_known == exp_known) && (!exp_known || got_value == exp_value);
		if(!row_ok) ok = false;
		Cout() << "  slot " << slot << " seat " << seat << " frame " << fid
		       << " expected=" << (exp_known ? IntStr(exp_value) : String("unknown"))
		       << " got=" << (got_known ? IntStr(got_value) : String("unknown"))
		       << (row_ok ? "  OK" : "  MISMATCH") << "\n";
	};
	for(int fid = 0; fid <= 11; fid++) {
		bool exp_known = fid >= 1;
		int  exp_value = fid < 6 ? -1 : (fid < 9 ? 10 : -1);
		Check(0, 2, fid, exp_known, exp_value);
	}
	for(int fid = 0; fid <= 11; fid++) {
		bool exp_known = fid >= 3;
		int  exp_value = fid < 8 ? 22 : 41;
		Check(1, 0, fid, exp_known, exp_value);
	}

	Cout() << "\n--self-test (hole-card) " << (ok ? "PASS" : "FAIL") << "\n";
	return ok;
}

// ---------------------------------------------------------------------------
static bool RunSelfTest()
{
	Cout() << "=== --self-test: synthetic dealer-seat derivation check ===\n\n";
	// Synthetic sequence, independent of any real fixture: frames 0..9,
	// dealer_button-role observations only at two transitions - mimics a
	// single-seat "appears" signal at frame 3 (dealer moves to seat 2), then
	// a same-transition double signal at frame 7 (seat 2's puck disappears
	// AND seat 4's appears - the tie-break picks the LAST one in observation
	// order, i.e. seat 4, per ApplyDealerButtonObservations' documented
	// last-wins rule).
	Vector<VsmLayoutObservationOut> synthetic;
	{
		VsmLayoutObservationOut o;
		o.role = "dealer_button"; o.frame_prev = 2; o.frame = 3; o.seat_index = 2;
		synthetic.Add(o);
	}
	{
		VsmLayoutObservationOut o;
		o.role = "dealer_button"; o.frame_prev = 6; o.frame = 7; o.seat_index = 2; // old dealer's puck disappearing
		synthetic.Add(o);
	}
	{
		VsmLayoutObservationOut o;
		o.role = "dealer_button"; o.frame_prev = 6; o.frame = 7; o.seat_index = 4; // new dealer's puck appearing
		synthetic.Add(o);
	}
	// A non-dealer_button observation in between, must be ignored.
	{
		VsmLayoutObservationOut o;
		o.role = "stack_text"; o.frame_prev = 4; o.frame = 5; o.seat_index = 1;
		synthetic.Add(o);
	}

	VectorMap<int, int> dealer_seat_by_frame;
	ApplyDealerButtonObservations(synthetic, dealer_seat_by_frame);

	Vector<bool> known;
	Vector<int> seat;
	DeriveDealerSeatPerFrame(dealer_seat_by_frame, 0, 9, known, seat);

	// Expected: frames 0-2 unknown; frames 3-6 seat 2; frames 7-9 seat 4
	// (last-observation-in-transition wins at frame 7).
	bool ok = true;
	for(int fid = 0; fid <= 9; fid++) {
		bool exp_known = fid >= 3;
		int  exp_seat  = fid < 3 ? -1 : (fid < 7 ? 2 : 4);
		bool got_known = known[fid];
		int  got_seat  = seat[fid];
		bool row_ok = (got_known == exp_known) && (!exp_known || got_seat == exp_seat);
		if(!row_ok)
			ok = false;
		Cout() << "  frame " << fid
		       << " expected=" << (exp_known ? IntStr(exp_seat) : String("unknown"))
		       << " got=" << (got_known ? IntStr(got_seat) : String("unknown"))
		       << (row_ok ? "  OK" : "  MISMATCH") << "\n";
	}

	Cout() << "\n--self-test " << (ok ? "PASS" : "FAIL") << "\n";
	return ok;
}

// NOTE: this needs GUI_APP_MAIN, not CONSOLE_APP_MAIN, even though it never
// shows a window and behaves as a plain stdout CLI tool: `uses TexasHoldem`
// pulls in CtrlLib/Form (TexasHoldem's GameTable.cpp etc. are Ctrl-derived),
// and CtrlCore.h hard-errors ("CtrlCore should not be included without GUI
// flag") unless this package's mainconfig sets the global "GUI" build flag.
// Once that flag is set, MscBuilder.cpp links the whole binary with
// `-subsystem:windows`, which requires a WinMain-shaped entry point -
// CONSOLE_APP_MAIN's `main()` won't link. Same reasoning, same fix,
// as reference/CardGameStateAdapter/main.cpp (see that file's own comment):
// GUI_APP_MAIN still runs fine as an ordinary command-line tool with working
// Cout()/stdout.
GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();

	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--self-test") {
			bool ok = RunSelfTest();
			// M05-09 (task 0127): both self-tests must pass for --self-test
			// to report PASS overall - printed one after the other, same as
			// running two independent fixture-free checks in one invocation.
			bool ok2 = RunActionIconSelfTest();
			// M05-10 (task 0128): a third independent, fixture-free check -
			// all three must pass for --self-test to report PASS overall.
			bool ok3 = RunHoleCardSelfTest();
			SetExitCode((ok && ok2 && ok3) ? 0 : 1);
			return;
		}
	}

	String session_dir;
	String form_path;
	String jsonl_out;

	Vector<String> positional;
	for(int i = 0; i < args.GetCount(); i++) {
		const String& arg = args[i];
		if(arg == "--help") {
			Cout() << "Usage: VisualStateLogicCompare <m01m02_session_dir> <path-to-.form> "
			          "[--jsonl-out <path>]\n"
			          "       VisualStateLogicCompare --self-test\n"
			          "  <m01m02_session_dir>  M01/M02 session directory (metadata.json +\n"
			          "                        groundtruth.jsonl + frames/%08d.png), e.g.\n"
			          "                        var/vsm_fixtures/texas_ps6p_sample\n"
			          "  <path-to-.form>       .form layout file, e.g.\n"
			          "                        game/TexasHoldem/GameTable_PS_6p.form\n"
			          "  --jsonl-out <path>    write one JSON comparator record per frame to <path>\n"
			          "  --self-test           run a synthetic (fixture-independent) check of the\n"
			          "                        dealer-seat derivation logic itself and exit\n";
			SetExitCode(0);
			return;
		}
		else if(arg == "--jsonl-out") {
			if(i + 1 >= args.GetCount()) { Fail("--jsonl-out requires a value"); return; }
			jsonl_out = args[++i];
		}
		else {
			positional.Add(arg);
		}
	}

	if(positional.GetCount() < 2) {
		Cout() << "Usage: VisualStateLogicCompare <m01m02_session_dir> <path-to-.form> "
		          "[--jsonl-out <path>]\n(pass --help for details)\n";
		SetExitCode(1);
		return;
	}
	session_dir = positional[0];
	form_path   = positional[1];

	Cout() << "=== VisualStateModel Logic Compare (M05-01 dealer-button slice) ===\n\n";

	if(!DirectoryExists(session_dir)) {
		Fail(Format("Session directory not found: %s", session_dir));
		return;
	}

	// --- Load ground truth (reuse TexasHoldemGroundTruthRecord::Jsonize -
	// NOT a hand-rolled second parser). ---
	String gt_path = AppendFileName(session_dir, "groundtruth.jsonl");
	if(!FileExists(gt_path)) {
		Fail(Format("Missing groundtruth.jsonl under: %s", session_dir));
		return;
	}
	Vector<TexasHoldemGroundTruthRecord> gt_records;
	{
		Vector<String> rows = Split(LoadFile(gt_path), '\n', false);
		for(String row : rows) {
			row = TrimBoth(row);
			if(row.IsEmpty())
				continue;
			TexasHoldemGroundTruthRecord rec;
			if(!LoadFromJson(rec, row)) {
				Fail(Format("Failed to parse groundtruth.jsonl row %d", gt_records.GetCount()));
				return;
			}
			gt_records.Add(pick(rec));
		}
	}
	if(gt_records.IsEmpty()) {
		Fail("groundtruth.jsonl has no rows");
		return;
	}
	Cout() << "Loaded " << gt_records.GetCount() << " ground truth record(s) from " << gt_path << "\n";

	VsmM01M02SessionInfo info;
	if(!VsmReadM01M02SessionInfo(session_dir, info)) {
		Fail(Format("Failed to read M01/M02 session metadata.json under: %s", session_dir));
		return;
	}
	Cout() << "Session: provider=" << info.provider
	       << " session_id=" << info.session_id
	       << " metadata size=" << info.table_width << "x" << info.table_height
	       << " frame_count=" << info.frame_count << "\n";
	if(info.frame_count != gt_records.GetCount())
		Cout() << "  NOTE: metadata.json frame_count (" << info.frame_count
		       << ") != groundtruth.jsonl row count (" << gt_records.GetCount() << ")\n";
	if(info.frame_count < 2) {
		Fail("Session has fewer than 2 frames - no transitions to detect");
		return;
	}

	// --- Load the .form layout profile (same path VisualStateLayoutAssign uses). ---
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

	VsmFrameImage probe_frame;
	if(!VsmLoadM01M02SessionFrame(session_dir, 0, probe_frame)) {
		Fail("Failed to decode frame 0");
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
	       << " sy=" << DblStr(sy) << "\n\n";

	Vector<VsmLayoutCandidate> candidates = VsmBuildCandidates(profile, sx, sy);
	Cout() << "Built " << candidates.GetCount() << " match candidate(s) "
	       << "(" << profile.elements.GetCount() << " element(s) + "
	       << profile.subslots.GetCount() << " sub-slot(s))\n\n";

	// M05-04 (task 0122): the per-seat button_puck sub-slot candidates the
	// recovery pass probes - built once here (candidates/rects don't change
	// per-frame), same list the normal VsmMatchRegion path above already
	// matches against.
	Vector<const VsmLayoutCandidate*> puck_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "dealer_button")
			puck_candidates.Add(&c);

	// M05-08 (task 0126): the 5 board_card_N sub-slot candidates, used below
	// both for the frame-0 initial seed pass and for reference by index while
	// printing/deriving.
	Vector<const VsmLayoutCandidate*> board_card_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "board_card")
			board_card_candidates.Add(&c);

	Vector<VsmBoardCardObservation> board_card_observations;

	// M05-09 (task 0127): the per-seat action_icon sub-slot candidates - built
	// once here, same pattern as puck/board_card above. (VsmScoreActionIcon's
	// empty-background stripe-phase calculation uses an empirically-measured
	// per-seat table, not this candidate list - see
	// VsmActionIconRowParityOffset's doc comment for why.)
	Vector<const VsmLayoutCandidate*> action_icon_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "action_icon")
			action_icon_candidates.Add(&c);

	// M05-10 (task 0128): the per-seat, per-slot hole_card_0/hole_card_1
	// sub-slot candidates - built once here, same pattern as the three roles
	// above.
	Vector<const VsmLayoutCandidate*> hole_card_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "hole_card")
			hole_card_candidates.Add(&c);

	Vector<VsmActionIconObservation> action_icon_observations;
	Vector<VsmHoleCardObservation> hole_card_observations;

	// M05-08 (task 0126): frame-0 initial seed pass. Unlike the dealer-button
	// signal (which only ever "appears" and is correctly left "unknown" at
	// frame 0 per DeriveDealerSeatPerFrame's own doc comment - there's no
	// incoming transition to observe yet), a board_card slot's HOLDER
	// reference is already visibly on screen from frame 0 (every slot shows
	// its holder at preflop, GameTable.cpp:1276-1288's `boardCards[i]>=0 ?
	// card : holder` choice), and it only ever gets a change-detected
	// observation later when a card is FIRST dealt onto it - the initial
	// "empty" state is otherwise never on the `changes` list at all (nothing
	// changed to produce it, it was already there in frame 0). Directly
	// scoring frame 0's own board_card_N rects here (not gated by change
	// detection) is what makes board_cards_known become true almost
	// immediately, exactly as task 0126 expects ("un-dealt slots resolve to
	// empty immediately").
	if(!board_card_candidates.IsEmpty()) {
		Cout() << "=== Board-card template match: frame-0 initial seed ===\n";
		Cout() << Format("%-6s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "slot", "best", "runnerup", "margin", "winner", "accepted?");
		Image frame0_img = VsmFrameImageToImage(probe_frame);
		for(const VsmLayoutCandidate* c : board_card_candidates) {
			double scores[kCardVocabSize];
			int winner = -1;
			if(!VsmScoreCardSlot(frame0_img, c->rect, c->card_index, scores, winner))
				continue;
			double runnerup = DBL_MAX;
			for(int i = 0; i < kCardVocabSize; i++)
				if(i != winner && scores[i] < runnerup)
					runnerup = scores[i];
			double margin = runnerup - scores[winner];
			bool accept = margin >= kCardMatchMinMargin;
			String winner_str = (winner == kCardHolderVocabIndex) ? String("holder") : IntStr(winner);
			Cout() << Format("%-6d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				0, c->card_index, DblStr(scores[winner]), DblStr(runnerup), DblStr(margin),
				winner_str, accept ? "accepted" : "rejected");
			if(accept) {
				VsmBoardCardObservation bco;
				bco.frame = 0;
				bco.card_index = c->card_index;
				bco.value = (winner == kCardHolderVocabIndex) ? -1 : winner;
				board_card_observations.Add(bco);
			}
		}
		Cout() << "\n";
	}

	// M05-10 (task 0128): frame-0 initial seed pass for hole cards, same
	// rationale as the board-card seed pass immediately above - whatever a
	// seat's hole_card_0/1 slots show at frame 0 (already-dealt "back", an
	// already-dealt real card for hero, or the genuine pre-deal blank state
	// - see this task's evidence section for which of these frame 0 actually
	// is in this task's own fixtures) is scored directly here, not gated by
	// change detection (nothing "changed" to produce frame 0's own content).
	if(!hole_card_candidates.IsEmpty()) {
		Cout() << "=== Hole-card template match: frame-0 initial seed ===\n";
		Cout() << Format("%-6s %-5s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "slot", "best", "runnerup", "margin", "winner", "accepted?");
		Image frame0_img = VsmFrameImageToImage(probe_frame);
		for(const VsmLayoutCandidate* c : hole_card_candidates) {
			double scores[kHoleCardVocabSize];
			int winner = -1;
			if(!VsmScoreHoleCardSlot(frame0_img, c->rect, c->seat_index, scores, winner))
				continue;
			double runnerup = DBL_MAX;
			for(int i = 0; i < kHoleCardVocabSize; i++)
				if(i != winner && scores[i] < runnerup)
					runnerup = scores[i];
			double margin = runnerup - scores[winner];
			bool accept = margin >= kHoleCardMatchMinMargin;
			String winner_str = (winner == kHoleCardBackVocabIndex) ? String("back") : IntStr(winner);
			Cout() << Format("%-6d %-5d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				0, c->seat_index, c->card_index, DblStr(scores[winner]), DblStr(runnerup), DblStr(margin),
				winner_str, accept ? "accepted" : "rejected");
			if(accept) {
				VsmHoleCardObservation hco;
				hco.frame = 0;
				hco.seat = c->seat_index;
				hco.card_index = c->card_index;
				hco.value = (winner == kHoleCardBackVocabIndex) ? -1 : winner;
				hole_card_observations.Add(hco);
			}
		}
		Cout() << "\n";
	}

	// --- Region detection across every transition, same params as
	// reference/VisualStateLayoutAssign. ---
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

	// M05-04 (task 0122): recovery-pass state, filled in during the
	// transition loop below, printed/merged after it (mirrors how the
	// existing `observations` vector is built during the loop and processed
	// after it).
	Vector<VsmPuckRecoveryAttempt>  recovery_attempts;
	Vector<VsmLayoutObservationOut> recovered_observations;
	int rescue_counter = 0;

	// M05-08 (task 0126): change-triggered board_card_N direct-probe state
	// (see the loop below, right after the puck recovery pass, for the full
	// design rationale - added because real recordings show board-card
	// reveals essentially never win a role=="board_card" observation via
	// the normal VsmMatchRegion path; this is this task's actual working
	// detection mechanism, verified against real fixtures, see the Manager
	// task file's evidence section).
	Vector<VsmBoardCardProbeAttempt> board_card_probe_attempts;

	// M05-09 (task 0127): change-triggered action_icon direct-probe state,
	// same pattern/rationale as board_card_probe_attempts immediately above.
	Vector<VsmActionIconProbeAttempt> action_icon_probe_attempts;

	// M05-10 (task 0128): change-triggered hole_card direct-probe state, same
	// pattern/rationale as action_icon_probe_attempts/board_card_probe_attempts
	// immediately above.
	Vector<VsmHoleCardProbeAttempt> hole_card_probe_attempts;

	VsmFrameImage prev_frame;
	prev_frame.Set(probe_frame.width, probe_frame.height, nullptr);
	memcpy(prev_frame.data, probe_frame.data, (size_t)probe_frame.width * probe_frame.height * 4);

	for(int fid = 1; fid < info.frame_count; fid++) {
		VsmFrameImage curr_frame;
		if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr_frame)) {
			Fail(Format("Failed to decode frame %d", fid));
			return;
		}

		Vector<VsmChangedRect> changes = VsmDetectChanges(prev_frame, curr_frame, params);
		// Lazily converted only if this frame's transition has at least one
		// dealer_button-role candidate (avoids the RGBA->Image conversion
		// cost on every frame for a signal that's rare in practice).
		Image curr_frame_img;
		bool curr_frame_img_ready = false;
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

			// M05-03 (task 0121): template-match disambiguation. Only
			// dealer_button-role candidates need scoring (that's the only
			// role ambiguous between dealer/SB/BB - see this file's header
			// comment).
			if(obs.role == "dealer_button") {
				if(!curr_frame_img_ready) {
					curr_frame_img = VsmFrameImageToImage(curr_frame);
					curr_frame_img_ready = true;
				}
				double scores[3];
				if(VsmScorePuckRoles(curr_frame_img, region_rect, scores)) {
					obs.puck_scored       = true;
					obs.puck_score_dealer = scores[0];
					obs.puck_score_sb     = scores[1];
					obs.puck_score_bb     = scores[2];
					int winner = 0;
					for(int r = 1; r < 3; r++)
						if(scores[r] < scores[winner])
							winner = r;
					obs.puck_role_winner = winner;
				}
			}

			// M05-08 (task 0126): board-card template-match scoring. Only
			// board_card-role candidates need this (see VsmScoreCardSlot's
			// doc comment) - scores the region's OWN detected rect
			// (region_rect), not the theoretical candidate rect, same
			// "score what was actually observed" approach the
			// dealer_button branch above uses.
			if(obs.role == "board_card") {
				if(!curr_frame_img_ready) {
					curr_frame_img = VsmFrameImageToImage(curr_frame);
					curr_frame_img_ready = true;
				}
				double scores[kCardVocabSize];
				int winner = -1;
				if(VsmScoreCardSlot(curr_frame_img, region_rect, obs.card_index, scores, winner)) {
					double runnerup = DBL_MAX;
					for(int i = 0; i < kCardVocabSize; i++)
						if(i != winner && scores[i] < runnerup)
							runnerup = scores[i];
					obs.card_scored         = true;
					obs.card_winner         = winner;
					obs.card_score_best     = scores[winner];
					obs.card_score_runnerup = runnerup;
				}
			}

			// M05-09 (task 0127): action-icon template-match scoring. Only
			// action_icon-role candidates need this (see VsmScoreActionIcon's
			// doc comment).
			if(obs.role == "action_icon") {
				if(!curr_frame_img_ready) {
					curr_frame_img = VsmFrameImageToImage(curr_frame);
					curr_frame_img_ready = true;
				}
				double scores[kActionIconRealVocabCount + 1];
				int winner = -2;
				if(VsmScoreActionIcon(curr_frame_img, region_rect, obs.seat_index, scores, winner)) {
					double runnerup = DBL_MAX;
					int winner_index = -1;
					for(int i = 0; i <= kActionIconRealVocabCount; i++) {
						int val = (i == kActionIconRealVocabCount) ? kActionIconEmptyWinnerValue : kActionIconRealVocab[i];
						if(val == winner)
							winner_index = i;
					}
					for(int i = 0; i <= kActionIconRealVocabCount; i++)
						if(i != winner_index && scores[i] < runnerup)
							runnerup = scores[i];
					obs.action_icon_scored         = true;
					obs.action_icon_winner         = winner;
					obs.action_icon_score_best     = scores[winner_index];
					obs.action_icon_score_runnerup = runnerup;
				}
			}

			// M05-10 (task 0128): hole-card template-match scoring. Only
			// hole_card-role candidates need this (see VsmScoreHoleCardSlot's
			// doc comment).
			if(obs.role == "hole_card") {
				if(!curr_frame_img_ready) {
					curr_frame_img = VsmFrameImageToImage(curr_frame);
					curr_frame_img_ready = true;
				}
				double scores[kHoleCardVocabSize];
				int winner = -1;
				if(VsmScoreHoleCardSlot(curr_frame_img, region_rect, obs.seat_index, scores, winner)) {
					double runnerup = DBL_MAX;
					for(int i = 0; i < kHoleCardVocabSize; i++)
						if(i != winner && scores[i] < runnerup)
							runnerup = scores[i];
					obs.hole_card_scored         = true;
					obs.hole_card_winner         = winner;
					obs.hole_card_score_best     = scores[winner];
					obs.hole_card_score_runnerup = runnerup;
				}
			}

			observations.Add(obs);
		}

		// M05-04 (task 0122): additive recovery pass for THIS transition,
		// using the exact same `changes` list above (trigger condition
		// only - no new detection). See the file-header comment above
		// VsmSearchPuckRecovery for the full design rationale.
		for(const VsmLayoutCandidate* pc : puck_candidates) {
			Rect padded = pc->rect.Inflated(kPuckRecoveryPadding);
			bool close_enough = false;
			for(const VsmChangedRect& cr : changes) {
				if(!(padded & cr.GetRect()).IsEmpty()) {
					close_enough = true;
					break;
				}
			}
			if(!close_enough)
				continue;

			if(!curr_frame_img_ready) {
				curr_frame_img = VsmFrameImageToImage(curr_frame);
				curr_frame_img_ready = true;
			}

			VsmPuckRecoveryAttempt attempt;
			attempt.frame = fid;
			attempt.seat  = pc->seat_index;
			VsmSearchPuckRecovery(curr_frame_img, pc->rect, attempt);
			recovery_attempts.Add(attempt);

			if(attempt.found && attempt.recovered) {
				int base_w = pc->rect.Width(), base_h = pc->rect.Height();
				int ccx = pc->rect.left + base_w / 2;
				int ccy = pc->rect.top  + base_h / 2;
				int w = (int)(base_w * attempt.scale + 0.5);
				int h = (int)(base_h * attempt.scale + 0.5);

				VsmLayoutObservationOut obs;
				obs.frame_prev = fid - 1;
				obs.frame      = fid;
				obs.x = ccx - w / 2 + attempt.dx;
				obs.y = ccy - h / 2 + attempt.dy;
				obs.w = w; obs.h = h;
				obs.score     = attempt.score_dealer;
				obs.region_id = Format("puck-rescue-%04d", ++rescue_counter);
				obs.assigned  = pc->label;
				obs.kind      = "subslot";
				obs.role      = "dealer_button";
				obs.seat_index = pc->seat_index;
				obs.card_index  = pc->card_index;
				obs.overlap     = 1.0; // n/a for a recovered observation; sentinel

				// Already scored/disambiguated by construction (only
				// recovered when the dealer reference wins) - flows into
				// the SAME ApplyDealerButtonObservations path 0121 wired
				// up, unchanged, via the disambiguation loop below.
				obs.puck_scored       = true;
				obs.puck_score_dealer = attempt.score_dealer;
				obs.puck_score_sb     = attempt.score_sb;
				obs.puck_score_bb     = attempt.score_bb;
				obs.puck_role_winner  = 0;

				recovered_observations.Add(obs);
			}
		}

		// M05-08 (task 0126): change-triggered direct probe of each
		// board_card_N candidate rect - added because real recordings (see
		// this task's evidence section) show board-card reveals essentially
		// NEVER produce a role=="board_card" observation via the normal
		// VsmMatchRegion path above: a card reveal's real pixel delta tends
		// to fall a few pixels short of clearing kOverlapThreshold for its
		// OWN board_card_N rect, or gets matched to a nearby/adjacent
		// changed region assigned to a completely different sub-slot/
		// element instead - the tiny board_card_N rect essentially never
		// wins purely on VsmMatchRegion's geometry. Trigger condition
		// mirrors 0122's puck-recovery trigger (any non-empty intersection
		// between the candidate rect and this transition's raw
		// VsmDetectChanges rects - same "a real change happened somewhere
		// near here" gate, not a per-frame blind scan), but - unlike 0122's
		// puck rescue - this does NOT need a multi-scale/position search
		// grid: each board_card_N rect's on-screen position is already
		// known exactly from the .form-derived candidate list (no puck-
		// style jitter/occlusion premise applies here), so this scores the
		// theoretical candidate rect directly. A "spurious" trigger (a
		// nearby unrelated change happens to touch this rect but the slot's
		// own content didn't actually change) is harmless: it just re-
		// scores whatever is ALREADY showing in that rect and re-confirms
		// the same sticky value, since VsmScoreCardSlot always scores the
		// CURRENT frame's real pixel content against the full 53-way
		// vocabulary, not a guess about what changed.
		for(const VsmLayoutCandidate* c : board_card_candidates) {
			bool close_enough = false;
			for(const VsmChangedRect& cr : changes) {
				if(!(c->rect & cr.GetRect()).IsEmpty()) {
					close_enough = true;
					break;
				}
			}
			if(!close_enough)
				continue;

			if(!curr_frame_img_ready) {
				curr_frame_img = VsmFrameImageToImage(curr_frame);
				curr_frame_img_ready = true;
			}

			VsmBoardCardProbeAttempt attempt;
			attempt.frame = fid;
			attempt.card_index = c->card_index;
			double scores[kCardVocabSize];
			int winner = -1;
			if(VsmScoreCardSlot(curr_frame_img, c->rect, c->card_index, scores, winner)) {
				double runnerup = DBL_MAX;
				for(int i = 0; i < kCardVocabSize; i++)
					if(i != winner && scores[i] < runnerup)
						runnerup = scores[i];
				attempt.scored         = true;
				attempt.winner         = winner;
				attempt.score_best     = scores[winner];
				attempt.score_runnerup = runnerup;
				attempt.accepted       = (runnerup - scores[winner]) >= kCardMatchMinMargin;
			}
			board_card_probe_attempts.Add(attempt);

			if(attempt.scored && attempt.accepted) {
				VsmBoardCardObservation bco;
				bco.frame      = fid;
				bco.card_index = c->card_index;
				bco.value      = (attempt.winner == kCardHolderVocabIndex) ? -1 : attempt.winner;
				board_card_observations.Add(bco);
			}
		}

		// M05-09 (task 0127): change-triggered direct probe of each
		// action_icon candidate rect - same rationale as the board_card probe
		// immediately above (checked first, per this task's own guidance:
		// role=="action_icon" observations from the normal VsmMatchRegion
		// path are rare/nonexistent in real fixtures too - see evidence
		// section for the real observation count found). Unlike the board
		// probe, no geometry correction is needed here (§2/this task's
		// evidence section confirmed the action_icon candidate rect already
		// matches the real on-screen rect exactly), so this probes the
		// theoretical candidate rect directly, same as the board probe does.
		for(const VsmLayoutCandidate* c : action_icon_candidates) {
			bool close_enough = false;
			for(const VsmChangedRect& cr : changes) {
				if(!(c->rect & cr.GetRect()).IsEmpty()) {
					close_enough = true;
					break;
				}
			}
			if(!close_enough)
				continue;

			if(!curr_frame_img_ready) {
				curr_frame_img = VsmFrameImageToImage(curr_frame);
				curr_frame_img_ready = true;
			}

			VsmActionIconProbeAttempt attempt;
			attempt.frame = fid;
			attempt.seat  = c->seat_index;
			double scores[kActionIconRealVocabCount + 1];
			int winner = -2;
			if(VsmScoreActionIcon(curr_frame_img, c->rect, c->seat_index, scores, winner)) {
				int winner_index = -1;
				for(int i = 0; i <= kActionIconRealVocabCount; i++) {
					int val = (i == kActionIconRealVocabCount) ? kActionIconEmptyWinnerValue : kActionIconRealVocab[i];
					if(val == winner)
						winner_index = i;
				}
				double runnerup = DBL_MAX;
				for(int i = 0; i <= kActionIconRealVocabCount; i++)
					if(i != winner_index && scores[i] < runnerup)
						runnerup = scores[i];
				attempt.scored         = true;
				attempt.winner         = winner;
				attempt.score_best     = scores[winner_index];
				attempt.score_runnerup = runnerup;
				attempt.accepted       = (scores[winner_index] <= kActionIconMatchMaxScore)
				                          && (runnerup - scores[winner_index] >= kActionIconMatchMinMargin);
			}
			action_icon_probe_attempts.Add(attempt);

			if(attempt.scored && attempt.accepted) {
				VsmActionIconObservation aio;
				aio.frame = fid;
				aio.seat  = c->seat_index;
				aio.value = attempt.winner;
				action_icon_observations.Add(aio);
			}
		}

		// M05-10 (task 0128): change-triggered direct probe of each
		// hole_card_0/hole_card_1 candidate rect - same rationale/trigger
		// condition as the board_card and action_icon probes above (real
		// recordings show tiny per-seat sub-slot roles essentially never win
		// a normal VsmMatchRegion observation on their own). This is also
		// the mechanism that probes a FOLDED seat's hole_card_0/1 rects even
		// though those sub-slots are Hidden() at that point in a real
		// session (see this task's evidence section, "the fold/combined
		// case" - GameTable::RenderToImage's DrawChild lambda draws
		// pixmapLabel_carda/cardb unconditionally, ignoring Show()/Hide(),
		// so a fold's real pixel content there is a genuine, probeable
		// blend of stale carda/cardb content and the new interlaced
		// pixmapLabel_cards image drawn on top of it - a real, deliberately
		// NOT-recognized case this task's acceptance gate must correctly
		// reject, verified with real evidence below).
		for(const VsmLayoutCandidate* c : hole_card_candidates) {
			bool close_enough = false;
			for(const VsmChangedRect& cr : changes) {
				if(!(c->rect & cr.GetRect()).IsEmpty()) {
					close_enough = true;
					break;
				}
			}
			if(!close_enough)
				continue;

			if(!curr_frame_img_ready) {
				curr_frame_img = VsmFrameImageToImage(curr_frame);
				curr_frame_img_ready = true;
			}

			VsmHoleCardProbeAttempt attempt;
			attempt.frame = fid;
			attempt.seat = c->seat_index;
			attempt.card_index = c->card_index;
			double scores[kHoleCardVocabSize];
			int winner = -1;
			if(VsmScoreHoleCardSlot(curr_frame_img, c->rect, c->seat_index, scores, winner)) {
				double runnerup = DBL_MAX;
				for(int i = 0; i < kHoleCardVocabSize; i++)
					if(i != winner && scores[i] < runnerup)
						runnerup = scores[i];
				attempt.scored         = true;
				attempt.winner         = winner;
				attempt.score_best     = scores[winner];
				attempt.score_runnerup = runnerup;
				attempt.accepted       = (runnerup - scores[winner]) >= kHoleCardMatchMinMargin;
			}
			hole_card_probe_attempts.Add(attempt);

			if(attempt.scored && attempt.accepted) {
				VsmHoleCardObservation hco;
				hco.frame      = fid;
				hco.seat       = c->seat_index;
				hco.card_index = c->card_index;
				hco.value      = (attempt.winner == kHoleCardBackVocabIndex) ? -1 : attempt.winner;
				hole_card_observations.Add(hco);
			}
		}

		if(prev_frame.width != curr_frame.width || prev_frame.height != curr_frame.height)
			prev_frame.Set(curr_frame.width, curr_frame.height, nullptr);
		memcpy(prev_frame.data, curr_frame.data, (size_t)curr_frame.width * curr_frame.height * 4);
	}

	int dealer_button_obs = 0;
	for(const VsmLayoutObservationOut& o : observations)
		if(o.role == "dealer_button")
			dealer_button_obs++;
	Cout() << "Total region observations: " << observations.GetCount()
	       << " (of which dealer_button-role: " << dealer_button_obs << ")\n\n";

	// --- M05-03 (task 0121): template-match disambiguation. For every
	// dealer_button-role observation, only keep it as a genuine dealer-seat
	// move if the DEALER reference (role 0) scored best among the 3 puck
	// references - observations where SB or BB scored best are the SAME
	// sub-slot firing for the wrong seat in a multi-seat rotation, and are
	// discarded here rather than fed into ApplyDealerButtonObservations's
	// "sticky" per-frame accumulation (that function's own logic is
	// otherwise unchanged from task 0119 - only its input is filtered now).
	if(dealer_button_obs > 0) {
		Cout() << "=== Template-match disambiguation (dealer_button-role observations) ===\n";
		Cout() << Format("%-6s %-10s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "dealer", "sb", "bb", "winner", "kept?");
	}
	Vector<VsmLayoutObservationOut> dealer_move_observations;
	int puck_discarded = 0;
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "dealer_button") {
			dealer_move_observations.Add(o);
			continue;
		}
		bool keep = o.puck_scored && o.puck_role_winner == 0;
		if(keep)
			dealer_move_observations.Add(o);
		else
			puck_discarded++;
		static const char* role_names[3] = { "dealer", "sb", "bb" };
		Cout() << Format("%-6d %-10d %-9s %-9s %-9s %-8s %-10s\n",
			o.frame, o.seat_index,
			o.puck_scored ? DblStr(o.puck_score_dealer) : String("n/a"),
			o.puck_scored ? DblStr(o.puck_score_sb)     : String("n/a"),
			o.puck_scored ? DblStr(o.puck_score_bb)     : String("n/a"),
			o.puck_scored ? String(role_names[o.puck_role_winner]) : String("n/a"),
			keep ? "kept" : "discarded");
	}
	if(dealer_button_obs > 0)
		Cout() << "dealer_button-role observations: " << dealer_button_obs
		       << " kept=" << (dealer_button_obs - puck_discarded)
		       << " discarded=" << puck_discarded << "\n\n";

	// M05-04 (task 0122): print the recovery-pass probe table (mirrors
	// 0121's disambiguation table above - real scores, not just the
	// verdict), then merge every recovered observation into
	// dealer_move_observations (additively - nothing produced by the normal
	// pipeline above is removed or altered by this).
	if(!recovery_attempts.IsEmpty()) {
		Cout() << "=== Scale/position-tolerant puck template rescue (task 0122) ===\n";
		Cout() << Format("%-6s %-6s %-7s %-5s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "scale", "dx", "dy", "dealer", "sb", "bb", "winner", "recovered?");
		static const char* role_names[3] = { "dealer", "sb", "bb" };
		for(const VsmPuckRecoveryAttempt& a : recovery_attempts) {
			Cout() << Format("%-6d %-6d %-7s %-5d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				a.frame, a.seat,
				a.found ? DblStr(a.scale) : String("n/a"),
				a.dx, a.dy,
				a.found ? DblStr(a.score_dealer) : String("n/a"),
				a.found ? DblStr(a.score_sb)     : String("n/a"),
				a.found ? DblStr(a.score_bb)     : String("n/a"),
				a.found ? String(role_names[a.winner]) : String("n/a"),
				a.recovered ? "recovered" : "no");
		}
		Cout() << "Recovery-pass triggers: " << recovery_attempts.GetCount()
		       << " recovered=" << recovered_observations.GetCount() << "\n\n";
	}
	for(const VsmLayoutObservationOut& o : recovered_observations)
		dealer_move_observations.Add(o);

	// M05-08 (task 0126): board-card confidence-margin acceptance gate. For
	// every board_card-role observation produced by the main per-transition
	// loop above, only accept it as a genuine per-slot recognition if its
	// winner/runner-up margin clears kCardMatchMinMargin - see
	// VsmScoreCardSlot's doc comment for why a MARGIN (not an absolute score
	// cap like 0122's kPuckRecoveryMatchThreshold) is the right gate shape
	// here. Accepted observations are appended to `board_card_observations`,
	// which already holds the frame-0 initial-seed observations added before
	// the transition loop started.
	int board_card_obs = 0;
	for(const VsmLayoutObservationOut& o : observations)
		if(o.role == "board_card")
			board_card_obs++;
	if(board_card_obs > 0) {
		Cout() << "=== Board-card template match (board_card-role observations) ===\n";
		Cout() << Format("%-6s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "slot", "best", "runnerup", "margin", "winner", "accepted?");
	}
	int card_discarded = 0;
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "board_card")
			continue;
		double margin = o.card_scored ? (o.card_score_runnerup - o.card_score_best) : -1.0;
		bool accept = o.card_scored && margin >= kCardMatchMinMargin;
		if(accept) {
			VsmBoardCardObservation bco;
			bco.frame      = o.frame;
			bco.card_index = o.card_index;
			bco.value      = (o.card_winner == kCardHolderVocabIndex) ? -1 : o.card_winner;
			board_card_observations.Add(bco);
		}
		else
			card_discarded++;
		String winner_str = !o.card_scored ? String("n/a")
			: (o.card_winner == kCardHolderVocabIndex ? String("holder") : IntStr(o.card_winner));
		Cout() << Format("%-6d %-5d %-9s %-9s %-9s %-8s %-10s\n",
			o.frame, o.card_index,
			o.card_scored ? DblStr(o.card_score_best)     : String("n/a"),
			o.card_scored ? DblStr(o.card_score_runnerup) : String("n/a"),
			o.card_scored ? DblStr(margin)                : String("n/a"),
			winner_str,
			accept ? "accepted" : "discarded");
	}
	if(board_card_obs > 0)
		Cout() << "board_card-role observations: " << board_card_obs
		       << " accepted=" << (board_card_obs - card_discarded)
		       << " discarded=" << card_discarded << "\n\n";

	// M05-08 (task 0126): print the change-triggered direct-probe table -
	// this is this task's ACTUAL working detection path in real recordings
	// (see the probe loop's own comment above, in the per-transition loop,
	// for why the role=="board_card" path above rarely/never fires) - real
	// scores, not just the final accept/reject verdict, mirroring 0122's
	// recovery-pass table.
	if(!board_card_probe_attempts.IsEmpty()) {
		Cout() << "=== Board-card change-triggered direct probe (task 0126) ===\n";
		Cout() << Format("%-6s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "slot", "best", "runnerup", "margin", "winner", "accepted?");
		int probe_accepted = 0;
		for(const VsmBoardCardProbeAttempt& a : board_card_probe_attempts) {
			double margin = a.scored ? (a.score_runnerup - a.score_best) : -1.0;
			String winner_str = !a.scored ? String("n/a")
				: (a.winner == kCardHolderVocabIndex ? String("holder") : IntStr(a.winner));
			Cout() << Format("%-6d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				a.frame, a.card_index,
				a.scored ? DblStr(a.score_best)     : String("n/a"),
				a.scored ? DblStr(a.score_runnerup) : String("n/a"),
				a.scored ? DblStr(margin)            : String("n/a"),
				winner_str,
				a.accepted ? "accepted" : "discarded");
			if(a.accepted)
				probe_accepted++;
		}
		Cout() << "Direct-probe triggers: " << board_card_probe_attempts.GetCount()
		       << " accepted=" << probe_accepted << "\n\n";
	}

	// M05-09 (task 0127): action-icon confidence-margin + absolute-cap
	// acceptance gate, mirroring the board-card gate immediately above (same
	// double loop shape: first the normal-VsmMatchRegion-path observations,
	// then the change-triggered direct-probe table) - see VsmScoreActionIcon's
	// doc comment for the gate's own derivation/limits.
	auto ActionIconWinnerStr = [](int w) -> String {
		if(w == kActionIconEmptyWinnerValue) return "empty";
		if(w == kActionIconVocabWinner) return "winner";
		return IntStr(w);
	};
	int action_icon_obs = 0;
	for(const VsmLayoutObservationOut& o : observations)
		if(o.role == "action_icon")
			action_icon_obs++;
	if(action_icon_obs > 0) {
		Cout() << "=== Action-icon template match (action_icon-role observations) ===\n";
		Cout() << Format("%-6s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "best", "runnerup", "margin", "winner", "accepted?");
	}
	int action_icon_discarded = 0;
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "action_icon")
			continue;
		double margin = o.action_icon_scored ? (o.action_icon_score_runnerup - o.action_icon_score_best) : -1.0;
		bool accept = o.action_icon_scored && (o.action_icon_score_best <= kActionIconMatchMaxScore)
		              && (margin >= kActionIconMatchMinMargin);
		if(accept) {
			VsmActionIconObservation aio;
			aio.frame = o.frame;
			aio.seat  = o.seat_index;
			aio.value = o.action_icon_winner;
			action_icon_observations.Add(aio);
		}
		else
			action_icon_discarded++;
		Cout() << Format("%-6d %-5d %-9s %-9s %-9s %-8s %-10s\n",
			o.frame, o.seat_index,
			o.action_icon_scored ? DblStr(o.action_icon_score_best)     : String("n/a"),
			o.action_icon_scored ? DblStr(o.action_icon_score_runnerup) : String("n/a"),
			o.action_icon_scored ? DblStr(margin)                       : String("n/a"),
			o.action_icon_scored ? ActionIconWinnerStr(o.action_icon_winner) : String("n/a"),
			accept ? "accepted" : "discarded");
	}
	if(action_icon_obs > 0)
		Cout() << "action_icon-role observations: " << action_icon_obs
		       << " accepted=" << (action_icon_obs - action_icon_discarded)
		       << " discarded=" << action_icon_discarded << "\n\n";

	if(!action_icon_probe_attempts.IsEmpty()) {
		Cout() << "=== Action-icon change-triggered direct probe (task 0127) ===\n";
		Cout() << Format("%-6s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "best", "runnerup", "margin", "winner", "accepted?");
		int probe_accepted = 0;
		for(const VsmActionIconProbeAttempt& a : action_icon_probe_attempts) {
			double margin = a.scored ? (a.score_runnerup - a.score_best) : -1.0;
			Cout() << Format("%-6d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				a.frame, a.seat,
				a.scored ? DblStr(a.score_best)     : String("n/a"),
				a.scored ? DblStr(a.score_runnerup) : String("n/a"),
				a.scored ? DblStr(margin)            : String("n/a"),
				a.scored ? ActionIconWinnerStr(a.winner) : String("n/a"),
				a.accepted ? "accepted" : "discarded");
			if(a.accepted)
				probe_accepted++;
		}
		Cout() << "Direct-probe triggers: " << action_icon_probe_attempts.GetCount()
		       << " accepted=" << probe_accepted << "\n\n";
	}

	// M05-10 (task 0128): hole-card confidence-margin acceptance gate, same
	// shape as the board-card gate above - accepted observations are
	// appended to `hole_card_observations`, which already holds the frame-0
	// initial-seed observations added before the transition loop started.
	int hole_card_obs = 0;
	for(const VsmLayoutObservationOut& o : observations)
		if(o.role == "hole_card")
			hole_card_obs++;
	if(hole_card_obs > 0) {
		Cout() << "=== Hole-card template match (hole_card-role observations) ===\n";
		Cout() << Format("%-6s %-5s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "slot", "best", "runnerup", "margin", "winner", "accepted?");
	}
	int hole_card_discarded = 0;
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "hole_card")
			continue;
		double margin = o.hole_card_scored ? (o.hole_card_score_runnerup - o.hole_card_score_best) : -1.0;
		bool accept = o.hole_card_scored && margin >= kHoleCardMatchMinMargin;
		if(accept) {
			VsmHoleCardObservation hco;
			hco.frame      = o.frame;
			hco.seat       = o.seat_index;
			hco.card_index = o.card_index;
			hco.value      = (o.hole_card_winner == kHoleCardBackVocabIndex) ? -1 : o.hole_card_winner;
			hole_card_observations.Add(hco);
		}
		else
			hole_card_discarded++;
		String winner_str = !o.hole_card_scored ? String("n/a")
			: (o.hole_card_winner == kHoleCardBackVocabIndex ? String("back") : IntStr(o.hole_card_winner));
		Cout() << Format("%-6d %-5d %-5d %-9s %-9s %-9s %-8s %-10s\n",
			o.frame, o.seat_index, o.card_index,
			o.hole_card_scored ? DblStr(o.hole_card_score_best)     : String("n/a"),
			o.hole_card_scored ? DblStr(o.hole_card_score_runnerup) : String("n/a"),
			o.hole_card_scored ? DblStr(margin)                     : String("n/a"),
			winner_str,
			accept ? "accepted" : "discarded");
	}
	if(hole_card_obs > 0)
		Cout() << "hole_card-role observations: " << hole_card_obs
		       << " accepted=" << (hole_card_obs - hole_card_discarded)
		       << " discarded=" << hole_card_discarded << "\n\n";

	if(!hole_card_probe_attempts.IsEmpty()) {
		Cout() << "=== Hole-card change-triggered direct probe (task 0128) ===\n";
		Cout() << Format("%-6s %-5s %-5s %-9s %-9s %-9s %-8s %-10s\n",
			"frame", "seat", "slot", "best", "runnerup", "margin", "winner", "accepted?");
		int probe_accepted = 0;
		for(const VsmHoleCardProbeAttempt& a : hole_card_probe_attempts) {
			double margin = a.scored ? (a.score_runnerup - a.score_best) : -1.0;
			String winner_str = !a.scored ? String("n/a")
				: (a.winner == kHoleCardBackVocabIndex ? String("back") : IntStr(a.winner));
			Cout() << Format("%-6d %-5d %-5d %-9s %-9s %-9s %-8s %-10s\n",
				a.frame, a.seat, a.card_index,
				a.scored ? DblStr(a.score_best)     : String("n/a"),
				a.scored ? DblStr(a.score_runnerup) : String("n/a"),
				a.scored ? DblStr(margin)            : String("n/a"),
				winner_str,
				a.accepted ? "accepted" : "discarded");
			if(a.accepted)
				probe_accepted++;
		}
		Cout() << "Direct-probe triggers: " << hole_card_probe_attempts.GetCount()
		       << " accepted=" << probe_accepted << "\n\n";
	}

	// --- Derive per-frame board-card slot values (frame-0 seed + every
	// accepted per-transition recognition above), 5 independent sticky
	// tracks. ---
	Vector<bool> board_known[5];
	Vector<int>  board_value[5];
	DeriveBoardCardsPerFrame(board_card_observations, 0, info.frame_count - 1, board_known, board_value);

	// --- Derive per-frame per-seat action-icon values (see
	// DeriveActionIconsPerFrame's own doc comment for the sticky/reset
	// design). Seat list: every seat that has an action_icon candidate at
	// all (built once above, `action_icon_candidates`). ---
	Vector<int> action_icon_seats;
	{
		VectorMap<int, bool> seen;
		for(const VsmLayoutCandidate* c : action_icon_candidates)
			if(seen.Find(c->seat_index) < 0) {
				seen.Add(c->seat_index, true);
				action_icon_seats.Add(c->seat_index);
			}
	}
	VectorMap<int, Vector<bool>> action_icon_known;
	VectorMap<int, Vector<int>>  action_icon_value;
	DeriveActionIconsPerFrame(action_icon_observations, 0, info.frame_count - 1, action_icon_seats,
	                            action_icon_known, action_icon_value);

	// --- Derive per-frame per-(seat,slot) hole-card values (see
	// DeriveHoleCardsPerFrame's own doc comment). Same seat list as action
	// icons, reused directly rather than re-scanning hole_card_candidates
	// for an equivalent list (every seat with any action_icon candidate also
	// has a hole_card candidate, both are per-active-seat sub-slots). ---
	VectorMap<int, Vector<bool>> hole_card_known[2];
	VectorMap<int, Vector<int>>  hole_card_value[2];
	DeriveHoleCardsPerFrame(hole_card_observations, 0, info.frame_count - 1, action_icon_seats,
	                          hole_card_known, hole_card_value);

	// --- Derive per-frame dealer seat from dealer_button observations
	// (input now pre-filtered to only genuine dealer-seat moves, above, PLUS
	// any task-0122 recovered observations merged in immediately above). ---
	VectorMap<int, int> dealer_seat_by_frame;
	ApplyDealerButtonObservations(dealer_move_observations, dealer_seat_by_frame);

	Vector<bool> derived_known;
	Vector<int>  derived_seat;
	DeriveDealerSeatPerFrame(dealer_seat_by_frame, 0, info.frame_count - 1, derived_known, derived_seat);

	// --- Build comparator records + count match/mismatch/unknown. ---
	Vector<VsmLogicCompareRecordOut> out_records;
	int matches = 0, mismatches = 0, unknowns = 0;

	Cout() << "=== Frame-by-frame dealer seat: derived vs. ground truth ===\n";
	Cout() << Format("%-6s %-16s %-18s %-10s\n", "frame", "derived_dealer", "gt_dealer", "verdict");

	for(int fid = 0; fid < gt_records.GetCount(); fid++) {
		const TexasHoldemGroundTruthRecord& gt = gt_records[fid];

		int gt_dealer_seat = -1;
		bool gt_dealer_known = false;
		for(const TexasHoldemPlayerSnapshot& p : gt.players) {
			if(p.button == 1) {
				gt_dealer_seat = p.seat;
				gt_dealer_known = true;
				break;
			}
		}

		VsmLogicCompareRecordOut rec;
		rec.frame_id = fid;
		rec.derived_dealer_seat_known = derived_known[fid];
		rec.derived_dealer_seat       = derived_seat[fid];
		rec.ground_truth_dealer_seat_known = gt_dealer_known;
		rec.ground_truth_dealer_seat       = gt_dealer_seat;

		if(!rec.derived_dealer_seat_known) {
			rec.verdict = "unknown";
			unknowns++;
		}
		else if(gt_dealer_known && rec.derived_dealer_seat == gt_dealer_seat) {
			rec.verdict = "match";
			matches++;
		}
		else {
			rec.verdict = "mismatch";
			mismatches++;
		}

		// Full logic-state struct (point 1 of this task's objective) - every
		// field left at its not-yet-parsed default EXCEPT frame_id (identity)
		// and dealer_seat (the one real vision-derived fact). `players` gets
		// one entry per ground-truth seat purely for seat IDENTITY (seat
		// index) so a later task can fill in per-player fields without
		// restructuring the vector - no ground-truth VALUE is copied into
		// any player field except the derived dealer's own button flag.
		TexasHoldemLogicState& ls = rec.logic_state;
		ls.frame_id = fid;
		ls.dealer_seat_known = rec.derived_dealer_seat_known;
		ls.dealer_seat       = rec.derived_dealer_seat;
		for(const TexasHoldemPlayerSnapshot& p : gt.players) {
			TexasHoldemLogicPlayerState ps;
			ps.seat = p.seat;
			if(rec.derived_dealer_seat_known && p.seat == rec.derived_dealer_seat) {
				ps.button_known = true;
				ps.button = 1; // GBUTTON_DEALER
				ls.players_known = true;
			}
			ls.players.Add(pick(ps));
		}

		Cout() << Format("%-6d %-16s %-18s %-10s\n",
			fid,
			rec.derived_dealer_seat_known ? IntStr(rec.derived_dealer_seat) : String("unknown"),
			rec.ground_truth_dealer_seat_known ? IntStr(rec.ground_truth_dealer_seat) : String("unknown"),
			rec.verdict);

		out_records.Add(pick(rec));
	}

	Cout() << "\n=== Comparison Summary ===\n";
	Cout() << "Total frames: " << out_records.GetCount() << "\n";
	Cout() << "Match: " << matches << "\n";
	Cout() << "Mismatch: " << mismatches << "\n";
	Cout() << "Unknown (dealer seat not yet observed from vision): " << unknowns << "\n";

	// --- M05-08 (task 0126): board-card frame-by-frame comparison. A
	// second pass over the SAME out_records (mutated in place - board-card
	// fields are additive, not a restructuring of the dealer-seat loop
	// above), using the per-slot sticky state derived above. See
	// VsmLogicCompareRecordOut::board_cards_verdict's doc comment for the
	// exact per-frame verdict semantics and why a naive whole-vector
	// equality check against ground truth would be wrong here.
	auto FormatBoardVec = [](const Vector<int>& v) {
		String s = "[";
		for(int i = 0; i < v.GetCount(); i++) {
			if(i) s << ",";
			s << v[i];
		}
		s << "]";
		return s;
	};

	Cout() << "\n=== Frame-by-frame board cards: derived vs. ground truth ===\n";
	Cout() << Format("%-6s %-20s %-20s %-10s\n", "frame", "derived_board", "gt_board", "verdict");

	int board_match_frames = 0, board_mismatch_frames = 0, board_pending_frames = 0, board_unknown_frames = 0;
	for(int fid = 0; fid < out_records.GetCount() && fid < gt_records.GetCount(); fid++) {
		VsmLogicCompareRecordOut& rec = out_records[fid];
		const TexasHoldemGroundTruthRecord& gt = gt_records[fid];

		rec.ground_truth_board_cards = clone(gt.board_cards);

		TexasHoldemLogicState& ls = rec.logic_state;
		ls.board_cards.Clear();
		bool all_known = true;
		for(int slot = 0; slot < 5; slot++) {
			bool k = fid < board_known[slot].GetCount() && board_known[slot][fid];
			int  v = (fid < board_value[slot].GetCount()) ? board_value[slot][fid] : -1;
			ls.board_cards.Add(k ? v : -1);
			if(!k)
				all_known = false;
		}
		ls.board_cards_known = all_known;

		rec.board_cards_match_slots = 0;
		rec.board_cards_mismatch_slots = 0;
		rec.board_cards_pending_slots = 0;
		if(!all_known) {
			rec.board_cards_verdict = "unknown";
			board_unknown_frames++;
		}
		else {
			int gt_count = gt.board_cards.GetCount();
			for(int slot = 0; slot < 5; slot++) {
				int derived_v = ls.board_cards[slot];
				if(derived_v < 0) {
					rec.board_cards_pending_slots++;
					continue;
				}
				int gt_v = (slot < gt_count) ? gt.board_cards[slot] : -1;
				if(derived_v == gt_v)
					rec.board_cards_match_slots++;
				else
					rec.board_cards_mismatch_slots++;
			}
			if(rec.board_cards_mismatch_slots > 0) {
				rec.board_cards_verdict = "mismatch";
				board_mismatch_frames++;
			}
			else if(rec.board_cards_match_slots > 0) {
				rec.board_cards_verdict = "match";
				board_match_frames++;
			}
			else {
				rec.board_cards_verdict = "pending";
				board_pending_frames++;
			}
		}

		Cout() << Format("%-6d %-20s %-20s %-10s\n",
			fid, FormatBoardVec(ls.board_cards), FormatBoardVec(rec.ground_truth_board_cards),
			rec.board_cards_verdict);
	}

	Cout() << "\n=== Board-card Comparison Summary ===\n";
	Cout() << "Total frames: " << out_records.GetCount() << "\n";
	Cout() << "Match (>=1 slot recognized as a real card, all recognized slots correct): "
	       << board_match_frames << "\n";
	Cout() << "Mismatch (>=1 slot recognized as a real card, disagrees with ground truth): "
	       << board_mismatch_frames << "\n";
	Cout() << "Pending (all 5 slots observed, all still \"not yet dealt\"): "
	       << board_pending_frames << "\n";
	Cout() << "Unknown (not every one of the 5 slots observed yet): "
	       << board_unknown_frames << "\n";

	// --- M05-09 (task 0127): action-icon frame-by-frame comparison. A third
	// pass over the SAME out_records (additive, mutated in place, same
	// pattern the board-card pass above already used), using the per-seat
	// sticky state derived above. Only seats whose sticky value is a REAL
	// action (1..6) are compared against ground truth's `action` field -
	// see the task's own ambiguity note (0/7/8 are all visually
	// indistinguishable) and VsmLogicCompareRecordOut's doc comment for why
	// "winner" (9) and "not yet observed" seats are excluded from
	// match/mismatch, not silently guessed.
	Cout() << "\n=== Frame-by-frame action icons: derived vs. ground truth (per seat) ===\n";
	Cout() << Format("%-6s %-5s %-10s %-10s %-10s\n", "frame", "seat", "derived", "gt_action", "verdict");

	int action_icon_match_frames = 0, action_icon_mismatch_frames = 0, action_icon_unscored_frames = 0;
	for(int fid = 0; fid < out_records.GetCount() && fid < gt_records.GetCount(); fid++) {
		VsmLogicCompareRecordOut& rec = out_records[fid];
		const TexasHoldemGroundTruthRecord& gt = gt_records[fid];
		TexasHoldemLogicState& ls = rec.logic_state;

		rec.action_icon_match_seats = 0;
		rec.action_icon_mismatch_seats = 0;
		rec.action_icon_winner_seats = 0;
		rec.action_icon_unscored_seats = 0;

		for(TexasHoldemLogicPlayerState& ps : ls.players) {
			int ki = action_icon_known.Find(ps.seat);
			bool k = (ki >= 0) && fid < action_icon_known[ki].GetCount() && action_icon_known[ki][fid];
			int vi = action_icon_value.Find(ps.seat);
			int v  = (k && vi >= 0 && fid < action_icon_value[vi].GetCount()) ? action_icon_value[vi][fid] : -1;

			if(!k)
				continue; // not yet observed since the last reset - leave action_known false (default)

			if(v == kActionIconVocabWinner) {
				ps.action_known = true;
				ps.action = v;
				ls.players_known = true;
				rec.action_icon_winner_seats++;
				continue; // excluded from ground-truth comparison, see doc comment above
			}

			ps.action_known = true;
			ps.action = v;
			ls.players_known = true;

			int gt_action = -1;
			for(const TexasHoldemPlayerSnapshot& p : gt.players)
				if(p.seat == ps.seat) { gt_action = p.action; break; }

			if(v == gt_action)
				rec.action_icon_match_seats++;
			else
				rec.action_icon_mismatch_seats++;

			Cout() << Format("%-6d %-5d %-10d %-10d %-10s\n",
				fid, ps.seat, v, gt_action, (v == gt_action) ? "match" : "mismatch");
		}

		rec.action_icon_unscored_seats = ls.players.GetCount()
			- rec.action_icon_match_seats - rec.action_icon_mismatch_seats - rec.action_icon_winner_seats;

		if(rec.action_icon_mismatch_seats > 0) {
			rec.action_icons_verdict = "mismatch";
			action_icon_mismatch_frames++;
		}
		else if(rec.action_icon_match_seats > 0) {
			rec.action_icons_verdict = "match";
			action_icon_match_frames++;
		}
		else {
			rec.action_icons_verdict = "unscored";
			action_icon_unscored_frames++;
		}
	}

	Cout() << "\n=== Action-icon Comparison Summary ===\n";
	Cout() << "Total frames: " << out_records.GetCount() << "\n";
	Cout() << "Match (frames with >=1 seat's real action (1..6) recognized, all recognized seats correct): "
	       << action_icon_match_frames << "\n";
	Cout() << "Mismatch (frames with >=1 recognized real action disagreeing with ground truth): "
	       << action_icon_mismatch_frames << "\n";
	Cout() << "Unscored (no seat had a confidently-recognized real action this frame): "
	       << action_icon_unscored_frames << "\n";
	{
		int total_match = 0, total_mismatch = 0, total_winner = 0, total_unscored = 0;
		for(const VsmLogicCompareRecordOut& o : out_records) {
			total_match    += o.action_icon_match_seats;
			total_mismatch += o.action_icon_mismatch_seats;
			total_winner   += o.action_icon_winner_seats;
			total_unscored += o.action_icon_unscored_seats;
		}
		Cout() << "Per-seat totals across all frames: match=" << total_match
		       << " mismatch=" << total_mismatch << " winner=" << total_winner
		       << " unscored=" << total_unscored << "\n";
	}

	// --- M05-10 (task 0128): hole-card frame-by-frame comparison. A fourth
	// pass over the SAME out_records (additive, mutated in place, same
	// pattern the board-card/action-icon passes above already used), using
	// the per-(seat,slot) sticky state derived above. Populates
	// `logic_state.players[i].hole_cards`/`hole_cards_known` (existing
	// fields, task 0119) for EVERY seat (both known and not-yet-known), then
	// compares only the slots vision recognized as a REAL card (excluding
	// -1/"back") against ground truth's OWN `hole_cards` at the same index -
	// see VsmLogicCompareRecordOut::hole_cards_match_seats' doc comment for
	// the exact verdict semantics.
	Cout() << "\n=== Frame-by-frame hole cards: derived vs. ground truth (per seat) ===\n";
	Cout() << Format("%-6s %-5s %-14s %-14s\n", "frame", "seat", "derived", "gt_hole_cards");

	int hole_cards_match_frames = 0, hole_cards_mismatch_frames = 0, hole_cards_unscored_frames = 0;
	for(int fid = 0; fid < out_records.GetCount() && fid < gt_records.GetCount(); fid++) {
		VsmLogicCompareRecordOut& rec = out_records[fid];
		const TexasHoldemGroundTruthRecord& gt = gt_records[fid];
		TexasHoldemLogicState& ls = rec.logic_state;

		rec.hole_cards_match_seats = 0;
		rec.hole_cards_mismatch_seats = 0;
		rec.hole_cards_hidden_seats = 0;
		rec.hole_cards_unknown_seats = 0;

		for(TexasHoldemLogicPlayerState& ps : ls.players) {
			int ki0 = hole_card_known[0].Find(ps.seat);
			int ki1 = hole_card_known[1].Find(ps.seat);
			bool k0 = (ki0 >= 0) && fid < hole_card_known[0][ki0].GetCount() && hole_card_known[0][ki0][fid];
			bool k1 = (ki1 >= 0) && fid < hole_card_known[1][ki1].GetCount() && hole_card_known[1][ki1][fid];
			int vi0 = hole_card_value[0].Find(ps.seat);
			int vi1 = hole_card_value[1].Find(ps.seat);
			int v0 = (k0 && vi0 >= 0 && fid < hole_card_value[0][vi0].GetCount()) ? hole_card_value[0][vi0][fid] : -1;
			int v1 = (k1 && vi1 >= 0 && fid < hole_card_value[1][vi1].GetCount()) ? hole_card_value[1][vi1][fid] : -1;

			ps.hole_cards.Clear();
			ps.hole_cards.Add(v0);
			ps.hole_cards.Add(v1);
			ps.hole_cards_known = k0 && k1;

			if(!ps.hole_cards_known) {
				rec.hole_cards_unknown_seats++;
				continue;
			}
			ls.players_known = true;

			const TexasHoldemPlayerSnapshot* gtp = NULL;
			for(const TexasHoldemPlayerSnapshot& p : gt.players)
				if(p.seat == ps.seat) { gtp = &p; break; }

			int seat_match = 0, seat_mismatch = 0;
			for(int slot = 0; slot < 2; slot++) {
				int derived_v = ps.hole_cards[slot];
				if(derived_v < 0)
					continue; // back/hidden - nothing to compare, mirrors board_cards' -1 exclusion
				int gt_v = (gtp && slot < gtp->hole_cards.GetCount()) ? gtp->hole_cards[slot] : -1;
				if(derived_v == gt_v)
					seat_match++;
				else
					seat_mismatch++;
			}
			if(seat_mismatch > 0)
				rec.hole_cards_mismatch_seats++;
			else if(seat_match > 0)
				rec.hole_cards_match_seats++;
			else
				rec.hole_cards_hidden_seats++;

			Cout() << Format("%-6d %-5d %-14s %-14s\n",
				fid, ps.seat, FormatBoardVec(ps.hole_cards),
				gtp ? FormatBoardVec(gtp->hole_cards) : String("n/a"));
		}

		if(rec.hole_cards_mismatch_seats > 0) {
			rec.hole_cards_verdict = "mismatch";
			hole_cards_mismatch_frames++;
		}
		else if(rec.hole_cards_match_seats > 0) {
			rec.hole_cards_verdict = "match";
			hole_cards_match_frames++;
		}
		else {
			rec.hole_cards_verdict = "unscored";
			hole_cards_unscored_frames++;
		}
	}

	Cout() << "\n=== Hole-card Comparison Summary ===\n";
	Cout() << "Total frames: " << out_records.GetCount() << "\n";
	Cout() << "Match (frames with >=1 seat's real hole card(s) recognized, all recognized seats correct): "
	       << hole_cards_match_frames << "\n";
	Cout() << "Mismatch (frames with >=1 recognized real hole card disagreeing with ground truth): "
	       << hole_cards_mismatch_frames << "\n";
	Cout() << "Unscored (no seat had a confidently-recognized real hole card this frame - covers "
	           "hidden/back-only and not-yet-observed seats alike): "
	       << hole_cards_unscored_frames << "\n";
	{
		int total_match = 0, total_mismatch = 0, total_hidden = 0, total_unknown = 0;
		for(const VsmLogicCompareRecordOut& o : out_records) {
			total_match   += o.hole_cards_match_seats;
			total_mismatch += o.hole_cards_mismatch_seats;
			total_hidden  += o.hole_cards_hidden_seats;
			total_unknown += o.hole_cards_unknown_seats;
		}
		Cout() << "Per-seat totals across all frames: match=" << total_match
		       << " mismatch=" << total_mismatch << " hidden(back)=" << total_hidden
		       << " unknown=" << total_unknown << "\n";
	}

	if(!jsonl_out.IsEmpty()) {
		String jsonl;
		for(const VsmLogicCompareRecordOut& o : out_records)
			jsonl << StoreAsJson(o) << "\n";
		if(!SaveFile(jsonl_out, jsonl)) {
			Fail(Format("Failed to write --jsonl-out file: %s", jsonl_out));
			return;
		}
		Cout() << "\nWrote " << out_records.GetCount() << " comparator record(s) to " << jsonl_out << "\n";
	}

	SetExitCode(0);
}
