#ifndef _VisualStateLogic_LogicCompare_h_
#define _VisualStateLogic_LogicCompare_h_

// ---------------------------------------------------------------------------
// M06-03 (task 0133): shared M05 logic-state scoring/derivation logic,
// extracted verbatim from reference/VisualStateLogicCompare/main.cpp's
// file-local `static` functions (tasks 0119-0128) so a GUI panel
// (reference/VisualStateWorkbench's logic-state timeline) can call the SAME
// recognition/derivation code the CLI uses, without duplicating it — the same
// rationale task 0117 used when it extracted M04's matching logic into
// uppsrc/VisualStateModel/RegionAssign.h (see that header), and the project's
// own "Headless/GUI Dual-Purpose Rule" (shared behavior belongs in common
// helper code, not copied into both consumers).
//
// WHY A NEW SIBLING LIBRARY PACKAGE (`uppsrc/VisualStateLogic`), NOT MERGED
// INTO `uppsrc/VisualStateModel/`:
// the task spec suggested `uppsrc/VisualStateModel/LogicCompare.h` as a
// natural-sounding location, but the scoring functions below call
// `TexasHoldemGet*ReferenceImage`/`kActionIconVocabWinner` (game/TexasHoldem)
// and `FitCardArt` (game/CardRender). `uppsrc/VisualStateModel` deliberately
// depends ONLY on Core/Draw/Painter/png (see its .upp `uses` list) —
// PngFrame.h's own header comment states this explicitly: "VSM is a
// lower-layer, headless package and must not depend upward on game code".
// Adding these files' TexasHoldem/CardRender dependency directly to
// VisualStateModel.upp would invert that documented layering AND silently
// drag CtrlLib/Form/Sql/sqlite3/SSL/CardEvaluator/GameRules/Poker/CardRender
// onto every one of VisualStateModel's existing consumers that today link
// only Core/Draw/Painter/png — confirmed empirically: `upptst/
// VisualStateModelTests`, `reference/VisualStateRegionDump`, `reference/
// VisualStateSessionValidate`, `reference/VisualStateSessionDiff`, and
// `reference/VisualStateBatchReport` all currently `uses VisualStateModel;`
// alone. That is a real, unwanted blast-radius increase for what must be a
// behavior-preserving, low-risk pure extraction — not merely a style
// preference.
//
// Instead, this is a NEW sibling package at the SAME `uppsrc/` library tier
// as VisualStateModel (not a `reference/`-tier CLI/GUI tool package — a
// `reference/` package's own layering role is "consumer/tool", never a
// shared library two other tools both depend on). It `uses VisualStateModel,
// TexasHoldem, CardRender` — one tier ABOVE VisualStateModel, exactly where a
// component that genuinely needs both belongs — so BOTH real consumers, the
// CLI (`reference/VisualStateLogicCompare`) and the GUI (`reference/
// VisualStateWorkbench`), add a plain `uses VisualStateLogic;` (alongside
// their own existing `uses VisualStateModel, TexasHoldem, CardRender;`) and
// share one implementation, the same way both already directly depend on all
// three of those packages today. `uppsrc/VisualStateModel/RegionAssign.h`
// (task 0117's own M04 precedent this task mirrors) did not face this
// problem because ITS logic is TexasHoldem-agnostic pure geometry — this
// extraction's functions are not; they call real TexasHoldem/CardRender
// asset-reference helpers directly, so the dependency has to sit somewhere
// real, and putting it above VisualStateModel (not inside it) is the
// smallest change that keeps every existing consumer's dependency footprint
// unchanged.
//
// This was a PURE extraction: no scoring/threshold/derivation logic changed
// (proven byte-for-byte against the six kept M05 fixtures — see task 0133's
// evidence section). The extensive per-function rationale comments (the real
// empirical findings behind each threshold/geometry correction) live with the
// definitions in LogicCompare.cpp, unchanged from the CLI's own source.

#include <VisualStateModel/VisualStateModel.h>
#include <TexasHoldem/TexasHoldemSessionContract.h>
#include <TexasHoldem/TexasHoldemLogicState.h>
#include <CardRender/CardRender.h>

