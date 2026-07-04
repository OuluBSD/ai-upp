#include "Compositing.h"

namespace Upp {

void CompositedLayer::Paint(Size sz, Function<void(Draw&)> paint_fn)
{
	// SImageDraw's own operator Image() only produces real per-pixel alpha
	// if its separate Alpha() grayscale sub-channel was painted too --
	// otherwise it force-sets alpha=255 for every pixel (fully opaque),
	// discarding whatever alpha ended up in the main pixel buffer. Since
	// our Function<void(Draw&)> paint_fn only gets one Draw& (no separate
	// alpha channel to paint into), we bypass that and read SImageDraw1's
	// own pixel buffer directly instead: PutRect always writes fully
	// opaque RGBA (a=255) for whatever shapes get drawn, and PutImage
	// alpha-composites ("Over") any pasted image on top of whatever is
	// already there -- which, starting from an all-zero destination, just
	// reproduces the pasted image's own real per-pixel (premultiplied)
	// alpha exactly. So once the buffer is cleared to zero, painting
	// leaves it as correct premultiplied-alpha content: opaque where
	// paint_fn drew solid shapes, transparent where it drew nothing, and
	// correctly partial where it drew a partially-transparent image.
	//
	// NOTE: ImageBuffer::Create() does NOT reliably zero the buffer --
	// in debug builds it deliberately poisons freshly allocated pixels
	// with an alternating (a=0/255, r=0/255) sentinel pattern (see
	// ImageBuffer::Create, uppsrc/Draw/Image.cpp) specifically to catch
	// code that wrongly assumes zero-init, exactly like this. So this
	// buffer must be cleared to RGBAZero() explicitly before painting,
	// not left to Create()'s allocation.
	struct RawImageDraw : SImageDraw1 {
		using SImageDraw1::ib; // re-expose the protected pixel buffer
	};

	RawImageDraw iw;
	iw.Create(sz);
	Upp::Fill(iw.ib.Begin(), RGBAZero(), iw.ib.GetLength());

	if(paint_fn)
		paint_fn(iw);

	ImageBuffer b(sz);
	memcpy(b, iw.ib.Begin(), sizeof(RGBA) * iw.ib.GetLength());
	b.SetKind(IMAGE_ALPHA);
	content = Image(b);
}

// Scales a whole (already premultiplied-alpha) layer image by an extra
// uniform alpha multiplier. For premultiplied storage, scaling every
// channel (including alpha) by the same factor is exact: if S is the
// "true" straight color and a0 the old alpha, the stored premultiplied
// color is S*a0; multiplying the whole pixel by f gives S*a0*f, which is
// exactly the premultiplied representation of the new alpha a0*f. So this
// does not need to unmultiply/re-premultiply around the scale.
static Image ScaleLayerAlpha(Image src, double alpha01)
{
	int mul = (int)(clamp(alpha01, 0.0, 1.0) * 255.0 + 0.5);
	if(mul >= 255)
		return src;
	ImageBuffer ib(src);
	for(RGBA& p : ib) {
		p.r = byte((int(p.r) * mul) / 255);
		p.g = byte((int(p.g) * mul) / 255);
		p.b = byte((int(p.b) * mul) / 255);
		p.a = byte((int(p.a) * mul) / 255);
	}
	ib.SetKind(IMAGE_ALPHA);
	return Image(ib);
}

void CompositeLayers(Draw& w, const Vector<LayerEntry>& layers)
{
	for(int i = 0; i < layers.GetCount(); i++) {
		const LayerEntry& e = layers[i];
		if(!e.layer)
			continue;
		Image img = e.layer->GetImage();
		if(img.IsEmpty() || e.alpha01 <= 0.0)
			continue;
		if(e.alpha01 < 0.999)
			img = ScaleLayerAlpha(img, e.alpha01);
		w.DrawImage(e.pos.x, e.pos.y, img);
	}
}

}
