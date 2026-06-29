#include "VisualStateModel/VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmFrameImage

void VsmFrameImage::Set(int w, int h, const byte* rgba)
{
	width  = w;
	height = h;
	int sz = w * h * 4;
	data.Alloc(sz);
	if(rgba && sz > 0)
		memcpy(data, rgba, sz);
}

void VsmFrameImage::GetPixel(int x, int y, byte& r, byte& g, byte& b, byte& a) const
{
	x = clamp(x, 0, width  - 1);
	y = clamp(y, 0, height - 1);
	const byte* p = data + (y * width + x) * 4;
	r = p[0]; g = p[1]; b = p[2]; a = p[3];
}

// ---------------------------------------------------------------------------
// VsmFingerprint32

String VsmFingerprint32::ComputeHash() const
{
	return "md5:" + MD5String((const char*)data, SIZE * SIZE);
}

// ---------------------------------------------------------------------------
// VsmRegionMemory

void VsmRegionMemory::Add(const VsmRegionId& id, const VsmFingerprint32& fp)
{
	Entry& e = entries_.Add();
	e.id = id;
	e.fp = fp;
	LogDebug(log_, "VsmRegionMemory", Format("Added fingerprint for region %s (total %d)",
	                                          id, entries_.GetCount()));
}

VsmRegionMatch VsmRegionMemory::FindNearest(const VsmFingerprint32& query,
                                             double max_distance) const
{
	VsmRegionMatch best;
	best.distance = max_distance;
	for(const Entry& e : entries_) {
		double d = Distance(query, e.fp);
		if(d < best.distance) {
			best.distance  = d;
			best.region_id = e.id;
		}
	}
	return best;
}

bool VsmRegionMemory::ExtractFingerprint(const VsmFrameImage& img, int x, int y,
                                          int w, int h, VsmFingerprint32& out)
{
	if(img.IsEmpty() || w <= 0 || h <= 0) return false;

	const int N = VsmFingerprint32::SIZE;
	for(int fy = 0; fy < N; fy++) {
		for(int fx = 0; fx < N; fx++) {
			int sx = x + fx * w / N;
			int sy = y + fy * h / N;
			byte r, g, b, a;
			img.GetPixel(sx, sy, r, g, b, a);
			out.data[fy * N + fx] = (byte)((int(r) * 77 + int(g) * 150 + int(b) * 29) >> 8);
		}
	}
	return true;
}

double VsmRegionMemory::Distance(const VsmFingerprint32& a, const VsmFingerprint32& b)
{
	const int N = VsmFingerprint32::SIZE * VsmFingerprint32::SIZE;
	int64 sum = 0;
	for(int i = 0; i < N; i++)
		sum += abs((int)a.data[i] - (int)b.data[i]);
	return double(sum) / (N * 255);
}

} // namespace Upp