namespace Upp {

// ---------------------------------------------------------------------------
// Threshold / vocabulary constants (moved verbatim from the CLI). Header-scope
// `static const` (internal linkage per translation unit) — the same simplest-
// option choice RegionAssign.h's `kOverlapThreshold` and FrameCrop.h's
// `kCropPadding` already made for small shared constants.

// M05-03 (task 0121): puck reference theme.
static const char* const kPuckReferenceTheme = "default";

// M05-04 (task 0122): scale/position-tolerant puck recovery search bounds.
static const int    kPuckRecoveryPadding = 24;
static const double kPuckRecoveryScaleSteps[] = { 0.8, 0.9, 1.0, 1.1, 1.2, 1.3 };
static const int    kPuckRecoveryScaleStepCount =
	(int)(sizeof(kPuckRecoveryScaleSteps) / sizeof(kPuckRecoveryScaleSteps[0]));
static const double kPuckRecoveryOffsetFractions[] = { -1.0, -0.5, 0.0, 0.5, 1.0 };
static const int    kPuckRecoveryOffsetFractionCount =
	(int)(sizeof(kPuckRecoveryOffsetFractions) / sizeof(kPuckRecoveryOffsetFractions[0]));
static const double kPuckRecoveryMatchThreshold = 70.0;

// M05-08 (task 0126): board-card 53-way vocabulary + card art geometry.
static const char* const kCardReferenceTheme = "default_800x480";
static const int  kCardVocabSize = 53;
static const int  kCardHolderVocabIndex = 52;
static const Size kCardNativeSize(48, 76);
static const double kCardResidualScaleSteps[] = { 0.92, 1.0, 1.08 };
static const int    kCardResidualScaleStepCount =
	(int)(sizeof(kCardResidualScaleSteps) / sizeof(kCardResidualScaleSteps[0]));
static const int    kCardResidualPadding = 6;
static const double kCardResidualOffsetFractions[] = { -1.0, 0.0, 1.0 };
static const int    kCardResidualOffsetFractionCount =
	(int)(sizeof(kCardResidualOffsetFractions) / sizeof(kCardResidualOffsetFractions[0]));
static const double kCardMatchMinMargin = 2.0;

// M05-09 (task 0127): action-icon vocabulary + acceptance gate.
static const int kActionIconRealVocab[7] = { 1, 2, 3, 4, 5, 6, kActionIconVocabWinner };
static const int kActionIconRealVocabCount = 7;
static const int kActionIconEmptyWinnerValue = 0;
static const double kActionIconMatchMaxScore = 40.0;
static const double kActionIconMatchMinMargin = 2.0;

// M05-10 (task 0128): hole-card vocabulary + acceptance gate.
static const int kHoleCardVocabSize = 53;
static const int kHoleCardBackVocabIndex = 52;
static const double kHoleCardMatchMinMargin = 1.2;

// ---------------------------------------------------------------------------
// One region-to-layout-element observation, same shape as
// reference/VisualStateLayoutAssign/main.cpp's VsmLayoutObservationOut, plus
// the M05-03/08/09/10 template-match score fields the recognizers fill in.
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

	// M05-03 (task 0121): dealer_button puck-role disambiguation scores.
	bool   puck_scored = false;
	double puck_score_dealer = -1, puck_score_sb = -1, puck_score_bb = -1;
	int    puck_role_winner = -1;

	// M05-08 (task 0126): board_card template-match scores.
	bool   card_scored = false;
	int    card_winner = -1;
	double card_score_best = -1, card_score_runnerup = -1;

	// M05-09 (task 0127): action_icon template-match scores.
	bool   action_icon_scored = false;
	int    action_icon_winner = -2;
	double action_icon_score_best = -1, action_icon_score_runnerup = -1;

	// M05-10 (task 0128): hole_card template-match scores.
	bool   hole_card_scored = false;
	int    hole_card_winner = -2;
	double hole_card_score_best = -1, hole_card_score_runnerup = -1;

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

// M07-02 (task 0138): one per-field recognizer-confidence datum, surfaced for
// production/video-only mode where there is no ground truth to compare against
// and the caller instead needs "how sure was the parser". This carries the
// best-vs-runner-up match-score separation each M05 recognizer ALREADY computes
// at accept-gate time (see LogicCompare.cpp:VsmScore*), which today is used only
// for the boolean accept/reject decision and then discarded. Pure additive
// surfacing — no recognition/threshold/scoring behavior changes; the same data
// that already drove accept/reject is simply also reported.
//
// `field` uses the SAME naming convention task 0134's mismatch panel
// (VsmMismatchRow) established: "dealer_seat", "board_card_0".."board_card_4",
// "action_icon[seat N]", "hole_cards[seat N]".
//
// `confidence` is FAMILY-RELATIVE (0..1), NOT globally comparable across
// families — each recognizer family scores on its own scale with its own gate,
// so the confidence for each is expressed relative to that family's own
// existing threshold constants (see the per-family formula comments in
// LogicCompare.cpp). It is the confidence of the observation that established
// the field's CURRENT (sticky/carried-forward) value at this frame; when the
// field is not known (`known == false`) the confidence is 0.0.
struct VsmFieldConfidence : Moveable<VsmFieldConfidence> {
	String field;
	bool   known = false;
	double confidence = 0.0;

