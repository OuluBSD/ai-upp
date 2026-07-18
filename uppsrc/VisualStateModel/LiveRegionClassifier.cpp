#include "VisualStateModel.h"
#include <plugin/jpg/jpg.h>

namespace Upp {

// ---------------------------------------------------------------------------
// rgb8x8 signature -- copied VERBATIM from reference/VideoConfidenceTieredCandidates
// /main.cpp's Signature() (itself copied from VideoChangedRegionReview) so a
// signature computed here is bit-for-bit comparable to those tools' library.
// ---------------------------------------------------------------------------
String VsmLiveRegionClassifier::Signature(const Image& image)
{
	String result = "rgb8x8:";
	for(int y = 0; y < 8; y++)
		for(int x = 0; x < 8; x++) {
			int sx = min(image.GetWidth() - 1, x * image.GetWidth() / 8);
			int sy = min(image.GetHeight() - 1, y * image.GetHeight() / 8);
			Color c = image[sy][sx];
			result << Format("%02x%02x%02x", c.GetR() >> 4, c.GetG() >> 4, c.GetB() >> 4);
		}
	return result;
}

static int VsmHexNibble(int c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}

static int VsmHexByte(const String& s, int off)
{
	return VsmHexNibble(s[off]) * 16 + VsmHexNibble(s[off + 1]);
}

static const char* RGB8X8_PREFIX = "rgb8x8:";
static const int RGB8X8_BLOCK_COUNT = 64;

int VsmLiveRegionClassifier::SignatureDistance(const String& a, const String& b)
{
	int prefix_len = (int)strlen(RGB8X8_PREFIX);
	int expect_len = prefix_len + RGB8X8_BLOCK_COUNT * 6;
	if(a.GetCount() != expect_len || b.GetCount() != expect_len) return -1;
	if(!a.StartsWith(RGB8X8_PREFIX) || !b.StartsWith(RGB8X8_PREFIX)) return -1;
	int total = 0;
	for(int i = 0; i < RGB8X8_BLOCK_COUNT; i++) {
		int off = prefix_len + i * 6;
		int r1 = VsmHexByte(a, off), g1 = VsmHexByte(a, off + 2), b1 = VsmHexByte(a, off + 4);
		int r2 = VsmHexByte(b, off), g2 = VsmHexByte(b, off + 2), b2 = VsmHexByte(b, off + 4);
		total += abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2);
	}
	return total;
}

static String VsmJsonText(ValueMap m, const char* key)
{
	Value v = m.Get(key, Value());
	return IsVoid(v) || IsNull(v) ? String() : AsString(v);
}

int VsmLiveRegionClassifier::Load(const String& dataset_json_path)
{
	entries_.Clear();
	if(!FileExists(dataset_json_path)) {
		LogError(log_, "LiveRegionClassifier", "dataset not found: " + dataset_json_path);
		return 0;
	}
	Value root = ParseJSON(LoadFile(dataset_json_path));
	if(IsError(root) || !IsValueMap(root)) {
		LogError(log_, "LiveRegionClassifier", "dataset is not a JSON object: " + dataset_json_path);
		return 0;
	}
	ValueMap m = root;
	ValueArray cands = m.Get("candidates", ValueArray());
	int skipped_no_cat = 0, skipped_no_crop = 0, skipped_decode = 0;
	for(int i = 0; i < cands.GetCount(); i++) {
		ValueMap c = cands[i];
		String cat = VsmJsonText(c, "category");
		if(cat.IsEmpty()) { skipped_no_cat++; continue; }
		String crop_path = VsmJsonText(c, "crop_path");
		if(crop_path.IsEmpty() || !FileExists(crop_path)) { skipped_no_crop++; continue; }
		Image img = StreamRaster::LoadFileAny(crop_path);
		if(img.IsEmpty()) { skipped_decode++; continue; }

		Entry& e = entries_.Add();
		e.global_index = c.Find("global_index") >= 0 ? (int)c["global_index"] : i;
		e.category = cat;
		e.rank = VsmJsonText(c, "rank");
		e.suit = VsmJsonText(c, "suit");
		if(c.Find("rect") >= 0 && IsValueMap(c["rect"])) {
			ValueMap rc = c["rect"];
			e.x = rc.Find("x") >= 0 ? (int)rc["x"] : 0;
			e.y = rc.Find("y") >= 0 ? (int)rc["y"] : 0;
		}
		e.w = img.GetWidth();
		e.h = img.GetHeight();
		e.signature = Signature(img);
	}
	LogInfo(log_, "LiveRegionClassifier",
	        Format("loaded %d reference entries "
	               "(skipped: no_category=%d missing_crop=%d decode_fail=%d)",
	               entries_.GetCount(), skipped_no_cat, skipped_no_crop, skipped_decode));
	return entries_.GetCount();
}

