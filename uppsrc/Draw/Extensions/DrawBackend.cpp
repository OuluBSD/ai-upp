#include "Extensions.h"


NAMESPACE_UPP

RawSysImage::operator Image() const {
	TODO // Not used - use ByteImage directly instead
	return Image();
}

RawSysImage::operator ByteImage() const {
	ByteImage img;
	if (!data.IsEmpty() && w > 0 && h > 0) {
		LOG("RawSysImage::operator ByteImage: converting size: " << w << "x" << h << " ch: " << ch << " pitch: " << pitch << " data.GetCount(): " << data.GetCount());
		// TGA reader returns ARGB format, convert to RGBA for ByteImage
		img.Set(w, h, ch, pitch, data.Begin());
		LOG("RawSysImage::operator ByteImage: after Set, ByteImage isEmpty=" << img.IsEmpty() << " GetWidth=" << img.GetWidth() << " GetHeight=" << img.GetHeight());
	}
	else {
		LOG("RawSysImage::operator ByteImage: ERROR: data.IsEmpty()=" << data.IsEmpty() << " w=" << w << " h=" << h);
	}
	return img;
}

END_UPP_NAMESPACE

