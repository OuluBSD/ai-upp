#ifndef _Node_Core_Viewport_h_
#define _Node_Core_Viewport_h_

#include "Core.h"

namespace Upp {

namespace Node {

class Viewport : public Moveable<Viewport> {
	Pointf offset; // World coordinates of the top-left view corner
	double scale;  // View/World ratio (pixels per world unit)

public:
	Viewport() : offset(0, 0), scale(1.0) {}
	
	void   SetOffset(Pointf p) { offset = p; }
	Pointf GetOffset() const   { return offset; }
	
	void   SetScale(double s)  { scale = max(1e-6, min(1e6, s)); }
	double GetScale() const    { return scale; }
	
	// Transformations
	Pointf WorldToView(Pointf p) const { return (p - offset) * scale; }
	Pointf ViewToWorld(Pointf p) const { return p / scale + offset; }
	
	Rectf  WorldToView(const Rectf& r) const {
		return Rectf(WorldToView(r.TopLeft()), WorldToView(r.BottomRight()));
	}
	Rectf  ViewToWorld(const Rectf& r) const {
		return Rectf(ViewToWorld(r.TopLeft()), ViewToWorld(r.BottomRight()));
	}
	
	// Zoom at specific view point (keeps that point fixed in world space)
	void Zoom(double new_scale, Pointf view_focus) {
		Pointf world_focus = ViewToWorld(view_focus);
		scale = max(1e-6, min(1e6, new_scale));
		offset = world_focus - view_focus / scale;
	}
	
	void Pan(Pointf view_delta) {
		offset -= view_delta / scale;
	}
};

} // namespace Node

} // namespace Upp

#endif
