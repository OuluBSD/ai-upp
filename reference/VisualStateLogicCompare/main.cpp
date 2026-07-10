#include <CtrlLib/CtrlLib.h>
#include <TexasHoldem/TexasHoldemSessionContract.h>
#include <TexasHoldem/TexasHoldemLogicState.h>
#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

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

// ---------------------------------------------------------------------------
// One region-to-layout-element observation, same shape as
// reference/VisualStateLayoutAssign/main.cpp's VsmLayoutObservationOut (this
// tool only actually consumes `frame`/`role`/`seat_index`, but keeps the full
// set of fields for parity/debuggability with that CLI's --jsonl-out output,
// e.g. when comparing --jsonl-out files by hand).
struct VsmLayoutObservationOut : Moveable<VsmLayoutObservationOut> {
	int    frame_prev = -1;
	int    frame      = -1;
	int    x = 0, y = 0, w = 0, h = 0;
	double score      = 0.0;
	String region_id;

	String assigned;
	String kind;
	String role;
	int    seat_index = -1;
	int    card_index = -1;
	double overlap    = 0.0;

	// M05-03 (task 0121): template-match disambiguation scores, filled in
	// only for role=="dealer_button" observations (see VsmScorePuckRoles).
	// puck_role_winner is the argmin index (0=dealer,1=SB,2=BB); the
	// observation is only treated as a genuine dealer-seat move when this is
	// 0 (see the filtering loop in GUI_APP_MAIN below).
	bool   puck_scored = false;
	double puck_score_dealer = -1, puck_score_sb = -1, puck_score_bb = -1;
	int    puck_role_winner = -1;

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

// The per-frame comparator record this tool emits.
struct VsmLogicCompareRecordOut : Moveable<VsmLogicCompareRecordOut> {
	int frame_id = -1;

	bool derived_dealer_seat_known = false;
	int  derived_dealer_seat = -1;

	bool ground_truth_dealer_seat_known = false;
	int  ground_truth_dealer_seat = -1;

	// "match", "mismatch", or "unknown" (derived_dealer_seat_known == false)
	String verdict;

	TexasHoldemLogicState logic_state;

