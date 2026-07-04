#ifndef _Ctrl_Compositing_Compositing_h_
#define _Ctrl_Compositing_Compositing_h_

#include <CtrlCore/CtrlCore.h>

namespace Upp {

// Renders arbitrary `Ctrl::Paint`-shaped paint logic into its own offscreen,
// alpha-capable buffer (built on Draw/SImageDraw, not Painter), and caches
// the result as a real per-pixel-alpha Image. Standalone primitive: does not
// hook into Ctrl::Paint/CtrlDraw.cpp dispatch in any way (see
// plan/CompositeEasingNetwork/Compositing/0002).
class CompositedLayer {
	Image content;

public:
	// Paints `paint_fn` into a fresh offscreen buffer of `sz`, cleared to
	// fully transparent (RGBA 0,0,0,0) first, and stores the result as an
	// IMAGE_ALPHA Image with genuine per-pixel alpha (not just an opaque
	// snapshot) -- so later compositing sees only what `paint_fn` actually
	// touched.
	void Paint(Size sz, Function<void(Draw&)> paint_fn);

	Image GetImage() const { return content; }
};

// One layer's placement/opacity for CompositeLayers: `pos` is where the
// layer's top-left goes relative to the composite target, `alpha01` is an
// extra alpha multiplier applied to the whole layer on top of its own
// per-pixel alpha, and `layer` is the already-painted source (not owned).
struct LayerEntry : Moveable<LayerEntry> {
	Point pos;
	double alpha01 = 1.0;
	const CompositedLayer *layer = NULL;
};

// Draws `layers` onto `w`, back-to-front (layers[0] first, layers.Top()
// last), each blended using its own image's real per-pixel alpha channel,
// further multiplied by that entry's alpha01. Real alpha-over compositing
// between independently rendered layers -- entries are just a per-layer
// Image + position, so this shape does not preclude later passing each
// layer's Image through a transform or a network encoder before blending.
void CompositeLayers(Draw& w, const Vector<LayerEntry>& layers);

}

#endif
