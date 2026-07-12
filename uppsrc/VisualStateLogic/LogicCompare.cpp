#include "LogicCompare.h"

namespace Upp {

// ===========================================================================
// LEAF SCORING / DERIVATION FUNCTIONS
//
// Moved verbatim (task 0133) from reference/VisualStateLogicCompare/main.cpp's
// file-local `static` functions (tasks 0119-0128). The ONLY change from the
// CLI's own source is dropping the `static` keyword (these now have external
// linkage so both the CLI and the workbench GUI can call them). No scoring,
// threshold, or geometry logic was altered — proven byte-for-byte against the
// six kept M05 fixtures (task 0133 evidence). The per-function rationale
// comments are the CLI's own, kept intact.
// ===========================================================================

// ===========================================================================
// M07-02 (task 0138): RECOGNIZER-CONFIDENCE FORMULAS
//
// Pure additive surfacing of data the recognizers already compute. Each family
// scores on its own scale with its own accept gate; the constants these
// formulas reference (kCardMatchMinMargin/kHoleCardMatchMinMargin/
// kActionIconMatchMinMargin/kActionIconMatchMaxScore) are the project's OWN,
// already-committed definition of "how much separation is enough to trust a
// guess", so confidence is expressed RELATIVE to them rather than invented from
// scratch. The result is a family-relative 0..1 measure — deliberately NOT
// claimed to be comparable across families (their score scales differ), per the
// task's "honestly-scoped, family-relative confidence over a fabricated
// universal one" guidance.
// ===========================================================================

// Card families (board card, hole card): the accept gate is a single
// best-vs-runner-up margin >= min_margin, where the scores are mean-abs-pixel
// distances (LOWER is better). We map the margin through margin/(margin +
// min_margin): monotically increasing in the margin, 0 at no separation,
// exactly 0.5 at the family's own accept boundary (margin == min_margin), and
// asymptotically approaching 1 as the winner pulls further ahead of the field.
// (min_margin > 0 for every real family, so this never divides by zero; a
// non-positive/degenerate margin clamps to 0.)
double VsmMarginConfidence(double best, double runnerup, double min_margin)
{
	double margin = runnerup - best;
	if(margin <= 0.0 || min_margin <= 0.0)
		return 0.0;
	return margin / (margin + min_margin);
}

// Action-icon family: uniquely gates on BOTH an absolute max-score
// (best <= kActionIconMatchMaxScore) AND a min-margin
// (margin >= kActionIconMatchMinMargin). A confident recognition must clear
// both, so its confidence is the MIN of two sub-confidences — whichever gate is
// closest to failing bounds it:
//   * margin sub-confidence: margin/(margin + kActionIconMatchMinMargin), same
//     shape as the card families;
//   * absolute-quality sub-confidence: (kActionIconMatchMaxScore - best) /
//     kActionIconMatchMaxScore, i.e. how far the winning distance sits below
//     the absolute-score ceiling (1 at a perfect 0-distance match, 0 at the
//     ceiling), clamped to [0,1].
double VsmActionIconConfidence(double best, double runnerup)
{
	double margin_conf = VsmMarginConfidence(best, runnerup, kActionIconMatchMinMargin);
	double abs_conf = (kActionIconMatchMaxScore - best) / kActionIconMatchMaxScore;
	if(abs_conf < 0.0) abs_conf = 0.0;
	if(abs_conf > 1.0) abs_conf = 1.0;
	return min(margin_conf, abs_conf);
}

// Dealer/puck family: acceptance is a pure argmin over 3 mean-abs-pixel-diff
// role scores (dealer=0, sb=1, bb=2) — the dealer observation is kept iff the
// dealer role is the lowest-scoring, with NO min-margin constant to scale
// against. So confidence is the scale-free normalized contrast between the
// winning dealer score and the next-best (SB/BB) role:
// (runnerup - dealer)/(runnerup + dealer). This is inherently bounded 0..1,
// invents no threshold: 0 when dealer ties the next role (a coin-flip), rising
// toward 1 as the dealer template matches far better than either alternative.
double VsmPuckDealerConfidence(double dealer_score, double sb_score, double bb_score)
{
	double runnerup = min(sb_score, bb_score);
	double denom = runnerup + dealer_score;
	if(denom <= 0.0)
		return 0.0;
	double c = (runnerup - dealer_score) / denom;
	if(c < 0.0) c = 0.0;
	if(c > 1.0) c = 1.0;
	return c;
}

// ---------------------------------------------------------------------------
// M05-03 (task 0121): template-match disambiguation for dealer_button-role
// observations.
//
// Mean absolute per-pixel RGB difference between two same-size images.
// Returns DBL_MAX if the sizes mismatch or either image is empty (so it can
// never spuriously "win" a role comparison).
double VsmMeanAbsPixelDiff(const Image& a, const Image& b)
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
bool VsmScorePuckRoles(const Image& frame_img, const Rect& candidate_rect,
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
// entirely. Searches EACH of the 3 puck references' own best (lowest-score)
// position INDEPENDENTLY across the whole scale/offset grid (see the CLI's
// header comment for why a single global argmin was rejected). The caller is
// responsible for the trigger/proximity check — this function always searches.
void VsmSearchPuckRecovery(const Image& frame_img, const Rect& candidate_rect,
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
// Core dealer-seat derivation (task 0119). Applies every dealer_button-role
// observation (in the order given) to a "seat N is dealer from this frame
// onward" running VectorMap<frame, seat>, taking the LAST observation for a
// given `frame` if more than one seat's button_puck sub-slot changed in the
// same transition.
void ApplyDealerButtonObservations(const Vector<VsmLayoutObservationOut>& observations,
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
// pair per frame, "sticky" from the frame a seat is first observed onward.
void DeriveDealerSeatPerFrame(const VectorMap<int, int>& dealer_seat_by_frame,
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

// M07-02 (task 0138): confidence siblings of the two functions above. Purely
// additive — the value derivation (ApplyDealerButtonObservations /
// DeriveDealerSeatPerFrame) is untouched.
//
// ApplyDealerButtonConfidence mirrors ApplyDealerButtonObservations exactly:
// iterate the SAME pre-filtered dealer-move observations in the SAME order and
// take the LAST one per frame (GetAdd(...) = ...), but store the dealer-role
// confidence instead of the seat, so the frame keys stay in lockstep with
// dealer_seat_by_frame.
void ApplyDealerButtonConfidence(const Vector<VsmLayoutObservationOut>& observations,
                                 VectorMap<int, double>& dealer_confidence_by_frame)
{
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "dealer_button")
			continue;
		double c = o.puck_scored
			? VsmPuckDealerConfidence(o.puck_score_dealer, o.puck_score_sb, o.puck_score_bb)
			: 0.0;
		dealer_confidence_by_frame.GetAdd(o.frame, c) = c;
	}
}

// DeriveDealerSeatConfidencePerFrame mirrors DeriveDealerSeatPerFrame's sticky
// latch exactly, over the confidence map instead of the seat map.
void DeriveDealerSeatConfidencePerFrame(const VectorMap<int, double>& dealer_confidence_by_frame,
                                        int frame_lo, int frame_hi,
                                        Vector<double>& confidence_out)
{
	confidence_out.Clear();
	double conf = 0.0;
	for(int fid = frame_lo; fid <= frame_hi; fid++) {
		int i = dealer_confidence_by_frame.Find(fid);
		if(i >= 0)
			conf = dealer_confidence_by_frame[i];
		confidence_out.Add(conf);
	}
}

// ---------------------------------------------------------------------------
// M05-08 (task 0126): board (community) card template-match recognition.
//
// The board_card_N candidate rect from VsmBuildCandidates does NOT match the
// REAL on-screen board_card rect GameTable::PaintBoard computes at record
// time; this returns the empirically-measured corrected rect (baseline frame
// size 1024x625, scaled proportionally for other decoded sizes). See the CLI's
// own header comment for the full diagnosis.
Rect VsmEmpiricalBoardCardRect(int card_index, Size frame_size)
{
	// Baseline measured at frame_size (1024,625).
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

// Crops `candidate_rect` out of `frame_img`, scores it against all 53
// references (52 real cards letterbox-composited the way PaintBoard renders
// them, plus the flat-color holder scored unsearched), returns the argmin.
// See the CLI's own doc comment for FIX 1 (letterbox compositing) and FIX 2
// (VsmEmpiricalBoardCardRect geometry correction).
bool VsmScoreCardSlot(const Image& frame_img, const Rect& candidate_rect, int card_index,
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
	// DrawFitted lambda performs: fit the card art into (w,h) preserving
	// aspect, center it over the same Color(0,80,0) felt fallback the holder
	// reference itself falls back to.
	auto ComposeFitted = [&](const Image& native, int w, int h) -> Image {
		Image fitted = FitCardArt(native, Size(w, h));
		ImageDraw canvas(w, h);
		canvas.Alpha().DrawRect(0, 0, w, h, White());
		canvas.DrawRect(0, 0, w, h, Color(0, 80, 0));
		if(!fitted.IsEmpty())
			canvas.DrawImage((w - fitted.GetWidth()) / 2, (h - fitted.GetHeight()) / 2, fitted);
		return canvas;
	};

	// Best-of-(scale,offset) score for one real card's NATIVE reference art.
	// Deliberately NOT applied to the holder reference (see the CLI comment):
	// a flat-color reference scored over a wide search is near-vacuous.
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
	// Holder: scored ONLY at the corrected rect, unsearched.
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

// Per-slot sticky-since-last-observation state: 5 INDEPENDENT tracks (one per
// board_card_N sub-slot), each holding a VALUE (-1 or 0..51). Always takes the
// LATEST observation for a given (slot, frame).
void DeriveBoardCardsPerFrame(const Vector<VsmBoardCardObservation>& observations,
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

// M07-02 (task 0138): board-card confidence sibling. Same 5 independent sticky
// tracks and same latest-observation-per-(slot,frame) rule as
// DeriveBoardCardsPerFrame, latching confidence instead of value. Value
// derivation above is untouched.
void DeriveBoardCardsConfidencePerFrame(const Vector<VsmBoardCardObservation>& observations,
                                        int frame_lo, int frame_hi,
                                        Vector<double> confidence_out[5])
{
	for(int slot = 0; slot < 5; slot++) {
		VectorMap<int, double> conf_by_frame;
		for(const VsmBoardCardObservation& o : observations)
			if(o.card_index == slot)
				conf_by_frame.GetAdd(o.frame, o.confidence) = o.confidence;

		Vector<double>& conf = confidence_out[slot];
		conf.Clear();
		double c = 0.0;
		for(int fid = frame_lo; fid <= frame_hi; fid++) {
			int i = conf_by_frame.Find(fid);
			if(i >= 0)
				c = conf_by_frame[i];
			conf.Add(c);
		}
	}
}

// ---------------------------------------------------------------------------
// M05-09 (task 0127): per-player action-icon template-match recognition.
//
// Empirically-measured per-seat stripe-PHASE correction (baseline 1024x625).
// See the CLI's own doc comment for why this can't be derived from the
// candidate list alone (two independently-rounded rects can flip parity).
int VsmActionIconRowParityOffset(int seat, Size frame_size)
{
	static const int kBaselineOffsets[6] = { 131, 102, 162, 146, 162, 102 };
	if(seat < 0 || seat >= 6)
		return 0;
	const double base_frame_h = 625.0;
	double sy = frame_size.cy > 0 ? frame_size.cy / base_frame_h : 1.0;
	return (int)(kBaselineOffsets[seat] * sy + 0.5);
}

// Crops `candidate_rect` out of `frame_img`, scores it against the 6 real-
// action + 1 winner + 1 empty-background references (8-way), returns the
// argmin's VALUE (not array index) in `winner_out`.
bool VsmScoreActionIcon(const Image& frame_img, const Rect& candidate_rect, int seat,
                        double scores_out[kActionIconRealVocabCount + 1], int& winner_out)
{
	int w = candidate_rect.GetWidth(), h = candidate_rect.GetHeight();
	if(w <= 0 || h <= 0)
		return false;
	Rect frame_rect(0, 0, frame_img.GetWidth(), frame_img.GetHeight());
	if(!frame_rect.Contains(candidate_rect))
		return false;

	Image candidate = Crop(frame_img, candidate_rect);
	for(int i = 0; i < kActionIconRealVocabCount; i++) {
		Image ref = TexasHoldemGetActionIconReferenceImage(kActionIconRealVocab[i], Size(w, h));
		scores_out[i] = VsmMeanAbsPixelDiff(candidate, ref);
	}
	int row_parity_offset = VsmActionIconRowParityOffset(seat, frame_img.GetSize());
	Image empty_ref = TexasHoldemGetActionIconEmptyReferenceImage(Size(w, h), row_parity_offset, false);
	scores_out[kActionIconRealVocabCount] = VsmMeanAbsPixelDiff(candidate, empty_ref);

	int best = 0;
	for(int i = 1; i <= kActionIconRealVocabCount; i++)
		if(scores_out[i] < scores_out[best])
			best = i;
	winner_out = (best == kActionIconRealVocabCount) ? kActionIconEmptyWinnerValue : kActionIconRealVocab[best];
	return true;
}

// Per-seat sticky-since-last-CONFIDENT-observation state. An empty/background
// observation (kActionIconEmptyWinnerValue) resets `known` back to false
// rather than asserting a specific value.
void DeriveActionIconsPerFrame(const Vector<VsmActionIconObservation>& observations,
                               int frame_lo, int frame_hi,
                               const Vector<int>& seats,
                               VectorMap<int, Vector<bool>>& known_out,
                               VectorMap<int, Vector<int>>& value_out)
{
	for(int seat : seats) {
		VectorMap<int, int> value_by_frame;
		for(const VsmActionIconObservation& o : observations)
			if(o.seat == seat)
				value_by_frame.GetAdd(o.frame, o.value) = o.value;

		Vector<bool> known;
		Vector<int>  value;
		bool k = false;
		int  v = -1;
		for(int fid = frame_lo; fid <= frame_hi; fid++) {
			int i = value_by_frame.Find(fid);
			if(i >= 0) {
				int ov = value_by_frame[i];
				if(ov == kActionIconEmptyWinnerValue) {
					k = false;
					v = -1;
				}
				else {
					k = true;
					v = ov;
				}
			}
			known.Add(k);
			value.Add(v);
		}
		known_out.Add(seat, pick(known));
		value_out.Add(seat, pick(value));
	}
}

// M07-02 (task 0138): action-icon confidence sibling. Mirrors
// DeriveActionIconsPerFrame's sticky-since-last-CONFIDENT-observation rule
// EXACTLY, including the reset: an empty/background observation
// (kActionIconEmptyWinnerValue) drops confidence back to 0.0 the same frame it
// drops `known` back to false, so this stays in lockstep with the value latch.
// Needs both value and confidence per frame to reproduce that branch; value
// derivation above is untouched.
void DeriveActionIconsConfidencePerFrame(const Vector<VsmActionIconObservation>& observations,
                                         int frame_lo, int frame_hi,
                                         const Vector<int>& seats,
                                         VectorMap<int, Vector<double>>& confidence_out)
{
	for(int seat : seats) {
		VectorMap<int, int>    value_by_frame;
		VectorMap<int, double> conf_by_frame;
		for(const VsmActionIconObservation& o : observations)
			if(o.seat == seat) {
				value_by_frame.GetAdd(o.frame, o.value) = o.value;
				conf_by_frame.GetAdd(o.frame, o.confidence) = o.confidence;
			}

		Vector<double> conf;
		double c = 0.0;
		for(int fid = frame_lo; fid <= frame_hi; fid++) {
			int i = value_by_frame.Find(fid);
			if(i >= 0) {
				int ov = value_by_frame[i];
				if(ov == kActionIconEmptyWinnerValue)
					c = 0.0;
				else
					c = conf_by_frame[i];
			}
			conf.Add(c);
		}
		confidence_out.Add(seat, pick(conf));
	}
}

// ---------------------------------------------------------------------------
// M05-10 (task 0128): per-player, per-slot hole-card template-match
// recognition.
//
// Real, measured PlayerCtrl top (baseline 1024x625), combined with the
// CANDIDATE rect's own `.top` at call time (self-correcting for +/-1px rect
// drift). See the CLI's own doc comment.
int VsmHoleCardRowParityOffset(const Rect& candidate_rect, int seat, Size frame_size)
{
	static const int kBaselinePlayerTop[6] = { 351, 260, 39, 10, 39, 260 };
	if(seat < 0 || seat >= 6)
		return 0;
	const double base_frame_h = 625.0;
	double sy = frame_size.cy > 0 ? frame_size.cy / base_frame_h : 1.0;
	int real_player_top = (int)(kBaselinePlayerTop[seat] * sy + 0.5);
	return candidate_rect.top - real_player_top;
}

// Composes a hole-card reference the way GameTable::RenderToImage's DrawChild
// lambda + ScaledImageCtrl FIT-mode scaling renders one: `native` aspect-fit
// into (w,h), centered over the deterministic striped PlayerCtrl background.
Image VsmComposeHoleCardFitted(const Image& native, int w, int h, int row_parity_offset, bool is_winner)
{
	Image bg = TexasHoldemGetActionIconEmptyReferenceImage(Size(w, h), row_parity_offset, is_winner);
	Image fitted = FitCardArt(native, Size(w, h));
	if(fitted.IsEmpty())
		return bg;
	ImageDraw iw(w, h);
	iw.Alpha().DrawRect(0, 0, w, h, White());
	iw.DrawImage(0, 0, bg);
	iw.DrawImage((w - fitted.GetWidth()) / 2, (h - fitted.GetHeight()) / 2, fitted);
	return iw;
}

// Crops `candidate_rect` out of `frame_img`, scores it against the 53-way
// vocabulary (52 real cards + card-back), returns the argmin. Uses the
// candidate rect AS-IS (no per-seat rect override, unlike board cards — the
// +/-1px drift doesn't defeat a mean-abs-diff comparison).
bool VsmScoreHoleCardSlot(const Image& frame_img, const Rect& candidate_rect, int seat,
                          double scores_out[kHoleCardVocabSize], int& winner_out)
{
	int w = candidate_rect.GetWidth(), h = candidate_rect.GetHeight();
	if(w <= 0 || h <= 0)
		return false;
	Rect frame_rect(0, 0, frame_img.GetWidth(), frame_img.GetHeight());
	if(!frame_rect.Contains(candidate_rect))
		return false;

	Image candidate_img = Crop(frame_img, candidate_rect);
	int row_parity_offset = VsmHoleCardRowParityOffset(candidate_rect, seat, frame_img.GetSize());

	auto ScoreAgainstNative = [&](const Image& native) -> double {
		double best = DBL_MAX;
		for(int wi = 0; wi < 2; wi++) {
			Image ref = VsmComposeHoleCardFitted(native, w, h, row_parity_offset, wi == 1);
			double d = VsmMeanAbsPixelDiff(candidate_img, ref);
			if(d < best)
				best = d;
		}
		return best;
	};

	for(int card = 0; card < 52; card++) {
		Image native = TexasHoldemGetCardReferenceImage(card, kCardNativeSize, kCardReferenceTheme);
		scores_out[card] = ScoreAgainstNative(native);
	}
	{
		Image back_native = TexasHoldemGetCardBackReferenceImage(kCardNativeSize, kCardReferenceTheme);
		scores_out[kHoleCardBackVocabIndex] = ScoreAgainstNative(back_native);
	}

	winner_out = 0;
	for(int i = 1; i < kHoleCardVocabSize; i++)
		if(scores_out[i] < scores_out[winner_out])
			winner_out = i;
	return true;
}

// Per-(seat, card_index) sticky-since-last-observation state. Recognizing
// "back" as the winner in a LATER frame is the reset signal.
void DeriveHoleCardsPerFrame(const Vector<VsmHoleCardObservation>& observations,
                             int frame_lo, int frame_hi,
                             const Vector<int>& seats,
                             VectorMap<int, Vector<bool>> known_out[2],
                             VectorMap<int, Vector<int>> value_out[2])
{
	for(int slot = 0; slot < 2; slot++) {
		for(int seat : seats) {
			VectorMap<int, int> value_by_frame;
			for(const VsmHoleCardObservation& o : observations)
				if(o.seat == seat && o.card_index == slot)
					value_by_frame.GetAdd(o.frame, o.value) = o.value;

			Vector<bool> known;
			Vector<int>  value;
			bool k = false;
			int  v = -1;
			for(int fid = frame_lo; fid <= frame_hi; fid++) {
				int i = value_by_frame.Find(fid);
				if(i >= 0) { k = true; v = value_by_frame[i]; }
				known.Add(k);
				value.Add(v);
			}
			known_out[slot].Add(seat, pick(known));
			value_out[slot].Add(seat, pick(value));
		}
	}
}

// M07-02 (task 0138): hole-card confidence sibling. Same per-(seat,slot) sticky
// tracks and same latest-observation-per-frame rule as DeriveHoleCardsPerFrame,
// latching confidence instead of value. Value derivation above is untouched.
void DeriveHoleCardsConfidencePerFrame(const Vector<VsmHoleCardObservation>& observations,
                                       int frame_lo, int frame_hi,
                                       const Vector<int>& seats,
                                       VectorMap<int, Vector<double>> confidence_out[2])
{
	for(int slot = 0; slot < 2; slot++) {
		for(int seat : seats) {
			VectorMap<int, double> conf_by_frame;
			for(const VsmHoleCardObservation& o : observations)
				if(o.seat == seat && o.card_index == slot)
					conf_by_frame.GetAdd(o.frame, o.confidence) = o.confidence;

			Vector<double> conf;
			double c = 0.0;
			for(int fid = frame_lo; fid <= frame_hi; fid++) {
				int i = conf_by_frame.Find(fid);
				if(i >= 0)
					c = conf_by_frame[i];
				conf.Add(c);
			}
			confidence_out[slot].Add(seat, pick(conf));
		}
	}
}

// ===========================================================================
// TOP-LEVEL SILENT DRIVER (task 0133)
//
// A faithful, stdout-free reproduction of reference/VisualStateLogicCompare's
// GUI_APP_MAIN derivation pipeline (main.cpp), composing the leaf functions
// above in the EXACT same order (frame-0 seed passes, per-transition
// detection + role scoring + probe passes, disambiguation filter, acceptance
// gates, per-slot/per-seat sticky derivation, and the four comparison passes
// that populate each record's logic_state). The CLI keeps its own copy of this
// orchestration (with its diagnostic printing) untouched, guaranteeing the
// CLI's byte-for-byte non-regression; this driver is proven to produce the
// identical Vector<VsmLogicCompareRecordOut> via the six-fixture cross-check in
// task 0133's evidence section (driver --jsonl-out == CLI --jsonl-out).
// ===========================================================================
bool VsmDeriveSessionLogicStates(const String& session_dir, const String& form_path,
                                 Vector<VsmLogicCompareRecordOut>& out_records,
                                 String& error)
{
	out_records.Clear();
	error.Clear();

	if(!DirectoryExists(session_dir)) {
		error = Format("Session directory not found: %s", session_dir);
		return false;
	}

	// --- Load ground truth (TexasHoldemGroundTruthRecord::Jsonize). ---
	String gt_path = AppendFileName(session_dir, "groundtruth.jsonl");
	if(!FileExists(gt_path)) {
		error = Format("Missing groundtruth.jsonl under: %s", session_dir);
		return false;
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
				error = Format("Failed to parse groundtruth.jsonl row %d", gt_records.GetCount());
				return false;
			}
			gt_records.Add(pick(rec));
		}
	}
	if(gt_records.IsEmpty()) {
		error = "groundtruth.jsonl has no rows";
		return false;
	}

	VsmM01M02SessionInfo info;
	if(!VsmReadM01M02SessionInfo(session_dir, info)) {
		error = Format("Failed to read M01/M02 session metadata.json under: %s", session_dir);
		return false;
	}
	if(info.frame_count < 2) {
		error = "Session has fewer than 2 frames - no transitions to detect";
		return false;
	}

	// --- Load the .form layout profile. ---
	Vector<VsmFormLayout> layouts = VsmParseFormFile(form_path);
	if(layouts.IsEmpty()) {
		error = Format("Failed to parse any <layouts><item> from: %s", form_path);
		return false;
	}
	const VsmFormLayout& layout = layouts[0];
	VsmLayoutProfile profile = VsmBuildLayoutProfile(layout);

	VsmFrameImage probe_frame;
	if(!VsmLoadM01M02SessionFrame(session_dir, 0, probe_frame)) {
		error = "Failed to decode frame 0";
		return false;
	}
	if(profile.width <= 0 || profile.height <= 0) {
		error = "Layout profile has zero/negative width or height - cannot compute scale";
		return false;
	}
	double sx = (double)probe_frame.width  / profile.width;
	double sy = (double)probe_frame.height / profile.height;

	Vector<VsmLayoutCandidate> candidates = VsmBuildCandidates(profile, sx, sy);

	Vector<const VsmLayoutCandidate*> puck_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "dealer_button")
			puck_candidates.Add(&c);

	Vector<const VsmLayoutCandidate*> board_card_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "board_card")
			board_card_candidates.Add(&c);

	Vector<VsmBoardCardObservation> board_card_observations;

	Vector<const VsmLayoutCandidate*> action_icon_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "action_icon")
			action_icon_candidates.Add(&c);

	Vector<const VsmLayoutCandidate*> hole_card_candidates;
	for(const VsmLayoutCandidate& c : candidates)
		if(c.kind == "subslot" && c.role == "hole_card")
			hole_card_candidates.Add(&c);

	Vector<VsmActionIconObservation> action_icon_observations;
	Vector<VsmHoleCardObservation> hole_card_observations;

	// M05-08 (task 0126): frame-0 initial seed pass (board cards).
	if(!board_card_candidates.IsEmpty()) {
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
			if(accept) {
				VsmBoardCardObservation bco;
				bco.frame = 0;
				bco.card_index = c->card_index;
				bco.value = (winner == kCardHolderVocabIndex) ? -1 : winner;
				bco.confidence = VsmMarginConfidence(scores[winner], runnerup, kCardMatchMinMargin);
				board_card_observations.Add(bco);
			}
		}
	}

