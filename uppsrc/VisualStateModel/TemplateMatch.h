#ifndef _VisualStateModel_TemplateMatch_h_
#define _VisualStateModel_TemplateMatch_h_

namespace Upp {

enum VsmTemplateMatchMode {
	VSM_TM_PRESENCE      = 0, // detect whether a single template is present
	VSM_TM_MULTI_OPTION  = 1, // classify among multiple template options
};

enum VsmTemplateMatchReq {
	VSM_TMR_OPTIONAL          = 0, // no match is acceptable (signal only)
	VSM_TMR_REQUIRED_ONE      = 1, // at least one match required
};

struct VsmTemplateAsset : Moveable<VsmTemplateAsset> {
	String asset_id;
	String relative_path; // in session directory
	String label;         // human-readable name
	void Jsonize(JsonIO& json) {
		json("asset_id",asset_id)("relative_path",relative_path)("label",label);
	}
};

struct VsmTemplateCandidate : Moveable<VsmTemplateCandidate> {
	String asset_id;
	String label;
	void Jsonize(JsonIO& json) { json("asset_id",asset_id)("label",label); }
};

struct VsmTemplateRule : Moveable<VsmTemplateRule> {
	String              rule_id;
	String              annotation_id; // which region this rule applies to
	int                 mode        = VSM_TM_PRESENCE;
	int                 requirement = VSM_TMR_OPTIONAL;
	double              threshold   = 0.8;
	String              pipeline_id; // preprocessing pipeline to apply first
	Vector<VsmTemplateCandidate> candidates;
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("annotation_id",annotation_id)
		    ("mode",mode)("requirement",requirement)
		    ("threshold",threshold)("pipeline_id",pipeline_id)
		    ("candidates",candidates);
	}
};

struct VsmTemplateMatchResult : Moveable<VsmTemplateMatchResult> {
	String  rule_id;
	bool    matched    = false;
	double  score      = 0.0;
	int     location_x = -1, location_y = -1;
	String  matched_label; // best candidate label (multi-option mode)
	String  ts;
	// required-no-match is a failure
	bool    is_required_failure = false;
	void Jsonize(JsonIO& json) {
		json("rule_id",rule_id)("matched",matched)("score",score)
		    ("location_x",location_x)("location_y",location_y)
		    ("matched_label",matched_label)("ts",ts)
		    ("is_required_failure",is_required_failure);
	}
};

// ---------------------------------------------------------------------------
// Template matching executor
//
// Uses nearest-neighbor 32×32 thumbnails and sum-of-squared-differences (SSD).
// Asset images are loaded from session paths (placeholder or RGBA raw file).
// For headless testing, a synthetic asset can be injected via AddSyntheticAsset.

class VsmTemplateMatcher {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	void SetSessionRoot(const String& root) { session_root_ = root; }

	// Inject a synthetic fingerprint for an asset_id (used in tests).
	void AddSyntheticAsset(const String& asset_id, const VsmFingerprint32& fp);

	VsmTemplateMatchResult Match(const VsmFrameImage& region_img,
	                              const VsmTemplateRule& rule);

private:
	CoreLog log_;
	String  session_root_;

	// Synthetic assets injected for headless testing
	struct SyntheticEntry : Moveable<SyntheticEntry> {
		String         asset_id;
		VsmFingerprint32 fp;
	};
	Vector<SyntheticEntry> synthetic_;

	bool LoadAssetFingerprint(const String& asset_id, VsmFingerprint32& out) const;
};

} // namespace Upp

#endif
