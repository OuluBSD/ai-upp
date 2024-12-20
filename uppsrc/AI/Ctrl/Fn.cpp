#include "Ctrl.h"


NAMESPACE_UPP


void SetColoredListValue(ArrayCtrl& list, int row, int col, const String& txt, Color clr, bool blend) {
	if (blend) {
		list.Set(row, col, AttrText(txt)
			.NormalPaper(Blend(clr, White(), 128+64)).NormalInk(Black())
			.Paper(Blend(clr, GrayColor())).Ink(White())
		);
	}
	else {
		list.Set(row, col, AttrText(txt)
			.NormalPaper(clr).NormalInk(Black())
			.Paper(Blend(clr, GrayColor(64))).Ink(White())
		);
	}
}

Image RescaleToFit(const Image& img, int smaller_side_length) {
	Size orig_sz = img.GetSize();
	double orig_small = max(orig_sz.cx, orig_sz.cy);
	double ratio = (double)smaller_side_length / orig_small;
	Size new_sz = orig_sz * ratio;
	Image scaled_img = CachedRescale(img, new_sz, FILTER_BILINEAR);
	return scaled_img;
}

String CacheImageFile(String dir, hash_t h) {
	String filename = MD5String(&h, sizeof(h)) + ".jpg";
	String cache_dir = AppendFileName(dir, "image-cache");
	String cache_path = AppendFileName(cache_dir, filename);
	return cache_path;
}

String ThumbnailImageFile(String dir, hash_t h) {
	String filename = MD5String(&h, sizeof(h)) + ".jpg";
	String thumb_dir = AppendFileName(dir, "images" DIR_SEPS "thumbnails");
	String thumb_path = AppendFileName(thumb_dir, filename);
	return thumb_path;
}

String FullImageFile(String dir, hash_t h) {
	String filename = MD5String(&h, sizeof(h)) + ".jpg";
	String thumb_dir = AppendFileName(dir, "images" DIR_SEPS "full");
	String thumb_path = AppendFileName(thumb_dir, filename);
	return thumb_path;
}


END_UPP_NAMESPACE
