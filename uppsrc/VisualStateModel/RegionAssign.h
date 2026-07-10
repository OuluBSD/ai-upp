#ifndef _VisualStateModel_RegionAssign_h_
#define _VisualStateModel_RegionAssign_h_

// ---------------------------------------------------------------------------
// M04-06 (task 0117): shared region-to-layout-element/sub-slot overlap-
// matching logic, extracted verbatim from
// reference/VisualStateLayoutAssign/main.cpp's task-0115/0116 file-local
// `static` functions (`VsmLayoutCandidate`, `VsmScaleRect`,
// `VsmBuildCandidates`, `VsmMatchTier`, `VsmMatchRegion`) so an automated
// test package (upptst/VisualStateModelTests) can call into the exact same
// matching code the CLI uses, without duplicating it — same rationale as
// task 0116's FrameCrop.h extraction and task 0105's RegionOverlay.h
// extraction (two things that need the same logic should share one
// implementation, not risk drift between two copies).
//
// See reference/VisualStateLayoutAssign/main.cpp's header comment (or task
// 0115's evidence section, Manager repo
// VisualStateModel/0115_m04_region_element_assignment.md) for the full
// rationale behind:
//   - the overlap-scoring formula (intersection-area / REGION-area, not
//     plain IoU — chosen so a small region fully inside a much larger
//     candidate still scores 1.0, since IoU would systematically starve
//     that "small region inside a bigger owning element" case).
//   - the two-tier (sub-slot-first, element-fallback) selection strategy —
//     a flat global argmax over all candidates was tried first and found
//     (empirically, not just by inspection) to match 100% of regions to
//     top-level elements and 0% to any sub-slot, since every sub-slot's
//     rect is a geometric subset of its owning element's rect.

namespace Upp {

// One scaled-to-frame-space layout candidate a region can be matched
// against: either a top-level element or a resolved sub-slot from
// VsmLayoutProfile, flattened into one list so both compete on equal
// footing for the best overlap.
struct VsmLayoutCandidate : Moveable<VsmLayoutCandidate> {
	String label;       // "Player0" (element) or "Player0.hole_card_0" (sub-slot)
	String kind;         // "element" or "subslot"
	String role;
	int    seat_index = -1;
	int    card_index = -1;
	Rect   rect;          // scaled to actual frame pixel space

	int Area() const { return max(0, rect.Width()) * max(0, rect.Height()); }
};

// Scales a design-space rect to frame pixel space, matching
// game/Poker/TableLayoutProfile.cpp's Sx()/Sy() convention: left/top/
// right/bottom are each scaled and truncated to int SEPARATELY (not
// width/height scaled and added to an untouched origin) so results are
// consistent with the rest of this codebase's scaling behavior.
Rect VsmScaleRect(const Rect& r, double sx, double sy);

// Builds the flattened, frame-space-scaled candidate list from one
// VsmLayoutProfile.
Vector<VsmLayoutCandidate> VsmBuildCandidates(const VsmLayoutProfile& profile,
                                               double sx, double sy);

// Overlap threshold: a strict majority (>=50%) of a region's own area must
// fall inside a candidate for it to count as a match. Header-scope `static
// const` (internal linkage per translation unit) rather than a namespace-
// scope `extern` + separate definition — same simplest-option choice
// FrameCrop.h's `kCropPadding` already made for a single small constant with
// no existing header-level-constant precedent to match.
static const double kOverlapThreshold = 0.5;

struct VsmMatchResult {
	const VsmLayoutCandidate* best = NULL;
	double overlap = 0.0;
};

// Finds the highest-overlap candidate of one specificity tier (kind ==
// "subslot" or "element") that clears kOverlapThreshold; ties broken by
// preferring the smaller-area candidate (deterministic, and mildly prefers
// the more specific of two equally-good same-tier matches).
VsmMatchResult VsmMatchTier(const Rect& region_rect, double region_area,
                             const Vector<VsmLayoutCandidate>& candidates,
                             const char* tier_kind);

// Matches `region_rect` against `candidates` in two specificity tiers,
// most-specific first: sub-slots (hole cards, player name/stack/bet text,
// action icon, dealer button, timeout bar, board cards), falling back to
// top-level elements ONLY if no sub-slot clears kOverlapThreshold. Returns
// a VsmMatchResult with `best == NULL` (unassigned) if nothing in either
// tier clears the threshold.
VsmMatchResult VsmMatchRegion(const Rect& region_rect,
                               const Vector<VsmLayoutCandidate>& candidates);

} // namespace Upp

#endif
