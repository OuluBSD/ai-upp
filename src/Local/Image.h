#ifndef _Local_Image_h_
#define _Local_Image_h_

NAMESPACE_TOPSIDE_BEGIN

class Ether;


enum {
	IMAGEID_GRID,
	
	IMAGEID_COUNT
};

Image GetDefaultImage(int code);
Image RealizeImage(Image& img, String path);
void SetCenterHotSpot(Image& img);

struct DefaultImages {
	static Image Arrow;
};


NAMESPACE_TOPSIDE_END

#endif
