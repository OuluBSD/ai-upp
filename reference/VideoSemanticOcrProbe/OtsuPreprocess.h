#ifndef _VideoSemanticOcrProbe_OtsuPreprocess_h_
#define _VideoSemanticOcrProbe_OtsuPreprocess_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

NAMESPACE_UPP

// ---------------------------------------------------------------------------
// Task 0272 Phase 1: Otsu binarization, ported from the proven
// ConvNetCpp/src/OCR implementation:
//   - OtsuThreshold():   ConvNetCpp/src/OCR/ImageUtils.cpp:231-273
//     (classic between-class-variance maximization over a 256-bin grayscale
//     histogram; ported verbatim, only the grayscale-conversion helper it
//     depends on was swapped for U++'s own Upp::Grayscale() to avoid
//     dragging in ConvNetCpp's separate GetGrayValue()/GrayscaleOCR()).
//   - IMAGE_PREPROCESS_OTSU / IMAGE_PREPROCESS_OTSU_INVERSE branches:
//     ConvNetCpp/src/OCR/Preprocessing.cpp:39-48 (3x upscale, grayscale,
//     Binarize(gray, OtsuThreshold(gray)), optionally inverted).
//
// Integration point decision: this lives in reference/VideoSemanticOcrProbe
// (not uppsrc/VisualStateModel/OcrLayer.h/.cpp) because OcrLayer.h/.cpp is
// purely the rule/comparison data model (VsmOcrRule, VsmOcrExecutor,
// VsmOcrEngine) — it has no image-manipulation code today. All the actual
// pixel preprocessing (GrayscaleImage, RescaleFilter, PreprocessCrop) already
// lives in VideoSemanticOcrProbe/main.cpp, so this is the existing OCR-
// pixel-processing home in ai-upp; adding a sibling .h/.cpp pair here keeps
// the port local to where it's used and avoids inventing a new dependency
// from OcrLayer onto Draw's Image type.

Image  OcrGrayscale(const Image& src);
Image  OcrInvert(const Image& src);
Image  OcrBinarize(const Image& gray, int threshold);
int    OcrOtsuThreshold(const Image& gray);

// Full Otsu preprocessing pipeline: 3x upscale (bilinear) + grayscale +
// Otsu-binarize, optionally inverted afterward.
Image OcrPreprocessOtsu(const Image& src, bool invert);

// ---------------------------------------------------------------------------
// Task 0272 Phase 2: convolution/edge-based predictive polarity detector.
// New algorithm, does not exist anywhere else in either codebase.
//
// Method (Sobel gradient magnitude, chosen over Laplacian because it gives
// separate horizontal/longitudinal gradient components that combine into a
// magnitude which is directly comparable to a single Otsu-style threshold,
// and because it is the standard first choice for "find the sharp edges of
// character strokes" in classic OCR preprocessing literature):
//   1. Grayscale the crop.
//   2. Run a 3x3 Sobel kernel (Gx, Gy) over interior pixels, magnitude =
//      sqrt(Gx^2 + Gy^2).
//   3. Treat the gradient-magnitude values themselves as a 256-bin
//      histogram (scaled to the crop's own observed magnitude range) and
//      re-use the same Otsu variance-maximization routine to split
//      "sharp shape" (high-gradient / character-stroke-edge) pixels from
//      flat background/noise pixels — this makes the edge/non-edge cut
//      self-calibrating per crop instead of a fixed magic threshold.
//   4. Compare the average grayscale intensity AT those edge-pixel
//      locations against the crop's dominant (histogram-mode) background
//      intensity. Edges lighter than background -> light-text-on-dark ->
//      recommend IMAGE_PREPROCESS_OTSU_INVERSE. Edges darker than
//      background -> dark-text-on-light -> recommend plain
//      IMAGE_PREPROCESS_OTSU.
struct OcrPolarityResult : Moveable<OcrPolarityResult> {
	bool   invert           = false; // true => use OcrPreprocessOtsu(src, true)
	bool   confident         = false; // false when no edge pixels were found (degenerate/blank crop)
	double edge_mean         = 0;    // average grayscale intensity at edge pixel locations
	double background_level  = 0;    // dominant (mode) grayscale intensity of the whole crop
	int    edge_pixel_count  = 0;
};

OcrPolarityResult OcrDetectPolarity(const Image& gray);

// ---------------------------------------------------------------------------
// Task 0274 Phase 4: blank/uniform-color crop detector. A crop with no text
// at all (just plain felt background) is nearly a single flat color; Otsu's
// histogram-based thresholding still finds SOME split point even on that
// tiny natural variation (JPEG blocking noise, a soft gradient) and produces
// a noisy-looking binarized image that OCR then hallucinates garbage text
// on, rather than legitimately failing. Grayscale standard deviation over
// the whole (original, pre-Otsu) crop is a direct, cheap signal: measured
// across this project's real 31-crop validation set, the one genuinely
// blank crop (hand2 frame42 seat_bet_label -- no bet placed, plain green
// felt) has stddev ~4.4, while every one of the other 30 crops (all of
// which have real visible text) has stddev >= 16.08 -- a threshold of 10
// cleanly separates them with no false positives on that set.
double OcrGrayscaleStdDev(const Image& src);

END_UPP_NAMESPACE

#endif