	void Jsonize(JsonIO& json)
	{
		json("field", field)("known", known)("confidence", confidence);
	}
};

// The per-frame comparator record. See LogicCompare.cpp / the CLI's own
// header comment for the exact per-field verdict semantics.
struct VsmLogicCompareRecordOut : Moveable<VsmLogicCompareRecordOut> {
	int frame_id = -1;

	bool derived_dealer_seat_known = false;
	int  derived_dealer_seat = -1;

	bool ground_truth_dealer_seat_known = false;
	int  ground_truth_dealer_seat = -1;

	String verdict;

	Vector<int> ground_truth_board_cards;
	int    board_cards_match_slots = 0, board_cards_mismatch_slots = 0, board_cards_pending_slots = 0;
	String board_cards_verdict;

	int    action_icon_match_seats = 0, action_icon_mismatch_seats = 0;
	int    action_icon_winner_seats = 0, action_icon_unscored_seats = 0;
	String action_icons_verdict;

	int    hole_cards_match_seats = 0, hole_cards_mismatch_seats = 0;
	int    hole_cards_hidden_seats = 0, hole_cards_unknown_seats = 0;
	String hole_cards_verdict;

	TexasHoldemLogicState logic_state;

	// M07-02 (task 0138): per-field recognizer confidence, additive. One entry
	// per field that carries a per-frame derived value (dealer seat, each of the
	// 5 board-card slots, each seat's action icon, each seat's hole cards).
	Vector<VsmFieldConfidence> field_confidence;

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
			("action_icon_match_seats", action_icon_match_seats)
			("action_icon_mismatch_seats", action_icon_mismatch_seats)
			("action_icon_winner_seats", action_icon_winner_seats)
			("action_icon_unscored_seats", action_icon_unscored_seats)
			("action_icons_verdict", action_icons_verdict)
			("hole_cards_match_seats", hole_cards_match_seats)
			("hole_cards_mismatch_seats", hole_cards_mismatch_seats)
			("hole_cards_hidden_seats", hole_cards_hidden_seats)
			("hole_cards_unknown_seats", hole_cards_unknown_seats)
			("hole_cards_verdict", hole_cards_verdict)
			("logic_state", logic_state)
			("field_confidence", field_confidence)
		;
	}
};

// M05-04 (task 0122): one (transition, seat) puck-recovery probe.
struct VsmPuckRecoveryAttempt : Moveable<VsmPuckRecoveryAttempt> {
	int    frame = -1;
	int    seat  = -1;
	bool   found = false;
	double scale = 0.0;
	int    dx = 0, dy = 0;
	double score_dealer = -1, score_sb = -1, score_bb = -1;
	int    winner = -1;
	bool   recovered = false;
};

// M05-08 (task 0126): one accepted board_card recognition.
struct VsmBoardCardObservation : Moveable<VsmBoardCardObservation> {
	int frame      = -1;
	int card_index = -1;
	int value      = -2; // -1 == holder, 0..51 == recognized card
	// M07-02 (task 0138): family-relative recognizer confidence [0..1] of this
	// accepted recognition (additive; internal — not Jsonized). See
	// VsmMarginConfidence in LogicCompare.cpp.
	double confidence = 0.0;
};

// M05-08 (task 0126): one change-triggered board_card direct-probe attempt.
struct VsmBoardCardProbeAttempt : Moveable<VsmBoardCardProbeAttempt> {
	int    frame      = -1;
	int    card_index = -1;
	bool   scored        = false;
	double score_best     = -1, score_runnerup = -1;
	int    winner         = -1;
	bool   accepted       = false;
};

