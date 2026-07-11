#include <CtrlLib/CtrlLib.h>
#include <TexasHoldem/TexasHoldemSessionContract.h>
#include <TexasHoldem/TexasHoldemLogicState.h>
#include <VisualStateModel/VisualStateModel.h>
#include <CardRender/CardRender.h>

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

	// M05-08 (task 0126): template-match scores, filled in only for
	// role=="board_card" observations (see VsmScoreCardSlot). card_winner is
	// the argmin index over the 53-way vocabulary: 0..51 == recognized card,
	// 52 == the holder ("not yet dealt") reference. card_score_best/
	// card_score_runnerup are the winning and second-best scores (used to
	// compute the confidence margin the acceptance gate checks).
	bool   card_scored = false;
	int    card_winner = -1;
	double card_score_best = -1, card_score_runnerup = -1;

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

	// M05-08 (task 0126): board-card comparison. `board_cards_verdict` is
	// frame-level, one of:
	//   "unknown"  - logic_state.board_cards_known is false (not every one
	//                of the 5 slots has fired a confident observation yet).
	//   "pending"  - all 5 slots known, but every one is still -1 ("not yet
	//                dealt") - nothing has been recognized as a real card
	//                yet this hand, so there is nothing to check against
	//                ground truth yet.
	//   "match"    - every slot recognized as a REAL card (0..51, i.e.
	//                excluding -1 "not yet dealt" slots) agrees with ground
	//                truth's board_cards at that same index.
	//   "mismatch" - at least one recognized-as-a-real-card slot disagrees.
	// See TexasHoldemLogicState::board_cards_known's usage below (in
	// GUI_APP_MAIN) for why a naive full-5-vector equality check against
	// ground truth is the WRONG comparison here: ground truth's board_cards
	// (game/TexasHoldem/Main.cpp:47) reflects the engine's own already-
	// shuffled deck state from hand start (street 0/preflop) onward, not
	// street-gated the way the RENDERED board is - so ground truth already
	// shows a slot's true eventual card well before it's ever visually
	// dealt, while vision correctly still sees "-1, not yet dealt" for that
	// same slot/frame. Comparing those two directly would flag every single
	// pre-reveal frame as a "mismatch", which reflects the two data
	// sources describing different things (visually-revealed vs. engine-
	// truth), not a recognition defect - see this task's evidence section
	// for the real fixture data this was confirmed against.
	Vector<int> ground_truth_board_cards;
	int    board_cards_match_slots = 0, board_cards_mismatch_slots = 0, board_cards_pending_slots = 0;
	String board_cards_verdict;

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
			("ground_truth_board_cards", ground_truth_board_cards)
			("board_cards_match_slots", board_cards_match_slots)
			("board_cards_mismatch_slots", board_cards_mismatch_slots)
			("board_cards_pending_slots", board_cards_pending_slots)
			("board_cards_verdict", board_cards_verdict)
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
// M05-04 (task 0122): scale/position-tolerant template rescue for
// button_puck observations that VsmDetectChanges's region-merge dilutes away
// entirely (see this task's Manager task file, root-caused there: a small
// per-seat button_puck pixel change gets folded into a much larger,
// spatially-adjacent changed rect - e.g. a board-reset region at a hand
// boundary - and VsmMatchTier's overlap formula, intersection-area /
// MERGED-REGION-area, never clears kOverlapThreshold for the puck's ~5%
// true contribution, so the seat's own dealer_button-role observation is
// never produced by the normal VsmDetectChanges -> VsmBuildCandidates ->
// VsmMatchRegion pipeline at all - not misassigned, simply absent).
//
// This is an ADDITIVE second pass over the exact same per-transition
// `changes` list that pipeline already produced (task guardrail: that
// pipeline, and 0121's VsmScorePuckRoles/VsmMeanAbsPixelDiff/
// ApplyDealerButtonObservations/DeriveDealerSeatPerFrame, are not modified -
// this only calls VsmScorePuckRoles with MORE candidate rects than the exact
// theoretical one, and only emits brand-new observations feeding into the
// same downstream path 0121 already wired up).
//
// Trigger condition (stays change-region-driven, never a per-frame scan):
// for each seat's theoretical button_puck candidate rect (from the same
// VsmBuildCandidates list), inflate it by kPuckRecoveryPadding and check
// whether it has ANY non-empty intersection with ANY of this transition's
// raw VsmDetectChanges rects, regardless of what VsmMatchRegion assigned
// that rect to. This is deliberately looser than VsmMatchTier's "%50 of the
// region's own area" rule (precisely the rule that fails for this dilution
// case - the puck's true contribution is ~5% of the merged region) while
// still requiring a REAL detected change nearby - a seat with no changed
// rect anywhere near it is skipped entirely, never probed.
//
//   kPuckRecoveryPadding = 24px. Measured button_puck candidate rects in
//   var/vsm_fixtures/texas_ps6p_puck_check (this session's frame-space,
//   sx=1/sy~0.969) range from Player1's ~32x32 to Player4's ~63x41 (see this
//   task's evidence section for the exact VsmBuildCandidates-derived
//   numbers) - 24px is roughly 40-80% of one such dimension: enough to
//   tolerate a real capture's "not pixel-perfect" alignment (premise 1 in
//   the task file) without approaching a whole-frame/whole-element scan.
//   The SAME padding also bounds the position-offset search below (one
//   padding concept, not two unrelated constants).
//
// Multi-scale template search: for each triggered (transition, seat) pair,
// scale the candidate rect by each of kPuckRecoveryScaleSteps (real-world
// zoom tolerance, premise 2) and try each of kPuckRecoveryOffsetFractions *
// kPuckRecoveryPadding as a center-offset in each axis (real-world position
// tolerance, and premise 3 - tolerating the detected region's own size/
// position noise too, since this doesn't anchor to the changed rect's own
// rect at all, only to the theoretical candidate). Every resulting rect is
// scored with the EXISTING VsmScorePuckRoles helper (0121's mean-absolute-
// per-pixel-RGB-diff metric against the 3 known puck references) -
// reasoning restated briefly per the task's guardrail: these are still
// small, low-noise, deterministic images (procedurally drawn discs/labels or
// a themed PNG), so plain pixel-difference stays the right level of
// complexity even when searched over more candidate rects; no NCC/SIFT/
// OpenCV dependency is introduced.
//
//   kPuckRecoveryScaleSteps = {0.8, 0.9, 1.0, 1.1, 1.2, 1.3} - exactly the
//   task's suggested "roughly 0.8x-1.3x" span, 6 fixed steps including the
//   nominal 1.0.
//   kPuckRecoveryOffsetFractions = {-1.0, -0.5, 0.0, 0.5, 1.0} (both axes,
//   5x5=25 combos per scale) - a small, bounded position search within the
//   same padded window the trigger check uses.
//
// The single best (lowest-score) (scale, offset, role) combination found
// across all of the above is kept. It's only turned into a recovered
// dealer_button-role observation if its winning role is DEALER (index 0)
// AND its score clears kPuckRecoveryMatchThreshold - see this task's
// evidence section for the real scores this threshold was picked against.
static const int    kPuckRecoveryPadding = 24;
static const double kPuckRecoveryScaleSteps[] = { 0.8, 0.9, 1.0, 1.1, 1.2, 1.3 };
static const int    kPuckRecoveryScaleStepCount =
	(int)(sizeof(kPuckRecoveryScaleSteps) / sizeof(kPuckRecoveryScaleSteps[0]));
