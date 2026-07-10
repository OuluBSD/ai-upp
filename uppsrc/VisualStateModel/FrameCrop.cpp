#include "VisualStateModel.h"
#include <plugin/png/png.h>

namespace Upp {

Image VsmFrameImageToImage(const VsmFrameImage& frame)
{
	int w = frame.width, h = frame.height;
	ImageBuffer buf(w, h);
	for(int y = 0; y < h; y++) {
		RGBA* row = buf[y];
		for(int x = 0; x < w; x++) {
			byte r, g, b, a;
			frame.GetPixel(x, y, r, g, b, a);
			row[x].r = r; row[x].g = g; row[x].b = b; row[x].a = a;
		}
	}
	return Image(buf);
}

bool VsmSaveRegionCropPng(const VsmFrameImage& frame, const VsmChangedRect& region,
                          const String& path)
{
	if(frame.IsEmpty())
		return false;

	Image frame_img = VsmFrameImageToImage(frame);

	int x0 = max(0, region.x - kCropPadding);
	int y0 = max(0, region.y - kCropPadding);
	int x1 = min(frame.width,  region.x + region.w + kCropPadding);
	int y1 = min(frame.height, region.y + region.h + kCropPadding);
	int cw = max(1, x1 - x0);
	int ch = max(1, y1 - y0);

	Image out = Crop(frame_img, x0, y0, cw, ch);

	RealizeDirectory(GetFileFolder(path));
	return PNGEncoder().SaveFile(path, out);
}

} // namespace Upp
