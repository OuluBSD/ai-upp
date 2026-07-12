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
