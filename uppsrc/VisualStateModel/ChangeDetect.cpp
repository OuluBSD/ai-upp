#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// Internal: coarse block-level diff pass

static Vector<Rect> sBlockDiff(const VsmFrameImage& prev, const VsmFrameImage& curr,
                                const VsmChangeDetectParams& p)
{
	Vector<Rect> dirty;
	int w = prev.width, h = prev.height;
	int bs = p.block_size;
	int cx = (w + bs - 1) / bs;
	int cy = (h + bs - 1) / bs;

	for(int gy = 0; gy < cy; gy++) {
		for(int gx = 0; gx < cx; gx++) {
			int x0 = gx * bs, y0 = gy * bs;
			int x1 = min(x0 + bs, w), y1 = min(y0 + bs, h);
			int total = (x1 - x0) * (y1 - y0);
			int changed = 0;
			for(int y = y0; y < y1; y++) {
				const byte* pp = prev.data + (y * w + x0) * 4;
				const byte* cp = curr.data + (y * w + x0) * 4;
				for(int x = x0; x < x1; x++, pp += 4, cp += 4) {
					int dr = abs((int)pp[0] - (int)cp[0]);
					int dg = abs((int)pp[1] - (int)cp[1]);
					int db = abs((int)pp[2] - (int)cp[2]);
					if(dr > p.pixel_threshold || dg > p.pixel_threshold || db > p.pixel_threshold)
						changed++;
				}
			}
			if(changed > (int)(total * p.block_min_score + 0.5))
				dirty.Add(Rect(x0, y0, x1, y1));
		}
	}
	return dirty;
}

// ---------------------------------------------------------------------------

Vector<Rect> VsmMergeRects(const Vector<Rect>& rects, int gap)
{
	if(rects.IsEmpty()) return Vector<Rect>();

	// Grow each rect by gap/2 on all sides, union overlapping, shrink back.
	int half = gap / 2;
	Vector<Rect> grown;
	grown.Reserve(rects.GetCount());
	for(const Rect& r : rects)
		grown.Add(Rect(r.left - half, r.top - half, r.right + half, r.bottom + half));

	// Simple O(n²) union — acceptable for the small counts expected here.
	bool merged = true;
	while(merged) {
		merged = false;
		for(int i = 0; i < grown.GetCount(); i++) {
			for(int j = i + 1; j < grown.GetCount(); j++) {
				Rect a = grown[i], b = grown[j];
				// Check overlap
				if(a.left < b.right && a.right > b.left &&
				   a.top  < b.bottom && a.bottom > b.top) {
					grown[i] = Rect(min(a.left, b.left), min(a.top, b.top),
					                max(a.right, b.right), max(a.bottom, b.bottom));
					grown.Remove(j);
					merged = true;
					break;
				}
			}
			if(merged) break;
		}
	}

	// Shrink back and clip to image bounds
	Vector<Rect> result;
	for(const Rect& r : grown) {
		Rect shrunk(r.left + half, r.top + half, r.right - half, r.bottom - half);
		if(!shrunk.IsEmpty())
			result.Add(shrunk);
	}
	return result;
}

// ---------------------------------------------------------------------------

Vector<VsmChangedRect> VsmDetectChanges(const VsmFrameImage& prev, const VsmFrameImage& curr,
                                         const VsmChangeDetectParams& params)
{
	Vector<VsmChangedRect> result;
	if(prev.IsEmpty() || curr.IsEmpty()) return result;
	if(prev.width != curr.width || prev.height != curr.height) return result;

	Vector<Rect> blocks = sBlockDiff(prev, curr, params);
	if(blocks.IsEmpty()) return result;

	Vector<Rect> merged = VsmMergeRects(blocks, params.merge_gap);

	int w = prev.width;
	for(const Rect& r : merged) {
		int area = r.GetWidth() * r.GetHeight();
		if(area < params.min_region_area) continue;

		// Compute exact score inside the merged rect
		int total = 0, changed = 0;
		for(int y = r.top; y < r.bottom; y++) {
			const byte* pp = prev.data + (y * w + r.left) * 4;
			const byte* cp = curr.data + (y * w + r.left) * 4;
			for(int x = r.left; x < r.right; x++, pp += 4, cp += 4) {
				int dr = abs((int)pp[0] - (int)cp[0]);
				int dg = abs((int)pp[1] - (int)cp[1]);
				int db = abs((int)pp[2] - (int)cp[2]);
				if(dr > params.pixel_threshold || dg > params.pixel_threshold ||
				   db > params.pixel_threshold)
					changed++;
				total++;
			}
		}

		VsmChangedRect& cr = result.Add();
		cr.x     = r.left;
		cr.y     = r.top;
		cr.w     = r.GetWidth();
		cr.h     = r.GetHeight();
		cr.score = total > 0 ? double(changed) / total : 0.0;
	}
	return result;
}

// ---------------------------------------------------------------------------

VsmChangeEvent VsmCompareFrames(const VsmFrameImage& prev, const VsmFrameImage& curr,
                                 int frame_index, const String& ts,
                                 const VsmChangeDetectParams& params)
{
	VsmChangeEvent ev;
	ev.frame = frame_index;
	ev.ts    = ts;
	ev.regions = VsmDetectChanges(prev, curr, params);
	return ev;
}

} // namespace Upp