// M05-09 (task 0127): one accepted action_icon recognition.
struct VsmActionIconObservation : Moveable<VsmActionIconObservation> {
	int frame = -1;
	int seat  = -1;
	int value = -2;
	// M07-02 (task 0138): family-relative recognizer confidence [0..1] (additive;
	// internal — not Jsonized). See VsmActionIconConfidence in LogicCompare.cpp.
	double confidence = 0.0;
};

// M05-09 (task 0127): one change-triggered action_icon direct-probe attempt.
struct VsmActionIconProbeAttempt : Moveable<VsmActionIconProbeAttempt> {
	int    frame = -1;
	int    seat  = -1;
	bool   scored         = false;
	double score_best     = -1, score_runnerup = -1;
	int    winner         = -2;
	bool   accepted       = false;
};

// M05-10 (task 0128): one accepted hole_card recognition.
struct VsmHoleCardObservation : Moveable<VsmHoleCardObservation> {
	int frame      = -1;
	int seat       = -1;
	int card_index = -1;
	int value      = -2;
	// M07-02 (task 0138): family-relative recognizer confidence [0..1] (additive;
	// internal — not Jsonized). See VsmMarginConfidence in LogicCompare.cpp.
	double confidence = 0.0;
};

// M05-10 (task 0128): one change-triggered hole_card direct-probe attempt.
struct VsmHoleCardProbeAttempt : Moveable<VsmHoleCardProbeAttempt> {
	int    frame      = -1;
	int    seat       = -1;
	int    card_index = -1;
	bool   scored         = false;
	double score_best     = -1, score_runnerup = -1;
	int    winner         = -1;
	bool   accepted       = false;
};

// ---------------------------------------------------------------------------
// M07-02 (task 0138): family-relative recognizer-confidence helpers. These
// derive a 0..1 "how sure was the parser" value from the SAME best/runner-up
// match scores each recognizer already computes at accept-gate time — no new
// scoring, no threshold change. Full per-family formula rationale lives with
// the definitions in LogicCompare.cpp.
//
// VsmMarginConfidence: for the card/hole-card families, whose accept gate is a
// single best-vs-runner-up margin >= min_margin. Returns
// margin/(margin + min_margin): 0 when there is no separation, exactly 0.5 at
// the family's own accept boundary, asymptotically ->1 as the winner pulls away.
double VsmMarginConfidence(double best, double runnerup, double min_margin);
// VsmActionIconConfidence: the action-icon family gates on BOTH an absolute
// max-score AND a min-margin, so its confidence is the min of a margin-relative
// and an absolute-quality sub-confidence (whichever gate is closer to failing).
double VsmActionIconConfidence(double best, double runnerup);
// VsmPuckDealerConfidence: the dealer/puck family has no min-margin constant —
// acceptance is a pure argmin over 3 mean-abs-pixel-diff role scores — so its
// confidence is the scale-free normalized contrast (runnerup - best)/(runnerup +
// best) between the winning dealer role and the next-best role.
double VsmPuckDealerConfidence(double dealer_score, double sb_score, double bb_score);

// M07-02 (task 0138): parallel, purely-additive per-frame confidence latches.
// Each mirrors the EXACT sticky/carry-forward (and, for action icons, the
// reset-on-empty) semantics of its value-deriving sibling above, but carries the
// confidence of the observation that established the current value instead of
// the value itself. Kept as separate functions so the byte-for-byte-proven
// value derivation (Derive*PerFrame) is not modified at all.
void DeriveBoardCardsConfidencePerFrame(const Vector<VsmBoardCardObservation>& observations,
                                        int frame_lo, int frame_hi,
                                        Vector<double> confidence_out[5]);
void DeriveActionIconsConfidencePerFrame(const Vector<VsmActionIconObservation>& observations,
                                         int frame_lo, int frame_hi,
                                         const Vector<int>& seats,
                                         VectorMap<int, Vector<double>>& confidence_out);
void DeriveHoleCardsConfidencePerFrame(const Vector<VsmHoleCardObservation>& observations,
                                       int frame_lo, int frame_hi,
                                       const Vector<int>& seats,
                                       VectorMap<int, Vector<double>> confidence_out[2]);
// Dealer seat: build a frame->confidence map from the same pre-filtered
// dealer-move observations ApplyDealerButtonObservations consumes (same
// last-wins-per-frame rule), then latch it sticky exactly like
// DeriveDealerSeatPerFrame latches the seat.
void ApplyDealerButtonConfidence(const Vector<VsmLayoutObservationOut>& observations,
                                 VectorMap<int, double>& dealer_confidence_by_frame);
