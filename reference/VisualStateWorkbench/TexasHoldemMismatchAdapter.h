#ifndef _VisualStateWorkbench_TexasHoldemMismatchAdapter_h_
#define _VisualStateWorkbench_TexasHoldemMismatchAdapter_h_

// ---------------------------------------------------------------------------
// TexasHoldem ground-truth mismatch adapter (task 0134 / M06-04).
//
// GUI-independent (NO CtrlLib / Ctrl dependency): for one frame of a loaded
// TexasHoldem M01/M02 session, builds one row per resolved field (dealer
// seat, each board_card slot, each seat's action icon, each seat's hole
// cards) with:
//   - the ground-truth ("expected") value, read directly off
//     VsmTexasHoldemSession::GroundTruthForFrame() (task 0131) — no new
//     ground-truth parsing.
//   - the derived ("parsed") value and per-field verdict, read directly off
//     task 0133's VsmSessionLogicModel / VsmLogicCompareRecordOut
//     (uppsrc/VisualStateLogic/LogicCompare.h) — the SAME comparison
//     uppsrc/VisualStateLogic/LogicCompare.cpp already performs. Where that
//     record only exposes an AGGREGATE verdict (e.g.
//     board_cards_match_slots/mismatch_slots/pending_slots), this adapter
//     exposes the SAME decision at per-row granularity by repeating the
//     identical ">= 0 ? compare : pending" / "known ? compare : unscored"
//     comparisons LogicCompare.cpp's own per-frame loops already perform on
//     data LogicCompare.cpp already computed and returned per-seat/per-slot
//     (TexasHoldemLogicPlayerState::action/hole_cards, TexasHoldemLogicState
//     ::board_cards) — no NEW recognition, scoring, or threshold logic is
//     introduced here; this file is a read-only consumer of both
//     VsmTexasHoldemSession and VsmSessionLogicModel.
//   - a REGION CROP of the relevant `.form` sub-slot rect from the real
//     current frame. The crop geometry (kCropPadding clamp margin) is the
//     SAME geometry uppsrc/VisualStateModel/FrameCrop.h's
//     VsmSaveRegionCropPng uses for reference/VisualStateLayoutAssign's own
//     `--crop-report-out` (confirmed byte-identical against that function on
//     an equivalent rect — see this task's evidence section); this adapter
//     returns an in-memory Image (via the same VsmFrameImageToImage +
//     Crop() primitives) instead of writing a PNG file, since a live,
//     frame-scrubbable panel needs a fresh in-memory crop per row per frame,
//     not a growing pile of temp files.
//   - an EXPECTED REFERENCE IMAGE via the exact reference-art functions
//     named in this task's spec (game/TexasHoldem/TexasHoldemLogicState.h):
//     TexasHoldemGetPuckReferenceImage / TexasHoldemGetCardReferenceImage /
//     TexasHoldemGetBoardHolderReferenceImage /
//     TexasHoldemGetCardBackReferenceImage /
//     TexasHoldemGetActionIconReferenceImage /
//     TexasHoldemGetActionIconEmptyReferenceImage — called directly, not
//     re-derived.
//
// Being CtrlLib-free, this whole adapter can be exercised from a
// CONSOLE/GUI_APP_MAIN harness (the project's "Headless/GUI Dual-Purpose
// Rule"), which is how it is verified — the GUI panel that consumes it
// cannot be driven by a background agent.
// ---------------------------------------------------------------------------

#include "TexasHoldemLayoutBindingAdapter.h"
#include "TexasHoldemLogicStateAdapter.h"

namespace Upp {

// One ground-truth-vs-derived comparison row for one frame.
struct VsmMismatchRow : Moveable<VsmMismatchRow> {
	String field;         // e.g. "dealer_seat", "board_card_2", "action_icon[seat 3]", "hole_cards[seat 1]"
	String verdict;        // "match" / "mismatch" / "pending" / "hidden" / "winner"
	String expected;       // ground-truth value, formatted for display
	String parsed;          // derived value, formatted for display

	bool has_region  = false; // true if a `.form` candidate rect was resolved for this row
	Rect region_rect = Rect(0, 0, 0, 0); // frame-pixel rect the crop/reference correspond to

	bool  has_crop = false;
	Image crop_image;       // real frame pixels at region_rect (+ FrameCrop.h's kCropPadding)

	bool  has_reference = false;
	Image reference_image;  // "what the ground-truth value should look like"
};

// Builds every resolved-verdict row for `frame_id` of `session`, using
// `layout_model` (task 0132, for `.form` candidate rects — frame-independent,
// reused as-is) and `logic_model` (task 0133, for the derived values/verdicts
// — already computed per frame, this function only performs an O(1) lookup
// plus the row-level comparisons documented above). `frame_img` must be the
// already-decoded current frame (VsmLoadTexasHoldemFrameImage output,
// converted via VsmFrameImageToImage) — passed in rather than re-decoded here
// so a caller that already holds the frame (as MainWindow::SetTexasFrame
// already does for FrameCanvas) doesn't pay for a second decode.
//
// A row is included only when its underlying field has a RESOLVED verdict
// this frame (dealer seat: derived_dealer_seat_known; board_cards: only when
// TexasHoldemLogicState::board_cards_known; action_icon: only when that
// seat's action_known; hole_cards: only when that seat's hole_cards_known) —
// unresolved fields produce no row for this frame, per this task's spec
// ("every field with a resolved verdict this frame"). Rows are returned in a
// stable order: dealer_seat, board_card_0..4, action_icon per seat (in
// `logic_state.players` order), hole_cards per seat (same order).
Vector<VsmMismatchRow> VsmBuildMismatchRows(const VsmTexasHoldemSession& session,
                                            const VsmSessionLayoutModel& layout_model,
                                            const VsmSessionLogicModel& logic_model,
                                            int frame_id, const Image& frame_img);

// Scans `model.records` starting at `from_frame_id + (forward ? 1 : -1)` and
// returns the first frame_id (moving in that direction) whose record has ANY
// mismatch verdict (verdict == "mismatch" for the dealer seat, or
// board_cards_verdict/action_icons_verdict/hole_cards_verdict == "mismatch")
// — reusing the SAME aggregate verdict fields VsmLogicCompareRecordOut
// already computes (no per-row rebuild needed just to answer "does this
// frame have any mismatch"). Returns -1 if none found (including when
// `model` is not loaded).
int VsmFindNextMismatchFrame(const VsmSessionLogicModel& model, int from_frame_id, bool forward);

} // namespace Upp

#endif
