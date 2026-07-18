#ifndef _VisualStateModel_LiveRegionClassifier_h_
#define _VisualStateModel_LiveRegionClassifier_h_

// ---------------------------------------------------------------------------
// Task 0280: the ONE genuinely-new recognition piece this milestone needs -- a
// fast, real-time-capable classifier that maps a brand-new, never-before-seen
// changed region (arriving live from VsmVideoServerFrameSource) to a semantic
// category, using the existing 267-candidate human/heuristic-labeled dataset
// (tmp/real_recording_combined_classified_dataset.json, Tasks 0266/0269) as a
// template-matching reference set.
//
// Everything the project built before this runs OFFLINE over an already-
// collected, already-labeled batch (VideoChangedRegionClusterer,
// VideoConfidenceTieredCandidates). None of them classify a fresh live region.
//
// The matching core is adapted (NOT reinvented) from
// reference/VideoConfidenceTieredCandidates/main.cpp's template_match tier
// (Task 0267): the rgb8x8 crop signature (Signature()) + its Manhattan-distance
// near-match (SignatureDistance()/NEAR_MATCH_MAX_DISTANCE). That code was
// validated against real Aludra captures: genuinely-same-content crop pairs
// topped out at distance ~74 (of 2880) while genuinely-different-content pairs
// bottomed out at ~285 -- a clear ~4x gap. This class reuses that exact
// fingerprint and threshold, but drives it PER-INCOMING-REGION against the
// labeled dataset instead of over a pre-collected candidate list.
// ---------------------------------------------------------------------------

namespace Upp {

struct VsmLiveClassifyResult : Moveable<VsmLiveClassifyResult> {
	String category;            // best-matching labeled category ("" = unresolved)
	String rank, suit;          // carried through for board_card / hole_card_faceup
	double confidence = 0.0;    // 0..1
	int    distance = -1;       // rgb8x8 Manhattan distance to the matched entry (-1 none)
	int    matched_index = -1;  // global_index of the matched dataset candidate
	String tier = "unresolved"; // exact_dim_exact_sig|exact_dim_near_sig|any_dim_near_sig|unresolved
};

class VsmLiveRegionClassifier {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Load the labeled dataset, computing an rgb8x8 signature per labeled crop
	// whose file exists on disk. Returns the number of usable reference entries
	// (candidates with a non-empty category AND a readable crop). Entries whose
	// crop file is missing/undecodable are skipped (reported via the log), so a
	// partial dataset still yields a working -- if weaker -- classifier.
	int  Load(const String& dataset_json_path);
	int  GetCount() const { return entries_.GetCount(); }

	// Classify one crop image against the loaded reference set.
	//
	// (rx,ry) is the changed region's ORIGIN in the (static) table-crop
	// coordinate space -- position is a very strong discriminator on this
	// fixed-layout poker UI (the pot label is always centre-top, a seat plate
	// always at its seat), so the matcher combines rect position + rect size +
	// the rgb8x8 pixel signature rather than appearance alone. Pass the same
	// coordinate space the labeled dataset's rects live in.
	//
	// exclude_global_index (default -1 = none) drops the reference entry with
	// that global_index from the search, which is what makes a rigorous
	// leave-one-out self-test possible (classify a labeled crop WITHOUT letting
	// it match itself).
	VsmLiveClassifyResult Classify(const Image& crop, int rx, int ry,
	                               int exclude_global_index = -1) const;

	// rgb8x8 crop signature -- byte-for-byte the same fingerprint
	// VideoConfidenceTieredCandidates / VideoChangedRegionReview key on, so a
	// signature computed here is directly comparable to those tools' output.
	static String Signature(const Image& image);

	// Manhattan (L1) distance between two rgb8x8 signature strings of identical
	// encoded length. Returns -1 if either is not a well-formed rgb8x8 signature.
	// Callers must only compare signatures whose source crops have matching
	// pixel dimensions (the 8x8 grid samples 64 points regardless of crop size,
	// so equal string length does NOT imply equal dimensions).
	static int SignatureDistance(const String& a, const String& b);

	static const int RGB8X8_MAX_DISTANCE = 64 * 3 * 15; // 2880

	void SetNearMatchMaxDistance(int d) { near_max_ = max(1, d); }
	int  GetNearMatchMaxDistance() const { return near_max_; }

private:
	struct Entry : Moveable<Entry> {
		int    global_index = -1;
		String category, rank, suit;
		int    x = 0, y = 0, w = 0, h = 0;
		String signature;
	};
	Vector<Entry> entries_;
	CoreLog       log_;
	int           near_max_ = 120; // NEAR_MATCH_MAX_DISTANCE, Task 0267 real-evidence value
};

} // namespace Upp

#endif
