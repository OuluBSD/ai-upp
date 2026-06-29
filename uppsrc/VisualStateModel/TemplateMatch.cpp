#include "VisualStateModel.h"

namespace Upp {

void VsmTemplateMatcher::AddSyntheticAsset(const String& asset_id,
                                            const VsmFingerprint32& fp)
{
	SyntheticEntry& e = synthetic_.Add();
	e.asset_id = asset_id;
	e.fp       = fp;
}

bool VsmTemplateMatcher::LoadAssetFingerprint(const String& asset_id,
                                               VsmFingerprint32& out) const
{
	// Check synthetic first
	for(const SyntheticEntry& e : synthetic_)
		if(e.asset_id == asset_id) { out = e.fp; return true; }

	// Try loading from session root (placeholder files have no usable image)
	// For real images, this would load and downsample.
	// For now: session root assets are placeholders — we can't extract a fingerprint.
	return false;
}

VsmTemplateMatchResult VsmTemplateMatcher::Match(const VsmFrameImage& region_img,
                                                  const VsmTemplateRule& rule)
{
	VsmTemplateMatchResult result;
	result.rule_id = rule.rule_id;
	result.ts      = AsString(GetUtcTime());

	if(rule.candidates.IsEmpty()) {
		LogWarn(log_, "VsmTM", "Rule '" + rule.rule_id + "' has no candidates");
		result.is_required_failure = (rule.requirement == VSM_TMR_REQUIRED_ONE);
		return result;
	}

	// Extract query fingerprint
	VsmFingerprint32 query;
	bool have_query = VsmRegionMemory::ExtractFingerprint(
	    region_img, 0, 0, region_img.width, region_img.height, query);
	if(!have_query) {
		LogWarn(log_, "VsmTM", "Cannot extract fingerprint from region image");
		result.is_required_failure = (rule.requirement == VSM_TMR_REQUIRED_ONE);
		return result;
	}

	// Find best match among candidates
	double best_score  = 0.0;
	String best_label;

	for(const VsmTemplateCandidate& cand : rule.candidates) {
		VsmFingerprint32 asset_fp;
		if(!LoadAssetFingerprint(cand.asset_id, asset_fp)) {
			LogWarn(log_, "VsmTM", "Asset not available: " + cand.asset_id);
			continue;
		}
		// Distance in [0,1], score = 1 - distance
		double dist  = VsmRegionMemory::Distance(query, asset_fp);
		double score = 1.0 - dist;
		if(score > best_score) {
			best_score = score;
			best_label = cand.label;
		}
	}

	result.score = best_score;
	result.matched = (best_score >= rule.threshold);
	result.matched_label = best_label;
	result.location_x = 0; result.location_y = 0; // footprint location placeholder

	if(result.matched) {
		LogInfo(log_, "VsmTM", Format("Rule '%s': matched '%s' score=%.3f",
		                               rule.rule_id, best_label, best_score));
	} else {
		String detail = Format("Rule '%s': no match (best=%.3f < thr=%.3f)",
		                        rule.rule_id, best_score, rule.threshold);
		if(rule.requirement == VSM_TMR_REQUIRED_ONE) {
			LogWarn(log_, "VsmTM", detail + " [REQUIRED MATCH FAILED]");
			result.is_required_failure = true;
		} else {
			LogInfo(log_, "VsmTM", detail);
		}
	}
	return result;
}

} // namespace Upp
