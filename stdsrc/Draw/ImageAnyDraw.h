#pragma once
#ifndef _Draw_ImageAnyDraw_h_
#define _Draw_ImageAnyDraw_h_

#include "Draw.h"
#include <memory>

namespace Upp {

// Interface for image drawing operations
class ImageDrawInterface {
public:
	virtual ~ImageDrawInterface() {}
	virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) = 0;
};

// Image drawing with various options
class ImageAnyDraw : public Draw, public ImageDrawInterface {
	Size   size;
	Image  image;
	
public:
	virtual void DrawImageOp(int x, int y, int cx, int cy, const Image& img, const Rect& src, Color color) override;
	
	// Create an image for drawing
	void Create(Size sz);
	void Create(int cx, int cy) { Create(Size(cx, cy)); }
	
	// Get the final image
	Image GetImage() const { return image; }
	
	ImageAnyDraw() {}
	ImageAnyDraw(Size sz) { Create(sz); }
	ImageAnyDraw(int cx, int cy) { Create(cx, cy); }
};

// Helper function for drawing images with various transformations
Image RescaleImage(const Image& img, int cx, int cy);
Image RescaleImage(const Image& img, Size sz);

}

#endif