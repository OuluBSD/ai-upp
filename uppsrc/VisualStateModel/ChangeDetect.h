#ifndef _VisualStateModel_ChangeDetect_h_
#define _VisualStateModel_ChangeDetect_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Parameters for change detection

struct VsmChangeDetectParams {
	int    pixel_threshold   = 30;  // per-channel absolute diff to count as "changed"
	int    block_size        = 8;   // pixels per grid cell for coarse pass
	double block_min_score   = 0.05; // min fraction of changed pixels in a block to flag it
	int    merge_gap         = 16;  // merge rectangles whose gap is <= this many pixels
	int    min_region_area   = 64;  // discard changed regions smaller than this (px²)
};

// ---------------------------------------------------------------------------
// Perform pixel-level change detection between two same-size RGBA images.
// Returns a list of VsmChangedRect (merged, above noise threshold).

Vector<VsmChangedRect> VsmDetectChanges(
	const VsmFrameImage& prev,
	const VsmFrameImage& curr,
	const VsmChangeDetectParams& params = VsmChangeDetectParams());

// ---------------------------------------------------------------------------
// Helper: merge a list of rectangles that are close together into larger ones.

Vector<Rect> VsmMergeRects(const Vector<Rect>& rects, int gap);

// ---------------------------------------------------------------------------
// Frame comparison helper: build a VsmChangeEvent from two frames.
// Fills in frame index, ts, and detected regions.

VsmChangeEvent VsmCompareFrames(
	const VsmFrameImage& prev,
	const VsmFrameImage& curr,
	int                  frame_index,
	const String&        ts,
	const VsmChangeDetectParams& params = VsmChangeDetectParams());

} // namespace Upp

#endif
