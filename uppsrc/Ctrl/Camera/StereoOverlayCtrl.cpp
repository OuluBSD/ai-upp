#include "Camera.h"

NAMESPACE_UPP

StereoOverlayCtrl::StereoOverlayCtrl() {
	view.SetLabels("Bright", "Dark");
	view.SetDrawLabel(true);
	view.WhenOverlay = THISBACK(OnOverlay);
	Add(view.SizePos());
}

void StereoOverlayCtrl::SetImages(const Image& bright, const Image& dark) {
	view.SetImages(bright, dark);
}

void StereoOverlayCtrl::SetLabels(const String& bright, const String& dark) {
	view.SetLabels(bright, dark);
}

void StereoOverlayCtrl::SetOverlay(bool is_bright, const StereoOverlayData& overlay) {
	auto CopyOverlay = [](StereoOverlayData& dst, const StereoOverlayData& src) {
		dst.left_size = src.left_size;
		dst.right_size = src.right_size;
		dst.left_points <<= src.left_points;
		dst.right_points <<= src.right_points;
		dst.match_left <<= src.match_left;
		dst.match_right <<= src.match_right;
	};
	if (is_bright) {
		CopyOverlay(bright_overlay, overlay);
		has_bright_overlay = true;
	} else {
		CopyOverlay(dark_overlay, overlay);
		has_dark_overlay = true;
	}
}

void StereoOverlayCtrl::ClearOverlay(bool is_bright) {
	if (is_bright) {
		bright_overlay.Clear();
		has_bright_overlay = false;
	} else {
		dark_overlay.Clear();
		has_dark_overlay = false;
	}
}

void StereoOverlayCtrl::SetStats(bool is_bright, const StereoTrackerStatsData& stats) {
	if (is_bright)
		bright_stats = stats;
	else
		dark_stats = stats;
}

void StereoOverlayCtrl::SetShowSplitView(bool b) {
	show_split_view = b;
	view.SetSplitView(b);
}

void StereoOverlayCtrl::DrawOverlay(Draw& w, const Rect& r, const Image& img, const StereoOverlayData& overlay,
                                    const StereoTrackerStatsData& stats, const char* label,
                                    const Color& point_color, const Color& match_color) const {
	if (img.IsEmpty())
		return;
	Size src = img.GetSize();
	if (src.cx <= 0 || src.cy <= 0)
		return;
	if (!show_descriptors && !show_match_lines && !show_stats_overlay)
		return;

	Size dst_sz = r.GetSize();
	double sx = (double)dst_sz.cx / (double)src.cx;
	double sy = (double)dst_sz.cy / (double)src.cy;
	int right_offset = 0;
	if (overlay.left_size.cx > 0 && src.cx == overlay.left_size.cx * 2)
		right_offset = overlay.left_size.cx;

	auto MapPoint = [&](const vec2& p, int offset_x) -> Pointf {
		return Pointf((double)r.left + (p[0] + offset_x) * sx,
		              (double)r.top + p[1] * sy);
	};

	const int max_points = 1536;
	if (show_descriptors) {
		int left_count = overlay.left_points.GetCount();
		if (left_count > max_points)
			left_count = max_points;
		for (int i = 0; i < left_count; i++) {
			Pointf pt = MapPoint(overlay.left_points[i], 0);
			w.DrawRect((int)pt.x - 1, (int)pt.y - 1, 3, 3, point_color);
			if (show_descriptor_ids)
				w.DrawText((int)pt.x + 2, (int)pt.y + 2, IntStr(i), Arial(9), point_color);
		}
		int right_count = overlay.right_points.GetCount();
		if (right_count > max_points)
			right_count = max_points;
		for (int i = 0; i < right_count; i++) {
			Pointf pt = MapPoint(overlay.right_points[i], right_offset);
			w.DrawRect((int)pt.x - 1, (int)pt.y - 1, 3, 3, point_color);
			if (show_descriptor_ids)
				w.DrawText((int)pt.x + 2, (int)pt.y + 2, IntStr(i), Arial(9), point_color);
		}
	}

	if (show_match_lines) {
		int match_count = overlay.match_left.GetCount();
		if (match_count > overlay.match_right.GetCount())
			match_count = overlay.match_right.GetCount();
		if (match_count > max_points)
			match_count = max_points;
		for (int i = 0; i < match_count; i++) {
			Pointf a = MapPoint(overlay.match_left[i], 0);
			Pointf b = MapPoint(overlay.match_right[i], right_offset);
			w.DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, 1, match_color);
			if (show_match_ids) {
				Pointf mid((a.x + b.x) * 0.5, (a.y + b.y) * 0.5);
				w.DrawText((int)mid.x + 2, (int)mid.y + 2, IntStr(i), Arial(9), match_color);
			}
		}
	}

	if (show_stats_overlay) {
		int match_count = overlay.match_left.GetCount();
		if (match_count > overlay.match_right.GetCount())
			match_count = overlay.match_right.GetCount();
		String line1 = Format("%s: frames=%d, kp=%d/%d, points=%d",
			label, stats.processed_frames, stats.last_left_keypoints,
			stats.last_right_keypoints, stats.last_tracked_points);
		String line2 = Format("tri=%d, matches=%d, usecs=%d, pose=%s",
			stats.last_tracked_triangles, match_count, stats.last_process_usecs,
			stats.has_pose ? "yes" : "no");
		w.DrawText(r.left + 6, r.top + 6, line1, Arial(12).Bold(), White());
		w.DrawText(r.left + 6, r.top + 22, line2, Arial(11), White());
	}
}

void StereoOverlayCtrl::OnOverlay(Draw& w, const Rect& r, const Image& img, int channel) {
	if (channel == 0) {
		if (has_bright_overlay)
			DrawOverlay(w, r, img, bright_overlay, bright_stats, "Bright", LtYellow(), LtGreen());
	} else {
		if (has_dark_overlay)
			DrawOverlay(w, r, img, dark_overlay, dark_stats, "Dark", LtBlue(), LtGreen());
	}
}

END_UPP_NAMESPACE
