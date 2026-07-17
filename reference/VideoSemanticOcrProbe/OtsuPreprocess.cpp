#include "VideoSemanticOcrProbe.h"
#include "OtsuPreprocess.h"
#include <cmath>

NAMESPACE_UPP

// Shared histogram-based Otsu variance-maximization, ported verbatim (loop
// structure and variable names kept close to the original) from
// ConvNetCpp/src/OCR/ImageUtils.cpp:231-273's OtsuThreshold(). Generalized
// to take an already-built histogram so both the grayscale-binarization
// path (Phase 1) and the gradient-magnitude edge/non-edge split
// (Phase 2) can reuse the identical, proven algorithm instead of
// duplicating it.
static int OtsuThresholdFromHistogram(const Vector<int>& histogram)
{
	int bins = histogram.GetCount();
	int total = 0;
	for(int i = 0; i < bins; i++)
		total += histogram[i];
	if(total == 0)
		return bins / 2;

	double sum = 0;
	for(int i = 0; i < bins; i++)
		sum += i * histogram[i];

	double sum_bg = 0;
	int    weight_bg = 0;
	double max_variance = 0;
	int    threshold = 0;

	for(int t = 0; t < bins; t++) {
		weight_bg += histogram[t];
		if(weight_bg == 0)
			continue;

		int weight_fg = total - weight_bg;
		if(weight_fg == 0)
			break;

		sum_bg += t * histogram[t];

		double mean_bg = sum_bg / weight_bg;
		double mean_fg = (sum - sum_bg) / weight_fg;

		double variance = (double)weight_bg * weight_fg * (mean_bg - mean_fg) * (mean_bg - mean_fg);

		if(variance > max_variance) {
			max_variance = variance;
			threshold = t;
		}
	}

	return threshold;
}

