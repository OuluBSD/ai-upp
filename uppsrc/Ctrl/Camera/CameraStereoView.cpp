#include "Camera.h"

NAMESPACE_UPP

void CameraStereoView::SetImages(const Image& a, const Image& b) {
	img_a = a;
	img_b = b;
	Refresh();
}

void CameraStereoView::SetSplitView(bool b) {
	split_view = b;
	Refresh();
}

void CameraStereoView::SetShowMissingText(bool b) {
	show_missing_text = b;
	Refresh();
}

void CameraStereoView::SetLabels(const String& a, const String& b) {
	label_a = a;
	label_b = b;
	Refresh();
}

void CameraStereoView::DrawImage(Draw& d, const Rect& r, const Image& img) {
	if (img.IsEmpty())
		return;
	Size isz = img.GetSize();
	Size dsz = r.GetSize();
	Size fit = GetFitSize(isz, dsz);
	int x = r.left + (dsz.cx - fit.cx) / 2;
	int y = r.top + (dsz.cy - fit.cy) / 2;
	d.DrawImage(x, y, fit.cx, fit.cy, img);
}

void CameraStereoView::Paint(Draw& d) {
	Size sz = GetSize();
	d.DrawRect(sz, Black());
	int h = split_view ? sz.cy / 2 : sz.cy;
	Rect rc_a = RectC(0, 0, sz.cx, h);
	Rect rc_b = split_view ? RectC(0, h, sz.cx, h) : Rect();

	if (split_view) {
		DrawImage(d, rc_a, img_a);
		DrawImage(d, rc_b, img_b);

		if (WhenOverlay && !img_a.IsEmpty())
			WhenOverlay(d, rc_a, img_a, 0);
		if (WhenOverlay && !img_b.IsEmpty())
			WhenOverlay(d, rc_b, img_b, 1);

		if (draw_label) {
			if (!label_a.IsEmpty())
				d.DrawText(rc_a.left + 6, rc_a.top + 6, label_a, Arial(12).Bold(), White());
			if (!label_b.IsEmpty())
				d.DrawText(rc_b.left + 6, rc_b.top + 6, label_b, Arial(12).Bold(), White());
		}
	}
	else {
		if (!img_a.IsEmpty()) {
			DrawImage(d, rc_a, img_a);
			if (WhenOverlay)
				WhenOverlay(d, rc_a, img_a, 0);
			if (draw_label && !label_a.IsEmpty())
				d.DrawText(rc_a.left + 6, rc_a.top + 6, label_a, Arial(12).Bold(), White());
		}
		else if (!img_b.IsEmpty()) {
			DrawImage(d, rc_a, img_b);
			if (WhenOverlay)
				WhenOverlay(d, rc_a, img_b, 1);
			if (draw_label && !label_b.IsEmpty())
				d.DrawText(rc_a.left + 6, rc_a.top + 6, label_b, Arial(12).Bold(), White());
		}
	}

	if (show_missing_text && img_a.IsEmpty() && img_b.IsEmpty())
		d.DrawText(10, 10, "No camera images", Arial(20).Bold(), White());
}

END_UPP_NAMESPACE