static const double kPuckRecoveryOffsetFractions[] = { -1.0, -0.5, 0.0, 0.5, 1.0 };
static const int    kPuckRecoveryOffsetFractionCount =
	(int)(sizeof(kPuckRecoveryOffsetFractions) / sizeof(kPuckRecoveryOffsetFractions[0]));
// See this task's evidence section for the real fixture scores this was
// picked against: the two genuine dealer-winner cases found in
// texas_ps6p_puck_check score 26.2 (clean, no occlusion) and 49.5 (the
// target rotation, moderately affected by an adjacent seat's overlapping
// PlayerCtrl rect but still recoverable); every non-dealer-winner
// observation's OWN dealer score in that same fixture is >= 81 (i.e. far
// above this threshold, so raising it to comfortably clear 49.5 does not
// risk admitting any of those). 70.0 sits with real margin above the
// highest genuine case (49.5) and real margin below the lowest false case
// this fixture set exhibits when the argmin isn't already dealer.
static const double kPuckRecoveryMatchThreshold = 70.0;

// One (transition, seat) recovery probe, kept for the evidence table this
// tool prints (mirrors 0121's disambiguation table, same rationale: show the
// real scores, not just the final verdict).
struct VsmPuckRecoveryAttempt : Moveable<VsmPuckRecoveryAttempt> {
	int    frame = -1;
	int    seat  = -1;
	bool   found = false;   // at least one in-bounds (scale,offset) rect was scored
	double scale = 0.0;
	int    dx = 0, dy = 0;
	double score_dealer = -1, score_sb = -1, score_bb = -1;
	int    winner = -1;     // argmin(dealer,sb,bb) of each role's OWN best score
	bool   recovered = false;
};

