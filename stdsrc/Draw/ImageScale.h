#pragma once
#ifndef _Draw_ImageScale_h_
#define _Draw_ImageScale_h_

#include "Draw.h"
#include <vector>

namespace Upp {

// Image scaling algorithms and operations
enum ImageScaleMethod {
	SCALE_NEAREST,
	SCALE_BILINEAR,
	SCALE_BICUBIC
};

// Scale an image to a new size using various algorithms
Image ScaleImage(const Image& img, int cx, int cy, ImageScaleMethod method = SCALE_BILINEAR);
Image ScaleImage(const Image& img, Size sz, ImageScaleMethod method = SCALE_BILINEAR);

// Scale to fit within a specific size while preserving aspect ratio
Image ScaleImageFit(const Image& img, int cx, int cy, ImageScaleMethod method = SCALE_BILINEAR);
Image ScaleImageFit(const Image& img, Size sz, ImageScaleMethod method = SCALE_BILINEAR);

// Scale to fill a specific size while preserving aspect ratio (may crop)
Image ScaleImageFill(const Image& img, int cx, int cy, ImageScaleMethod method = SCALE_BILINEAR);
Image ScaleImageFill(const Image& img, Size sz, ImageScaleMethod method = SCALE_BILINEAR);

// Scale with a specific zoom factor
Image ZoomImage(const Image& img, double zoom, ImageScaleMethod method = SCALE_BILINEAR);

// Rotate and scale operations
Image RotateImage(const Image& img, double angle);  // angle in degrees

// Resampling utilities
Image ResampleImage(const Image& img, int cx, int cy, ImageScaleMethod method = SCALE_BILINEAR);
Image ResampleImage(const Image& img, Size sz, ImageScaleMethod method = SCALE_BILINEAR);

}

#endif