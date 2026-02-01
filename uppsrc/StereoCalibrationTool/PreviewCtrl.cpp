#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
PreviewCtrl.cpp
===============
Implements a clean, documented image viewer with overlays for point matching.
- Coordinate conversions account for scale-to-fit offsets.
- Overlays are rendered in image-space but mapped to screen-space.
*/

PreviewCtrl::PreviewCtrl() {
	SetFrame(ViewFrame());
	img_offset = Pointf(0, 0);
	BackPaint(); // Reduce flickering
}

void PreviewCtrl::SetImage(const Image& _img) {
	img = _img;
	Refresh();
}

void PreviewCtrl::SetMatchingPoints(const Vector<Pointf>& pts) {
	match_points <<= pts;
	Refresh();
}

void PreviewCtrl::Clear() {
	img = Image();
	match_points.Clear();
	line_points.Clear();
	title = "";
	Refresh();
}

// Internal: Calculates scale and offset to fit image in control while preserving aspect ratio.
void PreviewCtrl::UpdateLayout() {
	Size sz = GetSize();
	if (img.IsEmpty() || sz.cx <= 0 || sz.cy <= 0) {
		img_scale = 1.0;
		img_offset = Pointf(0, 0);
		return;
	}

	double sw = (double)sz.cx / img.GetWidth();
	double sh = (double)sz.cy / img.GetHeight();
	img_scale = min(sw, sh);

	// Center image in the remaining space
	img_offset.x = (sz.cx - img.GetWidth() * img_scale) / 2.0;
	img_offset.y = (sz.cy - img.GetHeight() * img_scale) / 2.0;
}

// Public: Maps screen pixel coordinate to image pixel coordinate.
// Accounts for centering offset and scale.
Pointf PreviewCtrl::ScreenToImage(Point p) const {
	if (img_scale < 1e-9) return Pointf(-1, -1);
	return Pointf(
		(float)((p.x - img_offset.x) / img_scale),
		(float)((p.y - img_offset.y) / img_scale)
	);
}

// Public: Maps image pixel coordinate to screen pixel coordinate.
// Accounts for centering offset and scale.
Point PreviewCtrl::ImageToScreen(Pointf p) const {
	return Point(
		(int)(p.x * img_scale + img_offset.x),
		(int)(p.y * img_scale + img_offset.y)
	);
}

void PreviewCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	w.DrawRect(sz, SColorFace()); // Fill background

	if (img.IsEmpty()) {
		w.DrawText(10, 10, "No Image", StdFont().Bold(), SColorText());
		return;
	}

	// Update cached mapping parameters
	// NOTE: Calling this from Paint is acceptable for a preview control layout.
	const_cast<PreviewCtrl*>(this)->UpdateLayout();

	// 1. Draw scaled image
	int iw = (int)(img.GetWidth() * img_scale);
	int ih = (int)(img.GetHeight() * img_scale);
	w.DrawImage((int)img_offset.x, (int)img_offset.y, iw, ih, img);

	// 2. Draw Line Annotation (point chain)
	if (line_points.GetCount() > 1) {
		for (int i = 0; i < line_points.GetCount() - 1; i++) {
			w.DrawLine(ImageToScreen(line_points[i]), ImageToScreen(line_points[i+1]), 2, LtBlue());
		}
	}
	for (int i = 0; i < line_points.GetCount(); i++) {
		Point p = ImageToScreen(line_points[i]);
		w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, LtBlue());
	}

	// 3. Draw Matching Points
	for (int i = 0; i < match_points.GetCount(); i++) {
		Point p = ImageToScreen(match_points[i]);
		bool highlighted = (i == highlight_idx);
		
		Color c = highlighted ? Yellow() : Green();
		int r = highlighted ? 5 : 3;
		
		w.DrawEllipse(p.x - r, p.y - r, 2 * r, 2 * r, c);
		// Label index (1-based for user visibility)
		w.DrawText(p.x + 5, p.y + 5, AsString(i + 1), StdFont(), c);
	}

	// 4. Title Text (Top-Left)
	if (!title.IsEmpty()) {
		w.DrawText(8, 8, title, StdFont().Bold(), Yellow());
	}
}

void PreviewCtrl::LeftDown(Point p, dword flags) {
	if (img.IsEmpty()) return;
	
	Pointf img_p = ScreenToImage(p);
	
	// Verify click is strictly inside image boundaries
	if (img_p.x < 0 || img_p.y < 0 || img_p.x >= img.GetWidth() || img_p.y >= img.GetHeight())
		return;

	if (mode == ToolMode::LineAnnotate) {
		line_points.Add(img_p);
		Refresh();
	} else if (mode == ToolMode::PickMatch) {
		// Forward click to StageA logic
		WhenClickPoint(eye, img_p);
	}
}

void PreviewCtrl::MouseMove(Point p, dword flags) {
	if (img.IsEmpty()) return;
	
	Pointf img_p = ScreenToImage(p);
	
	// Simple hover highlight: find nearest point within threshold
	int best_idx = -1;
	double best_dist = 10.0 / img_scale; // 10 pixels radius on screen
	
	for (int i = 0; i < match_points.GetCount(); i++) {
		double d = Distance(img_p, match_points[i]);
		if (d < best_dist) {
			best_dist = d;
			best_idx = i;
		}
	}
	
	if (best_idx != highlight_idx) {
		highlight_idx = best_idx;
		Refresh();
	}
}

END_UPP_NAMESPACE
