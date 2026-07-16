#ifndef _VideoConfidenceTieredCandidates_VideoConfidenceTieredCandidates_h_
#define _VideoConfidenceTieredCandidates_VideoConfidenceTieredCandidates_h_

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <plugin/jpg/jpg.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Task 0261: Confidence-Tiered Recognition Pipeline
//
// Per-candidate result schema (Phase 1). Every changed-region candidate is
// resolved by exactly ONE tier, tried in strict order:
//
//   1. "structural"        (confidence 0.95) - board/state-transition keypoint
//                          events (flop/turn/river dealt, new hand, dealer
//                          rotation) that positively explain a candidate by
//                          frame-range + semantic region. Highest confidence:
//                          derived from cross-frame game-rule structure, not
//                          from the per-crop pixels at all.
//   2. "template_match"    (confidence 0.90) - exact rgb8x8 signature match of
//                          the candidate crop against a library of
//                          previously human-confirmed crops. Reuses
//                          VideoChangedRegionReview's Signature() verbatim.
//   3. "autoencoder_cluster" (confidence 0.30) - lowest-confidence fallback;
//                          only candidates unresolved by tiers 1+2 are routed
//                          here (0257/0260 autoencoder bottleneck + k-means).
//                          Low trust per Task 0260's finding.
//   4. "unresolved"        - fell through every tier.
//
// A candidate resolved at an earlier tier is never reprocessed by a later
// tier (no double-counting, no conflicting labels).
// ---------------------------------------------------------------------------

struct KeypointEvent : Moveable<KeypointEvent> {
	String event_type;   // "flop_dealt" / "turn_dealt" / "river_dealt" /
	                     // "new_hand" / "dealer_rotation"
	int    table_id  = -1;
	int    frame     = -1; // frame at which the new state is first observed
	int    prev_frame = -1;
	int    from_count = -1; // board card count before
	int    to_count   = -1; // board card count after
	int    street     = -1; // 1=flop,2=turn,3=river (for board events)
	int    from_seat  = -1; // dealer rotation
	int    to_seat    = -1;
	String description;
};

END_UPP_NAMESPACE

#endif