// Runs the multi-scale/position search described above for one seat's
// candidate rect within one already-triggered transition (the caller is
// responsible for the trigger/proximity check - this function always
// searches, it doesn't itself decide whether to).
//
// IMPORTANT design point (found empirically while building this task's
// evidence - see the concrete example there): this searches EACH of the 3
// puck references' own best (lowest-score) position INDEPENDENTLY across
// the whole scale/offset grid, rather than taking one single global argmin
// across all (scale, offset, role) triples. A single global argmin lets an
// unrelated patch elsewhere in the search window "accidentally" out-score
// the true puck's own best-aligned match for the CORRECT role (e.g. a
// slightly-shifted crop that happens to look a bit more red/blue-ish can
// numerically beat the correctly-centered dealer match at ITS best
// position), which would silently discard a real, visually-confirmed match.
// Giving each of the 3 references a fair, independent search for its own
// best fit and comparing only those 3 best-of-search scores at the end
// avoids that.
static void VsmSearchPuckRecovery(const Image& frame_img, const Rect& candidate_rect,
                                    VsmPuckRecoveryAttempt& attempt)
{
	int base_w = candidate_rect.Width(), base_h = candidate_rect.Height();
	int ccx = candidate_rect.left + base_w / 2;
	int ccy = candidate_rect.top  + base_h / 2;

	double best_score[3]   = { DBL_MAX, DBL_MAX, DBL_MAX };
	double best_scale_r[3] = { 0.0, 0.0, 0.0 };
	int    best_dx_r[3]    = { 0, 0, 0 };
	int    best_dy_r[3]    = { 0, 0, 0 };
	bool found = false;

	for(int si = 0; si < kPuckRecoveryScaleStepCount; si++) {
		double scale = kPuckRecoveryScaleSteps[si];
		int w = (int)(base_w * scale + 0.5);
		int h = (int)(base_h * scale + 0.5);
		if(w <= 0 || h <= 0)
			continue;
		for(int yi = 0; yi < kPuckRecoveryOffsetFractionCount; yi++) {
			int dy = (int)(kPuckRecoveryOffsetFractions[yi] * kPuckRecoveryPadding
			               + (kPuckRecoveryOffsetFractions[yi] >= 0 ? 0.5 : -0.5));
			for(int xi = 0; xi < kPuckRecoveryOffsetFractionCount; xi++) {
				int dx = (int)(kPuckRecoveryOffsetFractions[xi] * kPuckRecoveryPadding
				               + (kPuckRecoveryOffsetFractions[xi] >= 0 ? 0.5 : -0.5));
				int left = ccx - w / 2 + dx, top = ccy - h / 2 + dy;
				Rect r(left, top, left + w, top + h);
				double scores[3];
				if(!VsmScorePuckRoles(frame_img, r, scores))
					continue;
				found = true;
				for(int role = 0; role < 3; role++) {
					if(scores[role] < best_score[role]) {
						best_score[role]  = scores[role];
						best_scale_r[role] = scale;
						best_dx_r[role] = dx; best_dy_r[role] = dy;
					}
				}
			}
		}
	}

	attempt.found = found;
	if(found) {
		attempt.score_dealer = best_score[0];
		attempt.score_sb     = best_score[1];
		attempt.score_bb     = best_score[2];
		int winner = 0;
		for(int r = 1; r < 3; r++)
			if(best_score[r] < best_score[winner])
				winner = r;
		attempt.winner    = winner;
		attempt.scale = best_scale_r[winner];
		attempt.dx = best_dx_r[winner]; attempt.dy = best_dy_r[winner];
		attempt.recovered = (winner == 0) && (best_score[0] <= kPuckRecoveryMatchThreshold);
	}
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
// M05-08 (task 0126): board (community) card template-match recognition.
//
// Analogous to 0121's VsmScorePuckRoles, but a 53-way argmin instead of a
// 3-way one: the 52 real-card references (TexasHoldemGetCardReferenceImage)
// PLUS the one holder ("not yet dealt") reference relevant to this
// board_card sub-slot's own index (TexasHoldemGetBoardHolderReferenceImage) -
// see this file's guardrail comment for why these are NEW functions rather
// than an overload of VsmScorePuckRoles (different vocabulary size, entirely
// separate role). VsmMeanAbsPixelDiff itself is reused unchanged.
//
// Theme: "default_800x480", NOT kPuckReferenceTheme's "default" - see task
// 0126's Manager task file for the full empirical investigation. Confirmed by
// reading GameTable::LoadTheme (GameTable.cpp:1225-1228): currentCardTheme is
// read from config key "CardTheme" and falls back to the literal string
// "default_800x480" (not "default") whenever that key is unset - which is
// exactly the case for every --record-session run (no config file/key is
// ever written by the recorder). This does NOT, in practice, point at a
// different asset directory: CardRender::ResolveCardArtPath (game/CardRender/
// CardRender.cpp) itself falls back to the "default" theme directory whenever
// the requested theme's own directory doesn't have the file (and this
// checkout only ships share/imgs/cards/default/*, confirmed by directory
// listing - no share/imgs/cards/default_800x480/ exists), so
// TexasHoldemGetCardReferenceImage("default_800x480") still resolves to the
// real card art. Kept as its own named constant (not literally "default")
// specifically so this doesn't silently start being wrong if a real
// "default_800x480" card-art directory is ever added later - see this task's
// evidence section for the real, printed, non-empty image sizes this was
// verified against (not assumed).
static const char* kCardReferenceTheme = "default_800x480";

// 52 real cards (index 0..51) + 1 holder reference (index 52) for the given
// board_card sub-slot index.
static const int kCardVocabSize = 53;
static const int kCardHolderVocabIndex = 52;

// GameTable::GetCardImage's own fixed request size (game/TexasHoldem/
// GameTable.cpp:267's `LoadCardArt(filename, Size(48, 76), currentCardTheme)`
// call) - see VsmScoreCardSlot's doc comment for why this task's reference
// building requests the card art at THIS native size and fits it itself,
// rather than asking LoadCardArt to stretch straight to the candidate rect's
// size.
static const Size kCardNativeSize(48, 76);

// Mirrors VsmScorePuckRoles's contract: crops `candidate_rect` out of
// `frame_img`, scores it against all 53 references, and returns the argmin.
// Getting a usable score out of this required TWO separate, empirically-
// forced fixes over a naive first attempt (both documented in full below,
// with the real before/after numbers in this task's evidence section):
//
// FIX 1 - letterbox-aware reference compositing, not a plain stretch. An
// EARLIER version asked TexasHoldemGetCardReferenceImage for the reference
// already STRETCHED to the candidate rect's own (w,h) - the same "plain
// Rescale, no aspect preservation" shortcut VsmScorePuckRoles already uses
// for the 3 (near-square, simple) puck references. For cards this was
// measured to be actively wrong, not just imprecise: GameTable::PaintBoard
// never stretches a card to fill its slot - it draws GetCardImage's fixed-
// 48x76-native image through FitCardArt (aspect-preserving fit, game/
// CardRender/CardRender.cpp) centered over whatever background is already
// there (the same Color(0,80,0) felt fallback TexasHoldemGetBoardHolderReferenceImage's
// own tier-3 fallback uses - confirmed no themed table.png/felt asset exists
// in this checkout, so PaintBoard's own `tableImg.IsEmpty()` branch is what
// always runs, see GameTable.cpp:1260-1265). board_w:board_h (122:143
// design-space, task 0126's Manager task file) is a visibly different aspect
// ratio than a card's native 48:76, so the real on-screen card only fills
// part of its slot rect's width, with felt-green visible on both sides - a
// plain full-rect stretch of the SAME card is a materially different image
// (glyphs pushed outward, green edges replaced by stretched card content),
// and measured scores confirmed every genuine dealt card scored WORSE
// against every one of the 52 stretched card references than against the
// (wrong) holder reference - recognition was unusable. Rebuilding the
// reference the way PaintBoard itself actually composites it -
// FitCardArt(kCardNativeSize image, slot size), centered over a
// Color(0,80,0)-filled canvas (see ComposeFitted below) - fixed this.
//
// FIX 2 - correcting for a real geometry mismatch between the candidate rect
// and the true on-screen rect (see VsmEmpiricalBoardCardRect immediately
// below for the full diagnosis) - fixing the letterbox compositing alone
// (FIX 1) was NOT sufficient on its own, since it still centered the
// (correctly-composited) reference over the WRONG position/size.
// ---------------------------------------------------------------------------
// IMPORTANT empirical finding (see this task's evidence section for the full
// diagnosis): the board_card_N candidate rect this comparator gets from
// VsmBuildCandidates (task 0113/0114's FormLayout.cpp replica of
// game/Poker/TableLayoutProfile.cpp's `board_x/y/step/w/h` constants, scaled
// by owner_rect="Board" element's declared .form size) does NOT match the
// REAL on-screen board_card rect `GameTable::PaintBoard` itself computes at
// actual record time (`TexasTableLayout::BoardCardRect(board.GetSize(), i)`).
// Measured directly (temporary instrumentation added to and removed from
// GameTable::PaintBoard for this one diagnosis - see evidence section): for
// EVERY session recorded with this task's standard recipe (`--provider
// PS_6p ... --seed N`, default table size, decoding to a 1024x625 frame -
// the same frame size every fixture in this and prior VSM tasks has used),
// the REAL board_card_i rect is fixed and slot-independent in shape:
// left=362+60*i, top=169, w=50, h=76 (i=0..4) - vs. the candidate list's
// left=341+69*i, top=201, w=65, h=82. Root cause: GameTable's actual "Board"
// Ctrl resolves to a materially smaller runtime size than its raw
// `.form`-declared (0,0,1024,648) rect once the Form's own scale_mode=2
// letterboxing is applied - a runtime behavior VsmParseFormFile/
// VsmBuildLayoutProfile (uppsrc/VisualStateModel/FormLayout.cpp, task
// 0113/0114/0117, EXPLICITLY not modifiable by this task's guardrails) does
// not replicate. This is almost certainly the same underlying reason 0122
// needed its own scale/position recovery search for button_puck, just with
// a bigger effect size for board cards (a real size difference, not just a
// few pixels of position slop).
//
// Since the corrective transform is fixed for this fixed-recipe corpus (not
// a per-frame unknown), this task corrects for it DIRECTLY (an explicit,
// documented, evidence-derived rect override - scaled proportionally if a
// session's decoded frame size ever differs from the 1024x625 baseline this
// was measured against) rather than via a wide blind scale/offset SEARCH: an
// earlier version of this function tried exactly such a search (5 scale
// steps x 5x5 offsets per reference, in 0122's style) and it was
// significantly worse in TWO ways found from real measured scores (see
// evidence section): (1) a wide search gives the flat-color holder reference
// a trivial, near-always-available near-perfect match on SOME nearby
// felt-green patch regardless of whether a real card is genuinely showing
// (measured: a definite river-street real card scored a false PERFECT 0.0
// "holder" match), and (2) even after removing the search from the holder
// reference, the 52-way real-card argmin still frequently picked the wrong
// card (the true card ranked ~11th of 52 in one measured case) because a
// coarse grid search rarely lands exactly on the true alignment for a small,
// glyph-detailed image the way it can for a large flat-color one. Directly
// using the known-correct rect fixes the alignment outright; only a SMALL
// residual tolerance (kCardResidualScaleSteps/kCardResidualOffsetFractions
// below) is kept, for frame-to-frame/session-to-session jitter around that
// otherwise-precise corrected position - not for absorbing a large unknown
// mismatch anymore.
static Rect VsmEmpiricalBoardCardRect(int card_index, Size frame_size)
{
	// Baseline measured at frame_size (1024,625) - see this function's
	// header comment above.
	const double base_frame_w = 1024.0, base_frame_h = 625.0;
	const double base_left0 = 362.0, base_step = 60.0, base_top = 169.0;
	const double base_w = 50.0, base_h = 76.0;
	double sx = frame_size.cx / base_frame_w;
	double sy = frame_size.cy / base_frame_h;
	int left = (int)((base_left0 + card_index * base_step) * sx);
	int top  = (int)(base_top * sy);
	int w    = max(1, (int)(base_w * sx));
	int h    = max(1, (int)(base_h * sy));
	return RectC(left, top, w, h);
}

// Small residual tolerance around the corrected rect above (frame-to-frame
// jitter, not the big systematic mismatch VsmEmpiricalBoardCardRect already
// fixes) - same per-reference independent best-of-(scale,offset) search
// shape 0122's VsmSearchPuckRecovery uses, just much narrower now that the
// large known offset is already corrected for.
static const double kCardResidualScaleSteps[] = { 0.92, 1.0, 1.08 };
static const int    kCardResidualScaleStepCount =
	(int)(sizeof(kCardResidualScaleSteps) / sizeof(kCardResidualScaleSteps[0]));
static const int kCardResidualPadding = 6;
static const double kCardResidualOffsetFractions[] = { -1.0, 0.0, 1.0 };
static const int    kCardResidualOffsetFractionCount =
	(int)(sizeof(kCardResidualOffsetFractions) / sizeof(kCardResidualOffsetFractions[0]));

static bool VsmScoreCardSlot(const Image& frame_img, const Rect& candidate_rect, int card_index,
                               double scores_out[kCardVocabSize], int& winner_out)
{
	Rect frame_rect(0, 0, frame_img.GetWidth(), frame_img.GetHeight());
	if(candidate_rect.GetWidth() <= 0 || candidate_rect.GetHeight() <= 0)
		return false;

	Rect corrected = VsmEmpiricalBoardCardRect(card_index, frame_img.GetSize());
	int base_w = corrected.GetWidth(), base_h = corrected.GetHeight();
	if(base_w <= 0 || base_h <= 0 || !frame_rect.Contains(corrected))
		return false;
	int ccx = corrected.left + base_w / 2;
	int ccy = corrected.top  + base_h / 2;

	// Byte-for-byte the same compositing GameTable::PaintBoard's own
	// DrawFitted lambda performs (GameTable.cpp:1255-1259, 1276-1288): fit
	// the card art into (w,h) preserving aspect, center it over the same
	// Color(0,80,0) felt fallback the holder reference itself falls back to.
	auto ComposeFitted = [&](const Image& native, int w, int h) -> Image {
		Image fitted = FitCardArt(native, Size(w, h));
		ImageDraw canvas(w, h);
		canvas.Alpha().DrawRect(0, 0, w, h, White());
		canvas.DrawRect(0, 0, w, h, Color(0, 80, 0));
		if(!fitted.IsEmpty())
			canvas.DrawImage((w - fitted.GetWidth()) / 2, (h - fitted.GetHeight()) / 2, fitted);
		return canvas;
	};

	// Best-of-(scale,offset) score for one real card's NATIVE (kCardNativeSize)
	// reference art. IMPORTANT: this search-tolerant scoring is deliberately
	// NOT applied to the holder reference (see below) - a holder is a
	// perfectly uniform Color(0,80,0) fill (this checkout ships no themed
	// cardholder_*.png assets, confirmed empirically, see this task's
	// evidence section), and scoring a textureless flat-color reference over
	// a wide scale/position search is close to vacuous: almost any nearby
	// felt-green patch, at almost any scale/offset, scores near zero against
	// it, regardless of whether a real card is ALSO genuinely showing
	// somewhere in the unsearched candidate rect - measured directly (see
	// evidence section): giving the holder reference the SAME search freedom
	// as the 52 card references made a definite river-street real card score
	// a perfect 0.0 "holder" match, because the search found an unrelated
	// green patch nearby, silently masking the genuine dealt card. Real
	// cards, by contrast, genuinely need the search (see this function's
	// header comment for why the candidate rect's nominal geometry doesn't
	// match the real on-screen card rect) since their reference has actual
	// glyph content that only lines up at the true position/scale.
	auto ScoreAgainstNative = [&](const Image& native) -> double {
		double best = DBL_MAX;
		for(int si = 0; si < kCardResidualScaleStepCount; si++) {
			double scale = kCardResidualScaleSteps[si];
			int w = max(1, (int)(base_w * scale + 0.5));
			int h = max(1, (int)(base_h * scale + 0.5));
			Image ref = ComposeFitted(native, w, h);
			for(int yi = 0; yi < kCardResidualOffsetFractionCount; yi++) {
				int dy = (int)(kCardResidualOffsetFractions[yi] * kCardResidualPadding
				               + (kCardResidualOffsetFractions[yi] >= 0 ? 0.5 : -0.5));
				for(int xi = 0; xi < kCardResidualOffsetFractionCount; xi++) {
					int dx = (int)(kCardResidualOffsetFractions[xi] * kCardResidualPadding
					               + (kCardResidualOffsetFractions[xi] >= 0 ? 0.5 : -0.5));
					int left = ccx - w / 2 + dx, top = ccy - h / 2 + dy;
					Rect shifted(left, top, left + w, top + h);
					if(!frame_rect.Contains(shifted))
						continue;
					Image cropped = Crop(frame_img, shifted);
					double d = VsmMeanAbsPixelDiff(cropped, ref);
					if(d < best)
						best = d;
				}
			}
		}
		return best;
	};

	for(int card = 0; card < 52; card++) {
		Image native = TexasHoldemGetCardReferenceImage(card, kCardNativeSize, kCardReferenceTheme);
		scores_out[card] = ScoreAgainstNative(native);
	}
	// Holder: scored ONLY at the corrected rect, unsearched - see this
	// function's comment above for why the offset/scale tolerance is
	// deliberately withheld here.
	{
		Image candidate = Crop(frame_img, corrected);
		Image holder_ref = TexasHoldemGetBoardHolderReferenceImage(card_index, Size(base_w, base_h), kCardReferenceTheme);
		scores_out[kCardHolderVocabIndex] = VsmMeanAbsPixelDiff(candidate, holder_ref);
	}

	winner_out = 0;
	for(int i = 1; i < kCardVocabSize; i++)
		if(scores_out[i] < scores_out[winner_out])
			winner_out = i;
	return true;
}

// Confidence-margin acceptance gate: the winner is only trusted if it beats
// the SECOND-best of the 53 candidates by at least this much (absolute mean-
// abs-pixel-diff units), not merely by having the lowest raw score - unlike
// 0122's kPuckRecoveryMatchThreshold (an absolute cap on the winner's own
// score), a margin-over-runner-up is the right shape of check here because
// two DIFFERENT systematic score-inflation effects (see VsmScoreCardSlot's
// and VsmEmpiricalBoardCardRect's comments above: the FitCardArt letterbox
// background, and any small residual mis-alignment the narrow residual
// search doesn't perfectly cancel) raise EVERY one of the 53 scores by
// roughly the same amount, so the winner's raw absolute score is not by
// itself a reliable "is this real" signal - the GAP between the best and
// second-best candidate is, since that shared inflation cancels out of the
// difference.
//
// See this task's evidence section for the real measured scores this was
// picked against (gathered AFTER VsmEmpiricalBoardCardRect's rect correction
// was in place - see that function's comment for why an earlier, uncorrected
// version of this scoring needed a much bigger, unreliable margin instead):
// across var/vsm_fixtures/task0126_seedA (seed 101) and task0126_seedB (seed
// 202), 30-frame PS_6p recordings each reaching the river (every accepted
// card-slot observation in both fixtures - 90 in seedA, 88 in seedB - agreed
// with ground truth, 0 mismatches), EVERY genuine recognized-real-card
// observation (i.e. the 52-way argmin's own winner equalled ground truth for
// that slot) beat its runner-up by >= 2.14 (the single smallest margin
// observed, seedA frame 13 slot 1 - card 13 at 36.55 vs. runner-up at 38.69);
// every genuine holder-wins observation (a not-yet-dealt slot correctly
// recognized as empty) beat its runner-up by >= 108 (holder scores in the low
// single digits to low teens, vs. the best real-card score in the 120s any
// time the true content really is a flat felt-colored holder). No incorrect
// argmin (a wrong card or a wrong holder/card call) was observed anywhere in
// either fixture to compare against - see evidence section for the full
// per-slot/per-frame table. 2.0 sits just below the smallest genuine real-
// card margin measured (2.14) while still requiring a non-trivial, non-tied
// separation (rejecting an exact or near-exact score tie, which real
// recognitions never produce) - this is a THIN margin above the observed
// floor (not a wide safety cushion like 0122's puck threshold enjoyed),
// documented honestly: a future session with slightly noisier capture could
// plausibly produce a genuine case just under 2.0, which this gate would
// then reject as "pending" rather than misreport as wrong (a false NEGATIVE,
// not a false positive - the safer failure direction for a recognition gate).
static const double kCardMatchMinMargin = 2.0;

// One accepted board_card-role recognition: sticky value assignment for one
// slot. value: 0..51 for a recognized card, -1 if the holder ("not yet
// dealt") reference won this observation's 53-way argmin.
struct VsmBoardCardObservation : Moveable<VsmBoardCardObservation> {
	int frame      = -1;
	int card_index = -1; // which board_card_N sub-slot (0..4)
	int value      = -2; // -1 == holder, 0..51 == recognized card
};

// M05-08 (task 0126): one change-triggered direct-probe attempt of a
// board_card_N candidate rect (see the file-header comment above
// GUI_APP_MAIN's probe loop for why this exists), kept for this task's
// evidence-printing table - mirrors VsmPuckRecoveryAttempt's role (real
// scores printed, not just the final accept/reject verdict).
struct VsmBoardCardProbeAttempt : Moveable<VsmBoardCardProbeAttempt> {
	int    frame      = -1;
	int    card_index = -1;
	bool   scored        = false;
	double score_best     = -1, score_runnerup = -1;
	int    winner         = -1; // 0..51 recognized card, 52 == holder
	bool   accepted       = false;
};

// Per-slot sticky-since-last-observation state: 5 INDEPENDENT tracks (one per
// board_card_N sub-slot), each holding a VALUE (-1 or 0..51), not just a
// boolean/seat the way DeriveDealerSeatPerFrame does for the single dealer
// seat - see this file's header comment / task 0126's Manager task file for
// why board cards need value-level per-slot stickiness (a board slot must
// correctly RESET to -1 at a new hand's preflop, which "recognizing the
// holder reference as the winner in a later frame" already naturally
// expresses with no separate hand-boundary-detection logic needed: this
// function always takes the LATEST observation for a given (slot, frame),
// whatever value it carries, exactly the way DeriveDealerSeatPerFrame takes
// the latest dealer-seat observation for a frame).
//
// A deliberately NEW function rather than a generic-parameterized reuse of
// DeriveDealerSeatPerFrame, per this file's guardrail against
// repurposing/overloading the puck-specific functions for a different role.
static void DeriveBoardCardsPerFrame(const Vector<VsmBoardCardObservation>& observations,
                                       int frame_lo, int frame_hi,
                                       Vector<bool> known_out[5], Vector<int> value_out[5])
{
	for(int slot = 0; slot < 5; slot++) {
		VectorMap<int, int> value_by_frame;
		for(const VsmBoardCardObservation& o : observations)
			if(o.card_index == slot)
				value_by_frame.GetAdd(o.frame, o.value) = o.value;

		Vector<bool>& known = known_out[slot];
		Vector<int>&  value = value_out[slot];
		known.Clear();
		value.Clear();
		bool k = false;
		int  v = -1;
		for(int fid = frame_lo; fid <= frame_hi; fid++) {
			int i = value_by_frame.Find(fid);
			if(i >= 0) { k = true; v = value_by_frame[i]; }
			known.Add(k);
			value.Add(v);
		}
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

	// --- Derive per-frame board-card slot values (frame-0 seed + every
	// accepted per-transition recognition above), 5 independent sticky
	// tracks. ---
	Vector<bool> board_known[5];
	Vector<int>  board_value[5];
	DeriveBoardCardsPerFrame(board_card_observations, 0, info.frame_count - 1, board_known, board_value);

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
