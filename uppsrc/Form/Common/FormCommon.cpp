#include "FormCommon.h"

NAMESPACE_UPP

void ResolveAnchorLayout(Rect& r, dword& h_align, dword& v_align, const String& anchor, const Size& base_sz)
{
	int cx = r.Width();
	int cy = r.Height();
	int x  = r.left;
	int y  = r.top;

	if(anchor == "CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top  = y - (base_sz.cy - cy) / 2;
		h_align = Ctrl::CENTER;
		v_align = Ctrl::CENTER;
	}
	else if(anchor == "BOTTOM_CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top  = base_sz.cy - y - cy;
		h_align = Ctrl::CENTER;
		v_align = Ctrl::BOTTOM;
	}
	else if(anchor == "TOP_CENTER") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top  = y;
		h_align = Ctrl::CENTER;
		v_align = Ctrl::TOP;
	}
	else if(anchor == "CENTER_LEFT") {
		r.left = x;
		r.top  = y - (base_sz.cy - cy) / 2;
		h_align = Ctrl::LEFT;
		v_align = Ctrl::CENTER;
	}
	else if(anchor == "CENTER_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top  = y - (base_sz.cy - cy) / 2;
		h_align = Ctrl::RIGHT;
		v_align = Ctrl::CENTER;
	}
	else if(anchor == "BOTTOM_LEFT") {
		r.left = x;
		r.top  = base_sz.cy - y - cy;
		h_align = Ctrl::LEFT;
		v_align = Ctrl::BOTTOM;
	}
	else if(anchor == "BOTTOM_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top  = base_sz.cy - y - cy;
		h_align = Ctrl::RIGHT;
		v_align = Ctrl::BOTTOM;
	}
	else if(anchor == "TOP_HSIZE") {
		r.left = x;
		r.top  = y;
		h_align = Ctrl::SIZE;
		v_align = Ctrl::TOP;
	}
	else if(anchor == "CENTER_HSIZE") {
		r.left = x;
		r.top  = y - (base_sz.cy - cy) / 2;
		h_align = Ctrl::SIZE;
		v_align = Ctrl::CENTER;
	}
	else if(anchor == "BOTTOM_HSIZE") {
		r.left = x;
		r.top  = base_sz.cy - y - cy;
		h_align = Ctrl::SIZE;
		v_align = Ctrl::BOTTOM;
	}
	else if(anchor == "LEFT_VSIZE") {
		r.left = x;
		r.top  = y;
		h_align = Ctrl::LEFT;
		v_align = Ctrl::SIZE;
	}
	else if(anchor == "CENTER_VSIZE") {
		r.left = x - (base_sz.cx - cx) / 2;
		r.top  = y;
		h_align = Ctrl::CENTER;
		v_align = Ctrl::SIZE;
	}
	else if(anchor == "RIGHT_VSIZE") {
		r.left = base_sz.cx - x - cx;
		r.top  = y;
		h_align = Ctrl::RIGHT;
		v_align = Ctrl::SIZE;
	}
	else if(anchor == "SIZE") {
		r.left = x;
		r.top  = y;
		h_align = Ctrl::SIZE;
		v_align = Ctrl::SIZE;
	}
	else if(anchor == "TOP_RIGHT") {
		r.left = base_sz.cx - x - cx;
		r.top  = y;
		h_align = Ctrl::RIGHT;
		v_align = Ctrl::TOP;
	}
	else {
		r.left = x;
		r.top  = y;
		h_align = Ctrl::LEFT;
		v_align = Ctrl::TOP;
	}

	r.right  = r.left + cx;
	r.bottom = r.top  + cy;
}

END_UPP_NAMESPACE
