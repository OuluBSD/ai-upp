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
	persistent_lines.Clear();
	title = "";
	epipolar_y = -1.0;
	zoom_level = 1.0;
	zoom_center = Pointf(0.5, 0.5);
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
	double base_scale = min(sw, sh);
	
	// Apply zoom factor
	img_scale = base_scale * zoom_level;

	// Calculate zoomed image dimensions
	double zoomed_w = img.GetWidth() * img_scale;
	double zoomed_h = img.GetHeight() * img_scale;

	// Center the zoomed view around zoom_center
	// If zoom_level == 1.0, this centers the whole image
	// If zoom_level > 1.0, we shift offset to keep zoom_center at center of view (clamped)
	
	double center_x = sz.cx * 0.5;
	double center_y = sz.cy * 0.5;
	
	// Where is zoom_center in zoomed pixels relative to image top-left?
	double zx = zoom_center.x * zoomed_w;
	double zy = zoom_center.y * zoomed_h;
	
	// We want zx + img_offset.x = center_x
	img_offset.x = center_x - zx;
	img_offset.y = center_y - zy;
	
	// Optional: Clamp panning if we want to restrict view to image bounds?
	// For "fit" mode (zoom=1), we want standard centering.
	// Let's enforce that if zoom=1, we use exact centering.
	if (abs(zoom_level - 1.0) < 1e-6) {
		img_offset.x = (sz.cx - zoomed_w) / 2.0;
		img_offset.y = (sz.cy - zoomed_h) / 2.0;
	}
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

