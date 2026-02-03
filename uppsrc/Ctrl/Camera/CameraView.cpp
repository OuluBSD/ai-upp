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
	d.DrawImage((dsz.cx - fit.cx) / 2, (dsz.cy - fit.cy) / 2, fit.cx, fit.cy, img);
}

END_UPP_NAMESPACE