VsmLiveClassifyResult VsmLiveRegionClassifier::Classify(const Image& crop, int rx, int ry,
                                                        int exclude_global_index) const
{
	VsmLiveClassifyResult out;
	if(crop.IsEmpty() || entries_.IsEmpty()) return out;

	String sig = Signature(crop);
	int w = crop.GetWidth(), h = crop.GetHeight();

	// ---------------------------------------------------------------------
	// This UI is a fixed layout: every semantic element re-appears at the same
	// place with the same size, so POSITION is the primary discriminator. But
	// the live change-detector does NOT reproduce the labeled dataset's exact
	// region rects (different merge granularity), so a hard same-dimension gate
	// (as VideoConfidenceTieredCandidates used for its already-collected,
	// dimension-matched candidates) throws away most live regions. Instead we
	// anchor on the region CENTRE and allow cross-dimension rgb8x8 comparison
	// (the 8x8 grid samples 64 points regardless of crop size, so signatures of
	// differently-sized crops are still comparable, just noisier).
	//
	// Tier A -- EXACT rect (x,y,w,h): strongest possible signal.
	// Tier P -- centre within POS_RADIUS: score = signature distance + a
	//           position penalty + a size-mismatch penalty; pick the minimum.
	// Tier D -- nearest signature anywhere, only if within near_max_.
	// ---------------------------------------------------------------------
	const int   POS_RADIUS = 60;   // px centre distance in the 944x682 table space
	const double W_POS  = 3.0;     // per-px centre-distance penalty
	const double W_SIZE = 1.5;     // per-px (w+h) size-mismatch penalty

	int qcx = rx + w / 2, qcy = ry + h / 2;

	int a_idx = -1, a_dist = RGB8X8_MAX_DISTANCE + 1;
	int p_idx = -1, p_sig = RGB8X8_MAX_DISTANCE + 1;
	double p_score = 1e18;
	int d_idx = -1, d_dist = RGB8X8_MAX_DISTANCE + 1;

	for(int i = 0; i < entries_.GetCount(); i++) {
		const Entry& e = entries_[i];
		if(e.global_index == exclude_global_index) continue;
		int dist = SignatureDistance(sig, e.signature);
		if(dist < 0) continue;

		if(e.x == rx && e.y == ry && e.w == w && e.h == h) {
			if(dist < a_dist) { a_dist = dist; a_idx = i; }
		}

		int ecx = e.x + e.w / 2, ecy = e.y + e.h / 2;
		int cd = abs(ecx - qcx) + abs(ecy - qcy);
		if(cd <= POS_RADIUS) {
			double score = dist + W_POS * cd + W_SIZE * (abs(e.w - w) + abs(e.h - h));
			if(score < p_score) { p_score = score; p_idx = i; p_sig = dist; }
		}

		if(dist < d_dist) { d_dist = dist; d_idx = i; }
	}

	auto fill = [&](int idx, int dist, const char* tier, double conf) {
		const Entry& e = entries_[idx];
		out.category = e.category;
		out.rank = e.rank;
		out.suit = e.suit;
		out.confidence = conf;
		out.distance = dist;
		out.matched_index = e.global_index;
		out.tier = tier;
	};

	// Confidence tapers with signature distance.
	auto conf_band = [&](int dist, double hi, double lo) {
		double frac = min(1.0, (double)dist / (double)near_max_);
		return hi - (hi - lo) * frac;
	};

	if(a_idx >= 0) { fill(a_idx, a_dist, "exact_rect", conf_band(a_dist, 0.95, 0.75)); return out; }
	if(p_idx >= 0) { fill(p_idx, p_sig, "pos_anchored", conf_band(p_sig, 0.85, 0.45)); return out; }
	if(d_idx >= 0 && d_dist <= near_max_) { fill(d_idx, d_dist, "near_sig_any_pos", 0.35); return out; }

	return out; // unresolved
}

} // namespace Upp