// Helper: Fits a line Ax + By + C = 0 to points and returns max error
static double FitLineError(const Vector<Pointf>& pts, Pointf& p1, Pointf& p2) {
	if (pts.GetCount() < 2) return 0;
	
	// Simple PCA / Least Squares via means
	double mx = 0, my = 0;
	for (const auto& p : pts) { mx += p.x; my += p.y; }
	mx /= pts.GetCount(); my /= pts.GetCount();
	
	double uxx = 0, uxy = 0, uyy = 0;
	for (const auto& p : pts) {
		uxx += (p.x - mx)*(p.x - mx);
		uxy += (p.x - mx)*(p.y - my);
		uyy += (p.y - my)*(p.y - my);
	}
	
	// Eigenvector of covariance matrix
	double lambda = 0.5 * (uxx + uyy + sqrt((uxx - uyy)*(uxx - uyy) + 4*uxy*uxy));
	double nx = uxy;
	double ny = lambda - uxx;
	double len = sqrt(nx*nx + ny*ny);
	if (len < 1e-9) { nx = 1; ny = 0; } else { nx /= len; ny /= len; }
	
	// Line normal is (ny, -nx) or orthogonal. Wait, PCA gives direction of max variance.
	// Direction V = (nx, ny). Normal N = (-ny, nx).
	double A = -ny;
	double B = nx;
	double C = -(A*mx + B*my);
	
	double max_err = 0;
	for (const auto& p : pts) {
		max_err = max(max_err, fabs(A*p.x + B*p.y + C));
	}
	
	// Endpoints for visualization (projected start/end)
	// We just pick min/max projection along V
	double min_t = 1e9, max_t = -1e9;
	for (const auto& p : pts) {
		double t = nx*(p.x - mx) + ny*(p.y - my);
		min_t = min(min_t, t);
		max_t = max(max_t, t);
	}
	p1 = Pointf(mx + nx*min_t, my + ny*min_t);
	p2 = Pointf(mx + nx*max_t, my + ny*max_t);
	
	return max_err;
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
	
	// Clip to control area to avoid drawing way outside bounds when zoomed
	w.Clip(0, 0, sz.cx, sz.cy);
	w.DrawImage((int)img_offset.x, (int)img_offset.y, iw, ih, img);

	// 2. Draw Epipolar Line (Horizontal)
	if (epipolar_y >= 0) {
		Point p1 = ImageToScreen(Pointf(0, epipolar_y));
		Point p2 = ImageToScreen(Pointf(img.GetWidth(), epipolar_y));
		w.DrawLine(p1, p2, 1, Color(0, 255, 255)); // Cyan line
	}

	// 3. Draw Line Annotation (point chain)
	// Draw persistent lines first
	for (const auto& chain : persistent_lines) {
		if (chain.GetCount() > 1) {
			for (int i = 0; i < chain.GetCount() - 1; i++) {
				w.DrawLine(ImageToScreen(chain[i]), ImageToScreen(chain[i+1]), 2, LtBlue());
			}
		}
		for (int i = 0; i < chain.GetCount(); i++) {
			Point p = ImageToScreen(chain[i]);
			w.DrawEllipse(p.x - 2, p.y - 2, 4, 4, LtBlue());
		}
	}

	// Draw current transient line
	if (line_points.GetCount() > 0) {
		Color line_col = LtBlue();
		if (show_curvature && line_points.GetCount() >= 3) {
			Pointf fit_p1, fit_p2;
			double err = FitLineError(line_points, fit_p1, fit_p2);
			if (err > 1.0) line_col = Color(255, (int)(255 * max(0.0, 1.0 - (err-1.0)/5.0)), 0); // Redder as error grows
			
			// Draw fitted ideal line
			w.DrawLine(ImageToScreen(fit_p1), ImageToScreen(fit_p2), 1, LtGray());
			w.DrawText(10, 30, Format("Curvature Error: %.2f px", err), StdFont().Bold(), line_col);
		}

		if (line_points.GetCount() > 1) {
			for (int i = 0; i < line_points.GetCount() - 1; i++) {
				w.DrawLine(ImageToScreen(line_points[i]), ImageToScreen(line_points[i+1]), 2, line_col);
			}
		}
		for (int i = 0; i < line_points.GetCount(); i++) {
			Point p = ImageToScreen(line_points[i]);
			w.DrawEllipse(p.x - 3, p.y - 3, 6, 6, line_col);
		}
	}

	// 4. Draw Matching Points
	for (int i = 0; i < match_points.GetCount(); i++) {
		Point p = ImageToScreen(match_points[i]);
		bool highlighted = (i == highlight_idx);
		
		Color c = highlighted ? Yellow() : Green();
		int r = highlighted ? 5 : 3;
		
		w.DrawEllipse(p.x - r, p.y - r, 2 * r, 2 * r, c);
		// Label index (1-based for user visibility)
		w.DrawText(p.x + 5, p.y + 5, AsString(i + 1), StdFont(), c);
	}
	
	w.End(); // EndClip

	// 5. Title Text (Top-Left)
	if (!title.IsEmpty()) {
		String t = title;
		if (zoom_level > 1.001) t << Format(" (Zoom: %d%%)", (int)(zoom_level * 100));
		w.DrawText(8, 8, t, StdFont().Bold(), Yellow());
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

void PreviewCtrl::RightDown(Point p, dword flags) {
	if (mode == ToolMode::LineAnnotate) {
		if (line_points.GetCount() >= 2) {
			RLOG(Format("PreviewCtrl: Finalizing line with %d points", line_points.GetCount()));
			WhenFinalizeLine(eye, line_points);
			line_points.Clear();
			Refresh();
		} else {
			// Cancel if too few points
			line_points.Clear();
			Refresh();
		}
	}
}

void PreviewCtrl::MouseMove(Point p, dword flags) {
	if (img.IsEmpty()) return;
	
	Pointf img_p = ScreenToImage(p);
	
	// Signal hover to parent (for real-time epipolar update)
	if (img_p.x >= 0 && img_p.y >= 0 && img_p.x < img.GetWidth() && img_p.y < img.GetHeight()) {
		WhenHoverPoint(img_p);
	}
	
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

void PreviewCtrl::MouseWheel(Point p, int zdelta, dword keyflags) {
	if (img.IsEmpty()) return;

	double old_zoom = zoom_level;
	
	if (zdelta > 0) zoom_level *= 1.1;
	else zoom_level /= 1.1;
	
	// Limit min zoom to 1.0 (fit), max to reasonable 20x
	if (zoom_level < 1.0) zoom_level = 1.0;
	if (zoom_level > 20.0) zoom_level = 20.0;
	
	if (abs(zoom_level - old_zoom) > 1e-6) {
		// Adjust zoom center so we zoom towards/away from mouse cursor p
		if (zoom_level > 1.001) {
			// Current mouse position in normalized image coordinates (relative to current view)
			Pointf img_p = ScreenToImage(p);
			// Clamp to image bounds
			img_p.x = Upp::min(Upp::max(img_p.x, 0.0), (double)img.GetWidth());
			img_p.y = Upp::min(Upp::max(img_p.y, 0.0), (double)img.GetHeight());
			
			Pointf norm_p(img_p.x / img.GetWidth(), img_p.y / img.GetHeight());
			
			// We want norm_p to remain at screen position p.
			// Currently: ScreenToImage(p) == img_p
			// New img_offset will be calculated in UpdateLayout using zoom_center.
			// Simplification: Set zoom_center to the normalized point under cursor.
			// This effectively centers the zoom on the cursor, but then shifts it to center screen?
			// UpdateLayout logic centers the `zoom_center`. 
			// If we want to zoom *towards* cursor, we need to adjust zoom_center slightly differently.
			// But setting zoom_center = norm_p is a good "zoom to point" behavior if we center that point.
			zoom_center = norm_p;
		} else {
			zoom_center = Pointf(0.5, 0.5);
		}
		Refresh();
	}
}

END_UPP_NAMESPACE
