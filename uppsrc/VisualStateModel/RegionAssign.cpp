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
	for(const VsmLayoutCandidate& c : candidates) {
		if(c.kind != tier_kind)
			continue;
		Rect inter = region_rect & c.rect;
		if(inter.IsEmpty())
			continue;
		double overlap = ((double)inter.Width() * inter.Height()) / region_area;
		if(overlap < kOverlapThreshold)
			continue;
		if(!result.best || overlap > result.overlap + 1e-9 ||
		   (fabs(overlap - result.overlap) <= 1e-9 && c.Area() < result.best->Area())) {
			result.best = &c;
			result.overlap = overlap;
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