void DeriveDealerSeatConfidencePerFrame(const VectorMap<int, double>& dealer_confidence_by_frame,
                                        int frame_lo, int frame_hi,
                                        Vector<double>& confidence_out);

// ---------------------------------------------------------------------------
// Leaf scoring/derivation functions (definitions + full rationale in
// LogicCompare.cpp). Moved verbatim from the CLI's file-local statics.

// M05-03 (task 0121): puck-role disambiguation.
double VsmMeanAbsPixelDiff(const Image& a, const Image& b);
bool   VsmScorePuckRoles(const Image& frame_img, const Rect& candidate_rect, double scores_out[3]);

// M05-04 (task 0122): scale/position-tolerant puck recovery search.
void VsmSearchPuckRecovery(const Image& frame_img, const Rect& candidate_rect,
                           VsmPuckRecoveryAttempt& attempt);

// Dealer-seat derivation (task 0119).
void ApplyDealerButtonObservations(const Vector<VsmLayoutObservationOut>& observations,
                                   VectorMap<int, int>& dealer_seat_by_frame);
void DeriveDealerSeatPerFrame(const VectorMap<int, int>& dealer_seat_by_frame,
                              int frame_lo, int frame_hi,
                              Vector<bool>& known_out, Vector<int>& seat_out);

// M05-08 (task 0126): board-card recognition + derivation.
Rect VsmEmpiricalBoardCardRect(int card_index, Size frame_size);
bool VsmScoreCardSlot(const Image& frame_img, const Rect& candidate_rect, int card_index,
                      double scores_out[kCardVocabSize], int& winner_out);
void DeriveBoardCardsPerFrame(const Vector<VsmBoardCardObservation>& observations,
                              int frame_lo, int frame_hi,
                              Vector<bool> known_out[5], Vector<int> value_out[5]);

// M05-09 (task 0127): action-icon recognition + derivation.
int  VsmActionIconRowParityOffset(int seat, Size frame_size);
bool VsmScoreActionIcon(const Image& frame_img, const Rect& candidate_rect, int seat,
                        double scores_out[kActionIconRealVocabCount + 1], int& winner_out);
void DeriveActionIconsPerFrame(const Vector<VsmActionIconObservation>& observations,
                               int frame_lo, int frame_hi,
                               const Vector<int>& seats,
                               VectorMap<int, Vector<bool>>& known_out,
                               VectorMap<int, Vector<int>>& value_out);

// M05-10 (task 0128): hole-card recognition + derivation.
int   VsmHoleCardRowParityOffset(const Rect& candidate_rect, int seat, Size frame_size);
Image VsmComposeHoleCardFitted(const Image& native, int w, int h, int row_parity_offset, bool is_winner);
bool  VsmScoreHoleCardSlot(const Image& frame_img, const Rect& candidate_rect, int seat,
                           double scores_out[kHoleCardVocabSize], int& winner_out);
void  DeriveHoleCardsPerFrame(const Vector<VsmHoleCardObservation>& observations,
                              int frame_lo, int frame_hi,
                              const Vector<int>& seats,
                              VectorMap<int, Vector<bool>> known_out[2],
                              VectorMap<int, Vector<int>> value_out[2]);

// ---------------------------------------------------------------------------
// Top-level, GUI-independent driver (task 0133 addition): loads an M01/M02
// session + `.form`, runs the full M05 derivation pipeline (the same
// orchestration reference/VisualStateLogicCompare's GUI_APP_MAIN performs,
// minus all of that CLI's stdout diagnostic tables), and returns one derived
// VsmLogicCompareRecordOut per frame. This is what the workbench's logic-state
// timeline panel calls directly. Its output is verified byte-for-byte
// identical to the CLI's own --jsonl-out for the six kept M05 fixtures (see
// task 0133's evidence section), so the two share the recognition math AND
// agree on the composed result.
//
// Returns false and fills `error` on any load/parse failure; on success
// `out_records` holds one record per ground-truth frame.
bool VsmDeriveSessionLogicStates(const String& session_dir, const String& form_path,
                                 Vector<VsmLogicCompareRecordOut>& out_records,
                                 String& error);

} // namespace Upp

#endif
