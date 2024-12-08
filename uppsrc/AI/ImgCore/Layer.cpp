#include "ImgCore.h"


NAMESPACE_UPP

String ImageLayer::StoreString() {
	PNGEncoder enc;
	String png = enc.SaveString(img);
	String bz_enc = BZ2Compress(png);
	return bz_enc;
}

void ImageLayer::LoadString(const String& bz_enc) {
	if (bz_enc.IsEmpty())
		img.Clear();
	else {
		String png = BZ2Decompress(bz_enc);
		PNGRaster dec;
		img = dec.LoadStringAny(png);
	}
}

void ImageLayer::Serialize(Stream& s) {
	int v = 1; s % v;
	if (v >= 1) {
		if (s.IsStoring()) {
			String bz_enc = StoreString();
			s % bz_enc;
		}
		else {
			String bz_enc;
			s % bz_enc;
			LoadString(bz_enc);
		}
	}
}
void ImageLayer::Jsonize(JsonIO& json) {
	if (json.IsStoring()) {
		String bz_png = StoreString();
		json("bz_png", bz_png);
	}
	else {
		String bz_png;
		json("bz_png", bz_png);
		LoadString(bz_png);
	}
}
hash_t ImageLayer::GetHashValue() const {
	CombineHash c;
	c.Do(img);
	return c;
}

INITIALIZER_COMPONENT(ImageLayer);
INITIALIZER_COMPONENT(ImageGenLayer);

END_UPP_NAMESPACE