	// M05-10 (task 0128): frame-0 initial seed pass (hole cards).
	if(!hole_card_candidates.IsEmpty()) {
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
			if(accept) {
				VsmHoleCardObservation hco;
				hco.frame = 0;
				hco.seat = c->seat_index;
				hco.card_index = c->card_index;
				hco.value = (winner == kHoleCardBackVocabIndex) ? -1 : winner;
				hco.confidence = VsmMarginConfidence(scores[winner], runnerup, kHoleCardMatchMinMargin);
				hole_card_observations.Add(hco);
			}
		}
	}

	// --- Region detection across every transition. ---
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

	Vector<VsmLayoutObservationOut> recovered_observations;
	int rescue_counter = 0;

	VsmFrameImage prev_frame;
	prev_frame.Set(probe_frame.width, probe_frame.height, nullptr);
	memcpy(prev_frame.data, probe_frame.data, (size_t)probe_frame.width * probe_frame.height * 4);

	for(int fid = 1; fid < info.frame_count; fid++) {
		VsmFrameImage curr_frame;
		if(!VsmLoadM01M02SessionFrame(session_dir, fid, curr_frame)) {
			error = Format("Failed to decode frame %d", fid);
			return false;
		}

		Vector<VsmChangedRect> changes = VsmDetectChanges(prev_frame, curr_frame, params);
		Image curr_frame_img;
		bool curr_frame_img_ready = false;
		for(const VsmChangedRect& cr : changes) {
			VsmFingerprint32 fp;
			if(!VsmRegionMemory::ExtractFingerprint(curr_frame, cr.x, cr.y, cr.w, cr.h, fp)) {
				error = Format("ExtractFingerprint frame %d", fid);
				return false;
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

		// M05-04 (task 0122): additive puck recovery pass for THIS transition.
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
				obs.overlap     = 1.0;

				obs.puck_scored       = true;
				obs.puck_score_dealer = attempt.score_dealer;
				obs.puck_score_sb     = attempt.score_sb;
				obs.puck_score_bb     = attempt.score_bb;
				obs.puck_role_winner  = 0;

				recovered_observations.Add(obs);
			}
		}

		// M05-08 (task 0126): change-triggered board_card_N direct probe.
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

			double scores[kCardVocabSize];
			int winner = -1;
			if(VsmScoreCardSlot(curr_frame_img, c->rect, c->card_index, scores, winner)) {
				double runnerup = DBL_MAX;
				for(int i = 0; i < kCardVocabSize; i++)
					if(i != winner && scores[i] < runnerup)
						runnerup = scores[i];
				bool accepted = (runnerup - scores[winner]) >= kCardMatchMinMargin;
				if(accepted) {
					VsmBoardCardObservation bco;
					bco.frame      = fid;
					bco.card_index = c->card_index;
					bco.value      = (winner == kCardHolderVocabIndex) ? -1 : winner;
					bco.confidence = VsmMarginConfidence(scores[winner], runnerup, kCardMatchMinMargin);
					board_card_observations.Add(bco);
				}
			}
		}

		// M05-09 (task 0127): change-triggered action_icon direct probe.
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
				bool accepted = (scores[winner_index] <= kActionIconMatchMaxScore)
				                && (runnerup - scores[winner_index] >= kActionIconMatchMinMargin);
				if(accepted) {
					VsmActionIconObservation aio;
					aio.frame = fid;
					aio.seat  = c->seat_index;
					aio.value = winner;
					aio.confidence = VsmActionIconConfidence(scores[winner_index], runnerup);
					action_icon_observations.Add(aio);
				}
			}
		}

		// M05-10 (task 0128): change-triggered hole_card direct probe.
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

			double scores[kHoleCardVocabSize];
			int winner = -1;
			if(VsmScoreHoleCardSlot(curr_frame_img, c->rect, c->seat_index, scores, winner)) {
				double runnerup = DBL_MAX;
				for(int i = 0; i < kHoleCardVocabSize; i++)
					if(i != winner && scores[i] < runnerup)
						runnerup = scores[i];
				bool accepted = (runnerup - scores[winner]) >= kHoleCardMatchMinMargin;
				if(accepted) {
					VsmHoleCardObservation hco;
					hco.frame      = fid;
					hco.seat       = c->seat_index;
					hco.card_index = c->card_index;
					hco.value      = (winner == kHoleCardBackVocabIndex) ? -1 : winner;
					hco.confidence = VsmMarginConfidence(scores[winner], runnerup, kHoleCardMatchMinMargin);
					hole_card_observations.Add(hco);
				}
			}
		}

		if(prev_frame.width != curr_frame.width || prev_frame.height != curr_frame.height)
			prev_frame.Set(curr_frame.width, curr_frame.height, nullptr);
		memcpy(prev_frame.data, curr_frame.data, (size_t)curr_frame.width * curr_frame.height * 4);
	}

	// --- M05-03 (task 0121): dealer_button disambiguation filter. ---
	Vector<VsmLayoutObservationOut> dealer_move_observations;
	for(const VsmLayoutObservationOut& o : observations) {
		if(o.role != "dealer_button") {
			dealer_move_observations.Add(o);
			continue;
		}
		bool keep = o.puck_scored && o.puck_role_winner == 0;
		if(keep)
			dealer_move_observations.Add(o);
	}
	// M05-04 (task 0122): merge recovered observations.
	for(const VsmLayoutObservationOut& o : recovered_observations)
		dealer_move_observations.Add(o);

	// M05-08 (task 0126): board-card acceptance gate.
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
			bco.confidence = VsmMarginConfidence(o.card_score_best, o.card_score_runnerup, kCardMatchMinMargin);
			board_card_observations.Add(bco);
		}
	}

	// M05-09 (task 0127): action-icon acceptance gate.
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
			aio.confidence = VsmActionIconConfidence(o.action_icon_score_best, o.action_icon_score_runnerup);
			action_icon_observations.Add(aio);
		}
	}

	// M05-10 (task 0128): hole-card acceptance gate.
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
			hco.confidence = VsmMarginConfidence(o.hole_card_score_best, o.hole_card_score_runnerup, kHoleCardMatchMinMargin);
			hole_card_observations.Add(hco);
		}
	}

	// --- Per-frame board-card slot values. ---
	Vector<bool> board_known[5];
	Vector<int>  board_value[5];
	DeriveBoardCardsPerFrame(board_card_observations, 0, info.frame_count - 1, board_known, board_value);

	// --- Per-frame per-seat action-icon values. ---
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

	// --- Per-frame per-(seat,slot) hole-card values. ---
	VectorMap<int, Vector<bool>> hole_card_known[2];
	VectorMap<int, Vector<int>>  hole_card_value[2];
	DeriveHoleCardsPerFrame(hole_card_observations, 0, info.frame_count - 1, action_icon_seats,
	                        hole_card_known, hole_card_value);

	// --- Per-frame dealer seat. ---
	VectorMap<int, int> dealer_seat_by_frame;
	ApplyDealerButtonObservations(dealer_move_observations, dealer_seat_by_frame);

	Vector<bool> derived_known;
	Vector<int>  derived_seat;
	DeriveDealerSeatPerFrame(dealer_seat_by_frame, 0, info.frame_count - 1, derived_known, derived_seat);

	// --- M07-02 (task 0138): per-frame recognizer confidence, derived in
	// parallel to (and never altering) the value derivation above, using the
	// same observation lists and the same sticky/reset latch rules. ---
	Vector<double> board_confidence[5];
	DeriveBoardCardsConfidencePerFrame(board_card_observations, 0, info.frame_count - 1, board_confidence);

	VectorMap<int, Vector<double>> action_icon_confidence;
	DeriveActionIconsConfidencePerFrame(action_icon_observations, 0, info.frame_count - 1,
	                                    action_icon_seats, action_icon_confidence);

	VectorMap<int, Vector<double>> hole_card_confidence[2];
	DeriveHoleCardsConfidencePerFrame(hole_card_observations, 0, info.frame_count - 1,
	                                  action_icon_seats, hole_card_confidence);

	VectorMap<int, double> dealer_confidence_by_frame;
	ApplyDealerButtonConfidence(dealer_move_observations, dealer_confidence_by_frame);
	Vector<double> derived_dealer_confidence;
	DeriveDealerSeatConfidencePerFrame(dealer_confidence_by_frame, 0, info.frame_count - 1,
	                                   derived_dealer_confidence);

	// --- Build comparator records (dealer-seat pass). ---
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

		if(!rec.derived_dealer_seat_known)
			rec.verdict = "unknown";
		else if(gt_dealer_known && rec.derived_dealer_seat == gt_dealer_seat)
			rec.verdict = "match";
		else
			rec.verdict = "mismatch";

		TexasHoldemLogicState& ls = rec.logic_state;
		ls.frame_id = fid;
		ls.dealer_seat_known = rec.derived_dealer_seat_known;
		ls.dealer_seat       = rec.derived_dealer_seat;
		for(const TexasHoldemPlayerSnapshot& p : gt.players) {
			TexasHoldemLogicPlayerState ps;
			ps.seat = p.seat;
			if(rec.derived_dealer_seat_known && p.seat == rec.derived_dealer_seat) {
				ps.button_known = true;
				ps.button = 1;
				ls.players_known = true;
			}
			ls.players.Add(pick(ps));
		}

		out_records.Add(pick(rec));
	}

	// --- M05-08 (task 0126): board-card frame-by-frame comparison. ---
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
		if(!all_known)
			rec.board_cards_verdict = "unknown";
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
			if(rec.board_cards_mismatch_slots > 0)
				rec.board_cards_verdict = "mismatch";
			else if(rec.board_cards_match_slots > 0)
				rec.board_cards_verdict = "match";
			else
				rec.board_cards_verdict = "pending";
		}
	}

	// --- M05-09 (task 0127): action-icon frame-by-frame comparison. ---
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
				continue;

			if(v == kActionIconVocabWinner) {
				ps.action_known = true;
				ps.action = v;
				ls.players_known = true;
				rec.action_icon_winner_seats++;
				continue;
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
		}

		rec.action_icon_unscored_seats = ls.players.GetCount()
			- rec.action_icon_match_seats - rec.action_icon_mismatch_seats - rec.action_icon_winner_seats;

		if(rec.action_icon_mismatch_seats > 0)
			rec.action_icons_verdict = "mismatch";
		else if(rec.action_icon_match_seats > 0)
			rec.action_icons_verdict = "match";
		else
			rec.action_icons_verdict = "unscored";
	}

	// --- M05-10 (task 0128): hole-card frame-by-frame comparison. ---
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
					continue;
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
		}

		if(rec.hole_cards_mismatch_seats > 0)
			rec.hole_cards_verdict = "mismatch";
		else if(rec.hole_cards_match_seats > 0)
			rec.hole_cards_verdict = "match";
		else
			rec.hole_cards_verdict = "unscored";
	}

	// --- M07-02 (task 0138): per-field confidence pass. Purely additive; reads
	// the same per-frame `known` flags the existing passes already use so each
	// entry's `known`/`confidence` stay in lockstep with the derived value, and
	// reports the confidence of the observation that established that value. One
	// entry per field that carries a per-frame value: dealer_seat, board_card_0
	// ..4, action_icon[seat N] (per player), hole_cards[seat N] (per player). ---
	for(int fid = 0; fid < out_records.GetCount(); fid++) {
		VsmLogicCompareRecordOut& rec = out_records[fid];
		Vector<VsmFieldConfidence>& fc = rec.field_confidence;

		// dealer_seat
		{
			VsmFieldConfidence e;
			e.field = "dealer_seat";
			e.known = rec.derived_dealer_seat_known;
			e.confidence = e.known ? derived_dealer_confidence[fid] : 0.0;
			fc.Add(pick(e));
		}

		// board_card_0 .. board_card_4
		for(int slot = 0; slot < 5; slot++) {
			bool k = fid < board_known[slot].GetCount() && board_known[slot][fid];
			VsmFieldConfidence e;
			e.field = Format("board_card_%d", slot);
			e.known = k;
			e.confidence = (k && fid < board_confidence[slot].GetCount()) ? board_confidence[slot][fid] : 0.0;
			fc.Add(pick(e));
		}

		// action_icon[seat N], per player (same seat list / known rule as the
		// action-icon comparison pass above).
		for(const TexasHoldemLogicPlayerState& ps : rec.logic_state.players) {
			int ki = action_icon_known.Find(ps.seat);
			bool k = (ki >= 0) && fid < action_icon_known[ki].GetCount() && action_icon_known[ki][fid];
			int ci = action_icon_confidence.Find(ps.seat);
			VsmFieldConfidence e;
			e.field = Format("action_icon[seat %d]", ps.seat);
			e.known = k;
			e.confidence = (k && ci >= 0 && fid < action_icon_confidence[ci].GetCount())
				? action_icon_confidence[ci][fid] : 0.0;
			fc.Add(pick(e));
		}

		// hole_cards[seat N], per player. `known` == both slots known (same rule
		// as the hole-card comparison pass); confidence is the MIN of the two
		// slots' confidences — the weaker card bounds how sure the pair is.
		for(const TexasHoldemLogicPlayerState& ps : rec.logic_state.players) {
			int ki0 = hole_card_known[0].Find(ps.seat);
			int ki1 = hole_card_known[1].Find(ps.seat);
			bool k0 = (ki0 >= 0) && fid < hole_card_known[0][ki0].GetCount() && hole_card_known[0][ki0][fid];
			bool k1 = (ki1 >= 0) && fid < hole_card_known[1][ki1].GetCount() && hole_card_known[1][ki1][fid];
			bool k = k0 && k1;
			VsmFieldConfidence e;
			e.field = Format("hole_cards[seat %d]", ps.seat);
			e.known = k;
			if(k) {
				int ci0 = hole_card_confidence[0].Find(ps.seat);
				int ci1 = hole_card_confidence[1].Find(ps.seat);
				double c0 = (ci0 >= 0 && fid < hole_card_confidence[0][ci0].GetCount()) ? hole_card_confidence[0][ci0][fid] : 0.0;
				double c1 = (ci1 >= 0 && fid < hole_card_confidence[1][ci1].GetCount()) ? hole_card_confidence[1][ci1][fid] : 0.0;
				e.confidence = min(c0, c1);
			}
			fc.Add(pick(e));
		}
	}

	return true;
}

} // namespace Upp
