#include "Camera.h"

NAMESPACE_UPP

void CameraView::SetImage(const Image& m) {
	img = m;
	Refresh();
}

void CameraView::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, Black());
	if (img.IsEmpty())
		return;
	Size isz = img.GetSize();
	Size dsz = sz;
	Size fit = GetFitSize(isz, dsz);
	Rect rc((dsz.cx - fit.cx) / 2, (dsz.cy - fit.cy) / 2, (dsz.cx + fit.cx) / 2, (dsz.cy + fit.cy) / 2);
	d.DrawImage(rc.left, rc.top, fit.cx, fit.cy, img);
	if (WhenOverlay)
		WhenOverlay(d, rc, img);
}

END_UPP_NAMESPACE
