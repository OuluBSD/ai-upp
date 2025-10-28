#pragma once
#ifndef _Draw_SImageDraw_h_
#define _Draw_SImageDraw_h_

#include "Draw.h"
#include "SDraw.h"

namespace Upp {

// SImageDraw1 - Base class for software image drawing
class SImageDraw1 : public SDraw {
protected:
	ImageBuffer ib;
	friend class SImageDraw;

public:
	virtual void  PutImage(Point p, const Image& m, const Rect& src) override;
	virtual void  PutRect(const Rect& r, Color color) override;

	void Create(Size sz);
};

// SImageDraw - Software-based image drawing implementation with alpha support
class SImageDraw : public SImageDraw1 {
	SImageDraw1 alpha;
	
	bool has_alpha;

public:
	Draw& Alpha();

	operator Image() const;
	
	SImageDraw(Size sz);
	SImageDraw(int cx, int cy);
};

} // namespace Upp

#endif