Image OcrGrayscale(const Image& src)
{
	if(src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer ib(sz);
	for(int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = ib[y];
		for(int x = 0; x < sz.cx; x++) {
			byte g = (byte)Grayscale(s[x]);
			d[x].r = d[x].g = d[x].b = g;
			d[x].a = 255;
		}
	}
	return ib;
}

Image OcrInvert(const Image& src)
{
	if(src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer ib(sz);
	for(int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = ib[y];
		for(int x = 0; x < sz.cx; x++) {
			d[x].r = 255 - s[x].r;
			d[x].g = 255 - s[x].g;
			d[x].b = 255 - s[x].b;
			d[x].a = s[x].a;
		}
	}
	return ib;
}

Image OcrBinarize(const Image& gray, int threshold)
{
	if(gray.IsEmpty())
		return Image();
	Size sz = gray.GetSize();
	ImageBuffer ib(sz);
	for(int y = 0; y < sz.cy; y++) {
		const RGBA* s = gray[y];
		RGBA* d = ib[y];
		for(int x = 0; x < sz.cx; x++) {
			byte bw = (s[x].r >= threshold) ? 255 : 0;
			d[x].r = d[x].g = d[x].b = bw;
			d[x].a = 255;
		}
	}
	return ib;
}

int OcrOtsuThreshold(const Image& gray)
{
	if(gray.IsEmpty())
		return 128;
	Vector<int> histogram;
	histogram.SetCount(256, 0);
	Size sz = gray.GetSize();
	for(int y = 0; y < sz.cy; y++) {
		const RGBA* line = gray[y];
		for(int x = 0; x < sz.cx; x++)
			histogram[line[x].r]++;
	}
	return OtsuThresholdFromHistogram(histogram);
}

Image OcrPreprocessOtsu(const Image& src, bool invert)
{
	if(src.IsEmpty())
		return Image();
	Size target(src.GetWidth() * 3, src.GetHeight() * 3);
	Image upscaled = RescaleFilter(src, target, FILTER_BILINEAR);
	Image gray = OcrGrayscale(upscaled);
	Image bw = OcrBinarize(gray, OcrOtsuThreshold(gray));
	return invert ? OcrInvert(bw) : bw;
}

// ---------------------------------------------------------------------------
// Phase 2: convolution-based predictive polarity detector.

OcrPolarityResult OcrDetectPolarity(const Image& gray)
{
	OcrPolarityResult result;
	if(gray.IsEmpty())
		return result;

	Size sz = gray.GetSize();
	int w = sz.cx, h = sz.cy;
	if(w < 3 || h < 3)
		return result; // too small for a 3x3 Sobel kernel; not confident

	// 3x3 Sobel gradient magnitude over interior pixels.
	Vector<double> mag;
	mag.SetCount(w * h, 0.0);
	double max_mag = 0;
	for(int y = 1; y < h - 1; y++) {
		const RGBA* r0 = gray[y - 1];
		const RGBA* r1 = gray[y];
		const RGBA* r2 = gray[y + 1];
		for(int x = 1; x < w - 1; x++) {
			double gx = -(double)r0[x - 1].r + (double)r0[x + 1].r
			            - 2.0 * r1[x - 1].r + 2.0 * r1[x + 1].r
			            - (double)r2[x - 1].r + (double)r2[x + 1].r;
			double gy = -(double)r0[x - 1].r - 2.0 * r0[x].r - (double)r0[x + 1].r
			            + (double)r2[x - 1].r + 2.0 * r2[x].r + (double)r2[x + 1].r;
			double m = sqrt(gx * gx + gy * gy);
			mag[y * w + x] = m;
			if(m > max_mag)
				max_mag = m;
		}
	}

	if(max_mag <= 0)
		return result; // perfectly flat crop, no edges at all: not confident

	// Bucket the gradient-magnitude values into a 256-bin histogram scaled
	// to this crop's own observed magnitude range, then reuse the same
	// Otsu variance-maximization routine to find the edge/non-edge cut.
	Vector<int> mag_histogram;
	mag_histogram.SetCount(256, 0);
	for(int y = 1; y < h - 1; y++)
		for(int x = 1; x < w - 1; x++) {
			int bin = (int)((mag[y * w + x] / max_mag) * 255.0);
			bin = minmax(bin, 0, 255);
			mag_histogram[bin]++;
		}
	int mag_bin_threshold = OtsuThresholdFromHistogram(mag_histogram);
	double mag_cutoff = (mag_bin_threshold / 255.0) * max_mag;

	// Dominant (mode) grayscale intensity of the whole crop, used as the
	// background-level estimate (background is the majority of pixels).
	Vector<int> gray_histogram;
	gray_histogram.SetCount(256, 0);
	for(int y = 0; y < h; y++) {
		const RGBA* line = gray[y];
		for(int x = 0; x < w; x++)
			gray_histogram[line[x].r]++;
	}
	int mode_bin = 0, mode_count = -1;
	for(int i = 0; i < 256; i++) {
		if(gray_histogram[i] > mode_count) {
			mode_count = gray_histogram[i];
			mode_bin = i;
		}
	}
	result.background_level = mode_bin;

	// Average grayscale intensity AT the high-gradient ("sharp shape")
	// pixel locations.
	double sum = 0;
	int count = 0;
	for(int y = 1; y < h - 1; y++) {
		const RGBA* line = gray[y];
		for(int x = 1; x < w - 1; x++) {
			if(mag[y * w + x] >= mag_cutoff) {
				sum += line[x].r;
				count++;
			}
		}
	}

	if(count == 0)
		return result; // Otsu-on-magnitude found no separable edges: not confident

	result.edge_mean = sum / count;
	result.edge_pixel_count = count;
	result.confident = true;
	// Lighter-than-background sharp edges -> light text on dark background.
	result.invert = result.edge_mean > result.background_level;
	return result;
}

// Task 0274 Phase 4: see the doc comment on the declaration for validation
// numbers. Deliberately operates on the ORIGINAL (not Otsu-binarized) crop,
// since that's where the signal is direct -- a blank crop's grayscale
// values barely vary at all, whereas Otsu's own output is a forced binary
// image that always has some black/white split regardless of input.
double OcrGrayscaleStdDev(const Image& src)
{
	Image gray = OcrGrayscale(src);
	int w = gray.GetWidth();
	int h = gray.GetHeight();
	if(w <= 0 || h <= 0)
		return 0;
	double sum = 0;
	int count = 0;
	for(int y = 0; y < h; y++) {
		const RGBA* line = gray[y];
		for(int x = 0; x < w; x++) {
			sum += line[x].r;
			count++;
		}
	}
	if(count == 0)
		return 0;
	double mean = sum / count;
	double var_sum = 0;
	for(int y = 0; y < h; y++) {
		const RGBA* line = gray[y];
		for(int x = 0; x < w; x++) {
			double d = line[x].r - mean;
			var_sum += d * d;
		}
	}
	return sqrt(var_sum / count);
}

END_UPP_NAMESPACE