	void Jsonize(JsonIO& json)
	{
		json
			("frame_id", frame_id)
			("derived_dealer_seat_known", derived_dealer_seat_known)
			("derived_dealer_seat", derived_dealer_seat)
			("ground_truth_dealer_seat_known", ground_truth_dealer_seat_known)
			("ground_truth_dealer_seat", ground_truth_dealer_seat)
			("verdict", verdict)
			("logic_state", logic_state)
		;
	}
};

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

// ---------------------------------------------------------------------------
// M05-03 (task 0121): template-match disambiguation for dealer_button-role
// observations.
//
// Problem (see this file's header comment + task 0119/0120 for the full
// story): the "dealer_button" sub-slot role fires for the dealer's puck AND
// for the small-blind/big-blind pucks (all three sit on the same per-seat
// sub-slot kind), so a full dealer rotation (all three pucks move in one
// frame transition) produces 3 same-role observations in that transition,
// and the old "last one wins" tie-break has no way to know which of the 3 is
// actually the dealer's.
//
// Fix: task 0120 gave the three pucks fixed, visually distinct procedural
// graphics (dealer=cream/"D", SB=light-blue/"SB", BB=dark-red/"BB") for
// exactly this purpose. TexasHoldemGetPuckReferenceImage (game/TexasHoldem/
// TexasHoldemLogicState.h, this task's objective 1) returns "whatever image
// currently represents puck role N under theme T" - the same themed-file-
// load-first + procedural-fallback resolution GameTable itself uses. Since
// all three images are small, deterministic, non-photographic (procedurally
// drawn discs+labels, or later a themed PNG - never noisy camera input),
// plain mean absolute per-pixel RGB difference against each of the 3 is
// sufficient to tell them apart - there is no need for a general template-
// matching/NCC/SIFT pipeline for a fixed set of 3 known small images, and
// building one would be over-engineering for this narrow, bounded problem
// (see this task's guardrails).
//
// Theme is hardcoded to "default" (kPuckReferenceTheme below): GameTable.cpp
// constructs with `LoadTheme("default")` and no CLI flag exists yet to record
// a session under a different theme, but the constant is named/isolated so a
// future theme-aware recording setup only has to change this one spot.
static const char* kPuckReferenceTheme = "default";

// Mean absolute per-pixel RGB difference between two same-size images.
// Returns DBL_MAX if the sizes mismatch or either image is empty (so it can
// never spuriously "win" a role comparison).
static double VsmMeanAbsPixelDiff(const Image& a, const Image& b)
{
	Size sz = a.GetSize();
	if(sz.cx <= 0 || sz.cy <= 0 || sz != b.GetSize())
		return DBL_MAX;
	int64 sum = 0;
	for(int y = 0; y < sz.cy; y++) {
		const RGBA* pa = a[y];
		const RGBA* pb = b[y];
		for(int x = 0; x < sz.cx; x++) {
			sum += abs((int)pa[x].r - (int)pb[x].r);
			sum += abs((int)pa[x].g - (int)pb[x].g);
			sum += abs((int)pa[x].b - (int)pb[x].b);
		}
	}
	return (double)sum / ((double)sz.cx * sz.cy * 3.0);
}

// Crops `candidate_rect` out of `frame_img`, scales the 3 puck reference
// images (dealer=0, SB=1, BB=2) to that rect's actual on-screen size, and
// returns the mean-abs-diff score for each role in `scores_out[0..2]`.
// Returns false (and leaves scores_out untouched) if the candidate rect is
// degenerate/out of bounds.
static bool VsmScorePuckRoles(const Image& frame_img, const Rect& candidate_rect,
                                double scores_out[3])
{
	int w = candidate_rect.GetWidth(), h = candidate_rect.GetHeight();
	if(w <= 0 || h <= 0)
		return false;
	Rect frame_rect(0, 0, frame_img.GetWidth(), frame_img.GetHeight());
	if(!frame_rect.Contains(candidate_rect))
		return false;

	Image candidate = Crop(frame_img, candidate_rect);
	for(int role = 0; role < 3; role++) {
		Image ref = TexasHoldemGetPuckReferenceImage(role, kPuckReferenceTheme);
		Image ref_scaled = (ref.GetSize() == Size(w, h)) ? ref : Rescale(ref, w, h);
		scores_out[role] = VsmMeanAbsPixelDiff(candidate, ref_scaled);
	}
	return true;
}

// ---------------------------------------------------------------------------
// Core derivation logic, factored out so --self-test can exercise the exact
// same code with synthetic input (no frames/fixtures needed) as the real run
// does with actually-detected observations. Applies every dealer_button-role
// observation (in the order given) to a "seat N is dealer from this frame
// onward" running VectorMap<frame, seat>, taking the LAST observation for a
// given `frame` if more than one seat's button_puck sub-slot changed in the
// same transition (see main.cpp's header comment - this can't be
// disambiguated further without OCR/template-matching on the puck image
// itself, which is explicitly out of scope for this task).
static void ApplyDealerButtonObservations(const Vector<VsmLayoutObservationOut>& observations,
                                            VectorMap<int, int>& dealer_seat_by_frame)
{
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "dealer_button")
			continue;
		dealer_seat_by_frame.GetAdd(o.frame, o.seat_index) = o.seat_index;
	}
}

// Given the per-transition dealer_button observations and the full inclusive
// frame range [frame_lo, frame_hi], returns one (frame_id -> known/seat)
// pair per frame, "sticky" from the frame a seat is first observed onward
// (frame 0 / frame_lo always starts unknown - there is no incoming
// transition to observe for the very first frame in range).
static void DeriveDealerSeatPerFrame(const VectorMap<int, int>& dealer_seat_by_frame,
                                       int frame_lo, int frame_hi,
                                       Vector<bool>& known_out, Vector<int>& seat_out)
{
	known_out.Clear();
	seat_out.Clear();
	bool known = false;
	int seat = -1;
	for(int fid = frame_lo; fid <= frame_hi; fid++) {
		int i = dealer_seat_by_frame.Find(fid);
		if(i >= 0) {
			known = true;
			seat = dealer_seat_by_frame[i];
		}
		known_out.Add(known);
		seat_out.Add(seat);
	}
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
			SetExitCode(ok ? 0 : 1);
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

			observations.Add(obs);
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

	// --- Derive per-frame dealer seat from dealer_button observations
	// (input now pre-filtered to only genuine dealer-seat moves, above). ---
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
