#include "VisualStateModel.h"

namespace Upp {

Rect VsmScaleRect(const Rect& r, double sx, double sy)
{
	return Rect((int)(r.left * sx), (int)(r.top * sy),
	            (int)(r.right * sx), (int)(r.bottom * sy));
}

Vector<VsmLayoutCandidate> VsmBuildCandidates(const VsmLayoutProfile& profile,
                                               double sx, double sy)
{
	Vector<VsmLayoutCandidate> out;
	for(const VsmLayoutElementInfo& e : profile.elements) {
		VsmLayoutCandidate c;
		c.label = e.name;
		c.kind = "element";
		c.role = e.role;
		c.seat_index = e.seat_index;
		c.rect = VsmScaleRect(e.GetRect(), sx, sy);
		out.Add(c);
	}
	for(const VsmLayoutSubSlotInfo& s : profile.subslots) {
		VsmLayoutCandidate c;
		c.label = s.owner_name + "." + s.slot_name;
		c.kind = "subslot";
		c.role = s.role;
		c.seat_index = s.seat_index;
		c.card_index = s.card_index;
		c.rect = VsmScaleRect(s.GetRect(), sx, sy);
		out.Add(c);
	}
	return out;
}

VsmMatchResult VsmMatchTier(const Rect& region_rect, double region_area,
                             const Vector<VsmLayoutCandidate>& candidates,
                             const char* tier_kind)
{
	VsmMatchResult result;
	bool result_contains = false; // does result.best fully contain region_rect?
	for(const VsmLayoutCandidate& c : candidates) {
		if(c.kind != tier_kind)
			continue;
		Rect inter = region_rect & c.rect;
		if(inter.IsEmpty())
			continue;
		// Containment fraction: intersection area over the SMALLER of the two
		// rects' own areas (region vs. candidate), not always region_area. See
		// RegionAssign.h for the full derivation; in short this is
		// intersection / min(region_area, candidate_area), which equals 1.0
		// exactly when EITHER rect fully contains the other, in EITHER
		// direction — fixing the region-merge dilution case (a small candidate
		// fully inside a big, indiscriminately-merged region) that the old
		// intersection/region_area formula scored at only a few percent. It is
		// a strict, monotonic generalization: when region_area <=
		// candidate_area (the original intended "small region inside its bigger
		// owning element" case) min(...) == region_area, so the value is
		// bit-for-bit identical to the old formula; when candidate_area <
		// region_area the denominator only SHRINKS, so the score can only rise,
		// never fall — every (region,candidate) pair that cleared the threshold
		// before still clears it. c.Area() (>= 1 here, since inter is
		// non-empty the candidate rect is non-degenerate) is already computed.
		double denom = min(region_area, (double)c.Area());
		double overlap = denom > 0.0
		                 ? ((double)inter.Width() * inter.Height()) / denom
		                 : 0.0;
		if(overlap < kOverlapThreshold)
			continue;
		// Direction-aware tie-break (task 0125). Under the OLD
		// intersection/region_area formula a 1.0 score could ONLY mean
		// "region fully inside candidate", so among equal scorers the
		// smallest-area candidate was the tightest container of the region —
		// the right, most-specific pick. The min()-denominator generalization
		// above lets a 1.0 ALSO mean "candidate fully inside region" (the
		// dilution-fix case), and those two meanings can tie at 1.0
		// simultaneously — e.g. a bottom-of-screen changed region is both
		// inside the full-canvas "Board" element (region ⊆ Board) AND fully
		// contains the small "SpeedSlider" widget that happens to sit in it
		// (SpeedSlider ⊆ region). Plain smallest-area-wins would then hand the
		// region to SpeedSlider purely on geometry, even though nothing about
		// the slider's own pixels changed (a false positive). So on an overlap
		// tie we FIRST prefer the candidate that fully CONTAINS the region
		// (restoring the original semantics exactly for the case it was
		// designed for), and only then fall back to smaller area. A contained-
		// but-not-containing candidate can still win outright when it has the
		// STRICTLY higher score, or when no containing candidate clears the
		// threshold at all — which is exactly the intended dilution fix (a
		// tiny button_puck inside a big merged region, with no sub-slot
		// containing that whole region, still wins its tier).
		bool c_contains = c.rect.Contains(region_rect);
		bool take;
		if(!result.best)
			take = true;
		else if(overlap > result.overlap + 1e-9)
			take = true;
		else if(overlap < result.overlap - 1e-9)
			take = false;
		else if(c_contains != result_contains)
			take = c_contains;             // container beats contained on a tie
		else
			take = c.Area() < result.best->Area();
		if(take) {
			result.best = &c;
			result.overlap = overlap;
			result_contains = c_contains;
		}
	}
	return result;
}

VsmMatchResult VsmMatchRegion(const Rect& region_rect,
                               const Vector<VsmLayoutCandidate>& candidates)
{
	double region_area = (double)max(1, region_rect.Width()) * max(1, region_rect.Height());

	VsmMatchResult subslot_match = VsmMatchTier(region_rect, region_area, candidates, "subslot");
	if(subslot_match.best)
		return subslot_match;
	return VsmMatchTier(region_rect, region_area, candidates, "element");
}

} // namespace Upp
