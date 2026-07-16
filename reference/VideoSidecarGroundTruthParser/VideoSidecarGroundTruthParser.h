#ifndef _VideoSidecarGroundTruthParser_VideoSidecarGroundTruthParser_h_
#define _VideoSidecarGroundTruthParser_VideoSidecarGroundTruthParser_h_

#include <Core/Core.h>
#include <TexasHoldem/TexasHoldemLogicState.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Task 0262: Real-Recording Sidecar Ground Truth Ingestion (First Pass)
//
// Parses a hand-authored sidecar narration text file (see e.g.
// bin/video_record_25min_20260716_203356.txt) of a REAL captured poker
// session into a TexasHoldemLogicState-shaped ground-truth JSONL -- the same
// per-frame snapshot schema reference/VideoGameEngineSyncer and
// reference/VideoConfidenceTieredCandidates already consume via
// --logic-jsonl.
//
// Sidecar format (genuinely new, no other consumer in the repo -- see the
// Manager task file 0262 for the full grammar spec):
//   - a small seat-position legend block (top=seat1, topright=seat2, ...)
//     local to THIS ONE recording's camera/window orientation, not a
//     general convention.
//   - a blank line, then one "R HH:MM:SS: <event>" line per narrated event.
//
// All the real parsing/state-machine logic lives in main.cpp (mirroring how
// reference/VideoGameEngineSyncer keeps its GameEngineSyncer class entirely
// in main.cpp, with this header staying a thin, shared-nothing include --
// nothing else in the repo needs to reuse this package's internal types).
// ---------------------------------------------------------------------------

END_UPP_NAMESPACE

#endif
