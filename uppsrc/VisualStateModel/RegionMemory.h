#ifndef _VisualStateModel_RegionMemory_h_
#define _VisualStateModel_RegionMemory_h_

namespace Upp {

// ---------------------------------------------------------------------------
// A frame image adapter — raw RGBA bytes without Draw/GUI dependency

struct VsmFrameImage {
	int    width  = 0;
	int    height = 0;
	Buffer<byte> data; // RGBA, row-major

	bool IsEmpty() const { return width == 0 || height == 0; }

	// Fill from a flat RGBA buffer (width * height * 4 bytes expected)
	void Set(int w, int h, const byte* rgba);

	// Get pixel (clamped)
	void GetPixel(int x, int y, byte& r, byte& g, byte& b, byte& a) const;
};

// ---------------------------------------------------------------------------
// A matched region from memory

struct VsmRegionMatch : Moveable<VsmRegionMatch> {
	VsmRegionId region_id;
	double      distance = 1.0; // 0 = identical, 1 = maximally different
};

// ---------------------------------------------------------------------------
// Compact 32x32 grayscale fingerprint (1024 bytes)

struct VsmFingerprint32 : Moveable<VsmFingerprint32> {
	enum { SIZE = 32 };
	byte data[SIZE * SIZE] = {};

	String ComputeHash() const; // returns "sha1:..." hex string
};

// ---------------------------------------------------------------------------
// Region memory — stores fingerprints and supports nearest-match query

class VsmRegionMemory {
public:
	VsmRegionMemory() {}

	void SetLog(AppLog* sink) { log_.SetSink(sink); }
	CoreLog& GetLog()         { return log_; }

	// Add a fingerprint for a known region id
	void Add(const VsmRegionId& id, const VsmFingerprint32& fp);

	// Find the closest matching region for a query fingerprint.
	// Returns empty match if no entry within max_distance.
	VsmRegionMatch FindNearest(const VsmFingerprint32& query, double max_distance = 0.3) const;

	// Extract a 32x32 grayscale fingerprint from an image region
	static bool ExtractFingerprint(const VsmFrameImage& img, int x, int y, int w, int h,
	                               VsmFingerprint32& out);

	// Compute normalized mean absolute difference between two fingerprints (0..1)
	static double Distance(const VsmFingerprint32& a, const VsmFingerprint32& b);

	int GetCount() const { return entries_.GetCount(); }
	void Clear()         { entries_.Clear(); }

private:
	struct Entry : Moveable<Entry> {
		VsmRegionId      id;
		VsmFingerprint32 fp;
	};
	Vector<Entry> entries_;
	CoreLog       log_;
};

} // namespace Upp

#endif
