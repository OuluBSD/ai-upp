#include "Local.h"

NAMESPACE_TOPSIDE_BEGIN


Image DefaultImages::Arrow;


Image GetDefaultImage(int code) {
	static Image cube;
	
	switch (code) {
		case IMAGEID_GRID: return RealizeImage(cube, ShareDirFile("models" DIR_SEPS "cube.png"));
	}
	return Image();
}

Image RealizeImage(Image& img, String path) {
	if (img.IsEmpty())
		img = StreamRaster::LoadFileAny(path);
	return img;
}

void SetCenterHotSpot(Image& img) {
	Size sz = img.GetSize();
	if (!sz.IsEmpty()) {
		Point hs(sz.cx / 2, sz.cy / 2);
		#if IS_UPP_CORE
		ImageBuffer ib(img);
		ib.SetHotSpot(hs);
		img = ib;
		#else
		img.SetHotSpot(hs);
		#endif
	}
}




NAMESPACE_TOPSIDE_END
