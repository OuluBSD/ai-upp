#include "TexasHoldemMismatchAdapter.h"

namespace Upp {

// ---------------------------------------------------------------------------
// Local helpers (no recognition/scoring logic — pure lookups/formatting)

// Finds the `.form` sub-slot candidate for one role/seat_index/card_index
// (either or both of seat_index/card_index may be -1, meaning "don't care",
// matching how dealer_button/board_card candidates only carry ONE of the two
// indices). Mirrors the exact `kind == "subslot" && role == ...` filters
// TexasHoldemLayoutBindingAdapter.cpp / LogicCompare.cpp already use when
// collecting *_candidates vectors — just a single lookup instead of a
// collected vector, since this adapter only ever wants one specific slot's
// rect at a time.
static const VsmLayoutCandidate* FindSubslotCandidate(const VsmSessionLayoutModel& model,
                                                       const char* role,
                                                       int seat_index, int card_index)
{
	if(!model.loaded)
		return nullptr;
	for(const VsmLayoutCandidate& c : model.candidates) {
		if(c.kind != "subslot" || c.role != role)
			continue;
		if(seat_index >= 0 && c.seat_index != seat_index)
			continue;
		if(card_index >= 0 && c.card_index != card_index)
			continue;
		return &c;
	}
	return nullptr;
}

// Same crop geometry as uppsrc/VisualStateModel/FrameCrop.h's
// VsmSaveRegionCropPng (kCropPadding margin, clamped to frame bounds), reusing
// that header's own VsmFrameImageToImage-derived Image + kCropPadding
// constant so the padding/clamp math is byte-identical to the CLI's own
// `--crop-report-out` crops — just returning an in-memory Image instead of
// writing a PNG file (verified byte-identical against VsmSaveRegionCropPng's
// own file output for an equivalent rect, see this task's evidence section).
static Image CropFrameRect(const Image& frame_img, const Rect& rect)
{
	if(frame_img.IsEmpty() || rect.IsEmpty())
		return Image();
	int x0 = max(0, rect.left - kCropPadding);
	int y0 = max(0, rect.top - kCropPadding);
	int x1 = min(frame_img.GetWidth(),  rect.right  + kCropPadding);
	int y1 = min(frame_img.GetHeight(), rect.bottom + kCropPadding);
	int cw = max(1, x1 - x0);
	int ch = max(1, y1 - y0);
	return Crop(frame_img, x0, y0, cw, ch);
}

static const TexasHoldemPlayerSnapshot* FindGtPlayer(const TexasHoldemGroundTruthRecord& gt, int seat)
{
	for(const TexasHoldemPlayerSnapshot& p : gt.players)
		if(p.seat == seat)
			return &p;
	return nullptr;
}

static String FormatCardValue(int v)
{
	return v < 0 ? String("none/back") : IntStr(v);
}

static String FormatSeatValue(bool known, int v)
{
	return known ? IntStr(v) : String("unknown");
}

static String FormatActionValue(bool known, int v)
{
	if(!known)
		return "unscored";
	if(v == kActionIconVocabWinner)
		return "winner-marker";
	return IntStr(v);
}

// ---------------------------------------------------------------------------
// Row builders — one per field kind. Each mirrors the SAME known/pending/
// match/mismatch decision LogicCompare.cpp's own per-frame comparison loop
// already makes (see that file's "board-card / action-icon / hole-card
// frame-by-frame comparison" sections) on data it already computed and
// returned (TexasHoldemLogicState / TexasHoldemLogicPlayerState) — this file
// only re-exposes that SAME decision at per-row (rather than only aggregated-
// count) granularity, plus attaches the crop/reference imagery.

static void AddDealerSeatRow(const VsmTexasHoldemSession& session,
                             const VsmSessionLayoutModel& layout_model,
                             const VsmLogicCompareRecordOut& rec,
                             const Image& frame_img, Vector<VsmMismatchRow>& rows)
{
	if(!rec.derived_dealer_seat_known)
		return; // no resolved verdict yet this frame

	VsmMismatchRow row;
	row.field    = "dealer_seat";
	row.verdict  = rec.verdict; // "match" or "mismatch" (never "unknown" here, guarded above)
	row.expected = FormatSeatValue(rec.ground_truth_dealer_seat_known, rec.ground_truth_dealer_seat);
	row.parsed   = IntStr(rec.derived_dealer_seat);

	const VsmLayoutCandidate* c = FindSubslotCandidate(layout_model, "dealer_button",
	                                                    rec.derived_dealer_seat, -1);
	if(c) {
		row.has_region   = true;
		row.region_rect  = c->rect;
		row.crop_image   = CropFrameRect(frame_img, c->rect);
		row.has_crop     = !row.crop_image.IsEmpty();
		// role 0 == dealer puck (TexasHoldemGetPuckReferenceImage's own
		// convention, matching VsmScorePuckRoles' scores_out[0]/kPuckReferenceTheme).
		Image ref = TexasHoldemGetPuckReferenceImage(0, kPuckReferenceTheme);
		if(!ref.IsEmpty() && ref.GetSize() != c->rect.GetSize())
			ref = Rescale(ref, c->rect.GetSize());
		row.reference_image = ref;
		row.has_reference    = !ref.IsEmpty();
	}

	rows.Add(pick(row));
}

static void AddBoardCardRows(const VsmTexasHoldemSession& session,
                             const VsmSessionLayoutModel& layout_model,
                             const VsmLogicCompareRecordOut& rec,
                             const Image& frame_img, Vector<VsmMismatchRow>& rows)
{
	const TexasHoldemLogicState& ls = rec.logic_state;
	if(!ls.board_cards_known)
		return; // matches LogicCompare.cpp's own "if(!all_known) verdict=unknown" gate

	int gt_count = rec.ground_truth_board_cards.GetCount();
	for(int slot = 0; slot < ls.board_cards.GetCount() && slot < 5; slot++) {
		int derived_v = ls.board_cards[slot];

		VsmMismatchRow row;
		row.field = Format("board_card_%d", slot);
		int gt_v = (slot < gt_count) ? rec.ground_truth_board_cards[slot] : -1;
		row.expected = FormatCardValue(gt_v);
		row.parsed   = FormatCardValue(derived_v);
		// Same "derived_v < 0 -> pending" test LogicCompare.cpp's board-card
		// comparison loop performs before counting match/mismatch.
		if(derived_v < 0)
			row.verdict = "pending";
		else
			row.verdict = (derived_v == gt_v) ? "match" : "mismatch";

		const VsmLayoutCandidate* c = FindSubslotCandidate(layout_model, "board_card", -1, slot);
		if(c) {
			row.has_region  = true;
			row.region_rect = c->rect;
			row.crop_image  = CropFrameRect(frame_img, c->rect);
			row.has_crop    = !row.crop_image.IsEmpty();
			Image ref = (gt_v < 0)
				? TexasHoldemGetBoardHolderReferenceImage(slot, c->rect.GetSize(), kCardReferenceTheme)
				: TexasHoldemGetCardReferenceImage(gt_v, c->rect.GetSize(), kCardReferenceTheme);
			row.reference_image = ref;
			row.has_reference    = !ref.IsEmpty();
		}

		rows.Add(pick(row));
	}
}

static void AddActionIconRows(const VsmTexasHoldemSession& session,
                              const VsmSessionLayoutModel& layout_model,
                              const VsmLogicCompareRecordOut& rec, int frame_id,
                              const Image& frame_img, Vector<VsmMismatchRow>& rows)
{
	const TexasHoldemGroundTruthRecord* gt = session.GroundTruthForFrame(frame_id);
	const TexasHoldemLogicState& ls = rec.logic_state;

	for(const TexasHoldemLogicPlayerState& ps : ls.players) {
		if(!ps.action_known)
			continue; // matches LogicCompare.cpp's own "if(!k) continue;"

		VsmMismatchRow row;
		row.field = Format("action_icon[seat %d]", ps.seat);

		const TexasHoldemPlayerSnapshot* gtp = gt ? FindGtPlayer(*gt, ps.seat) : nullptr;
		int gt_action = gtp ? gtp->action : -1;
		row.expected = gtp ? IntStr(gt_action) : String("no ground truth");
		row.parsed   = FormatActionValue(ps.action_known, ps.action);

		// Mirrors LogicCompare.cpp's own action-icon comparison exactly: a
		// missing ground-truth player is NOT special-cased there either — it
		// falls through with gt_action's default -1 sentinel, so the compare
		// below counts as a mismatch (v is never legitimately -1), the SAME
		// as rec.action_icon_mismatch_seats would count it.
		if(ps.action == kActionIconVocabWinner)
			row.verdict = "winner"; // not compared, matches action_icon_winner_seats
		else
			row.verdict = (ps.action == gt_action) ? "match" : "mismatch";

		const VsmLayoutCandidate* c = FindSubslotCandidate(layout_model, "action_icon", ps.seat, -1);
		if(c) {
			row.has_region  = true;
			row.region_rect = c->rect;
			row.crop_image  = CropFrameRect(frame_img, c->rect);
			row.has_crop    = !row.crop_image.IsEmpty();

			Image ref;
			if(gt_action == kActionIconVocabWinner || gt_action >= 1)
				ref = TexasHoldemGetActionIconReferenceImage(gt_action, c->rect.GetSize());
			else {
				// gt_action == 0 (or no ground truth) -> "no icon shown" positive
				// reference, using the SAME row-parity-offset helper the scorer
				// itself uses (VsmActionIconRowParityOffset, LogicCompare.h).
				int row_parity_offset = VsmActionIconRowParityOffset(ps.seat, frame_img.GetSize());
				ref = TexasHoldemGetActionIconEmptyReferenceImage(c->rect.GetSize(), row_parity_offset, false);
			}
			row.reference_image = ref;
			row.has_reference    = !ref.IsEmpty();
		}

		rows.Add(pick(row));
	}
}

static void AddHoleCardRows(const VsmTexasHoldemSession& session,
                            const VsmSessionLayoutModel& layout_model,
                            const VsmLogicCompareRecordOut& rec, int frame_id,
                            const Image& frame_img, Vector<VsmMismatchRow>& rows)
{
	const TexasHoldemGroundTruthRecord* gt = session.GroundTruthForFrame(frame_id);
	const TexasHoldemLogicState& ls = rec.logic_state;

	for(const TexasHoldemLogicPlayerState& ps : ls.players) {
		if(!ps.hole_cards_known)
			continue; // matches LogicCompare.cpp's own "if(!ps.hole_cards_known) { unknown_seats++; continue; }"

		VsmMismatchRow row;
		row.field = Format("hole_cards[seat %d]", ps.seat);

		const TexasHoldemPlayerSnapshot* gtp = gt ? FindGtPlayer(*gt, ps.seat) : nullptr;

		String expected = "[";
		String parsed   = "[";
		int seat_match = 0, seat_mismatch = 0, seat_pending = 0;
		for(int slot = 0; slot < 2 && slot < ps.hole_cards.GetCount(); slot++) {
			if(slot) { expected << ","; parsed << ","; }
			int derived_v = ps.hole_cards[slot];
			int gt_v = (gtp && slot < gtp->hole_cards.GetCount()) ? gtp->hole_cards[slot] : -1;
			expected << FormatCardValue(gt_v);
			parsed   << FormatCardValue(derived_v);
			if(derived_v < 0)
				seat_pending++;
			else if(derived_v == gt_v)
				seat_match++;
			else
				seat_mismatch++;
		}
		expected << "]";
		parsed   << "]";
		row.expected = expected;
		row.parsed   = parsed;

		// Same aggregation LogicCompare.cpp's own hole-card comparison loop
		// performs per seat (seat_mismatch>0 -> mismatch; else seat_match>0 ->
		// match; else "hidden"/pending — both slots recognized as back/unknown).
		if(seat_mismatch > 0)
			row.verdict = "mismatch";
		else if(seat_match > 0)
			row.verdict = "match";
		else
			row.verdict = "hidden";

		// Crop/reference for hole_card slot 0 (the first of the two candidate
		// sub-slots) — a genuine simplification (this row groups both slots,
		// per this task's "each seat's hole cards" wording, so only one
		// crop/reference pair is shown per row rather than compositing both
		// slots into one image); documented honestly in this task's evidence.
		const VsmLayoutCandidate* c = FindSubslotCandidate(layout_model, "hole_card", ps.seat, 0);
		if(c) {
			row.has_region  = true;
			row.region_rect = c->rect;
			row.crop_image  = CropFrameRect(frame_img, c->rect);
			row.has_crop    = !row.crop_image.IsEmpty();

			int gt_v0 = (gtp && gtp->hole_cards.GetCount() > 0) ? gtp->hole_cards[0] : -1;
			Image ref = (gt_v0 < 0)
				? TexasHoldemGetCardBackReferenceImage(c->rect.GetSize(), kCardReferenceTheme)
				: TexasHoldemGetCardReferenceImage(gt_v0, c->rect.GetSize(), kCardReferenceTheme);
			row.reference_image = ref;
			row.has_reference    = !ref.IsEmpty();
		}

		rows.Add(pick(row));
	}
}

Vector<VsmMismatchRow> VsmBuildMismatchRows(const VsmTexasHoldemSession& session,
                                            const VsmSessionLayoutModel& layout_model,
                                            const VsmSessionLogicModel& logic_model,
                                            int frame_id, const Image& frame_img)
{
	Vector<VsmMismatchRow> rows;
	const VsmLogicCompareRecordOut* rec = logic_model.RecordForFrame(frame_id);
	if(!rec)
		return rows;

	AddDealerSeatRow(session, layout_model, *rec, frame_img, rows);
	AddBoardCardRows(session, layout_model, *rec, frame_img, rows);
	AddActionIconRows(session, layout_model, *rec, frame_id, frame_img, rows);
	AddHoleCardRows(session, layout_model, *rec, frame_id, frame_img, rows);
	return rows;
}

int VsmFindNextMismatchFrame(const VsmSessionLogicModel& model, int from_frame_id, bool forward)
{
	if(!model.loaded)
		return -1;
	int step = forward ? 1 : -1;
	for(int fid = from_frame_id + step; fid >= 0 && fid < model.records.GetCount(); fid += step) {
		const VsmLogicCompareRecordOut& rec = model.records[fid];
		if(rec.verdict == "mismatch"
		   || rec.board_cards_verdict == "mismatch"
		   || rec.action_icons_verdict == "mismatch"
		   || rec.hole_cards_verdict == "mismatch")
			return fid;
	}
	return -1;
}

} // namespace Upp
