#include "StereoCalibrationTool.h"

#undef CPU_SSE2
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

NAMESPACE_UPP

// Helper: Convert U++ Image to OpenCV Gray Mat
static cv::Mat ImageToGrayMat(const Image& img) {
	if(img.IsEmpty()) return cv::Mat();
	Size sz = img.GetSize();
	cv::Mat m(sz.cy, sz.cx, CV_8UC1);
	for(int y=0; y<sz.cy; y++) {
		const RGBA* s = img[y];
		uint8_t* d = m.ptr<uint8_t>(y);
		for(int x=0; x<sz.cx; x++) {
			// Simple grayscale: 0.299R + 0.587G + 0.114B
			d[x] = (uint8_t)((s[x].r * 77 + s[x].g * 150 + s[x].b * 29) >> 8);
		}
	}
	return m;
}

// Helper: Convert U++ Image to OpenCV BGR Mat (for color operations like remap)
static cv::Mat ImageToBGRMat(const Image& img) {
	if(img.IsEmpty()) return cv::Mat();
	Size sz = img.GetSize();
	cv::Mat m(sz.cy, sz.cx, CV_8UC3);
	for(int y=0; y<sz.cy; y++) {
		const RGBA* s = img[y];
		cv::Vec3b* d = m.ptr<cv::Vec3b>(y);
		for(int x=0; x<sz.cx; x++) {
			// OpenCV uses BGR order
			d[x][0] = s[x].b;
			d[x][1] = s[x].g;
			d[x][2] = s[x].r;
		}
	}
	return m;
}

// Helper: Convert OpenCV BGR Mat to U++ Image
static Image MatToImage(const cv::Mat& mat) {
	if(mat.empty()) return Image();
	if(mat.type() != CV_8UC3) return Image();

	ImageBuffer out(mat.cols, mat.rows);
	for(int y=0; y<mat.rows; y++) {
		const cv::Vec3b* s = mat.ptr<cv::Vec3b>(y);
		RGBA* d = out[y];
		for(int x=0; x<mat.cols; x++) {
			// OpenCV uses BGR order, convert to RGBA
			d[x].r = s[x][2];
			d[x].g = s[x][1];
			d[x].b = s[x][0];
			d[x].a = 255;
		}
	}
	return out;
}

/*
StageA.cpp
==========
Purpose:
- Stage A UI and basic preview/match picking.
- Owns its own plotters (left/right) and does not reuse Camera/LiveResult previews.
 - Does NOT start/stop the camera or capture new frames.

Data flow:
- Reads/writes AppModel.project_state.
- Reads AppModel.captured_frames for preview.

Gotchas / invariants:
- Match points are stored normalized (0..1) relative to raw images.
- Identity expected when yaw/pitch/roll are all zero.
*/

bool IsValidAnglePoly(const vec4& poly) {
	return fabs(poly.data[0]) > 1e-9;
}

bool IsSamePoly(const vec4& a, const vec4& b) {
	for (int i = 0; i < 4; i++) {
		if (fabs(a.data[i] - b.data[i]) > 1e-6)
			return false;
	}
	return true;
}

// Tint image blue (reduce R and G channels) - view-only for overlay
Image TintBlue(const Image& src) {
	if (src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer out(sz);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			d[x].r = (byte)(s[x].r * 0.25);
			d[x].g = (byte)(s[x].g * 0.25);
			d[x].b = s[x].b;
			d[x].a = s[x].a;
		}
	}
	return out;
}

// Tint image red (reduce G and B channels) - view-only for overlay
Image TintRed(const Image& src) {
	if (src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer out(sz);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			d[x].r = s[x].r;
			d[x].g = (byte)(s[x].g * 0.25);
			d[x].b = (byte)(s[x].b * 0.25);
			d[x].a = s[x].a;
		}
	}
	return out;
}

// Alpha blend two images: out = base*(1-alpha) + top*alpha
Image AlphaBlend(const Image& base, const Image& top, float alpha) {
	if (base.IsEmpty())
		return top;
	if (top.IsEmpty())
		return base;
	Size sz = base.GetSize();
	if (top.GetSize() != sz)
		return base; // Size mismatch, return base

	alpha = alpha < 0.0f ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
	float inv_alpha = 1.0f - alpha;

	ImageBuffer out(sz);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* b = base[y];
		const RGBA* t = top[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			d[x].r = (byte)(b[x].r * inv_alpha + t[x].r * alpha);
			d[x].g = (byte)(b[x].g * inv_alpha + t[x].g * alpha);
			d[x].b = (byte)(b[x].b * inv_alpha + t[x].b * alpha);
			d[x].a = 255;
		}
	}
	return out;
}

// Compute difference image: diff = abs(left - right) per pixel
Image ComputeDiff(const Image& left, const Image& right) {
	if (left.IsEmpty() || right.IsEmpty())
		return Image();
	Size sz = left.GetSize();
	if (right.GetSize() != sz)
		return left; // Size mismatch

	ImageBuffer out(sz);
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* l = left[y];
		const RGBA* r = right[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			int diff_r = abs((int)l[x].r - (int)r[x].r);
			int diff_g = abs((int)l[x].g - (int)r[x].g);
			int diff_b = abs((int)l[x].b - (int)r[x].b);
			// Use max diff across channels for visibility
			int max_diff = max(max(diff_r, diff_g), diff_b);
			// Amplify for visibility (2x gain)
			max_diff = min(255, max_diff * 2);
			d[x].r = d[x].g = d[x].b = (byte)max_diff;
			d[x].a = 255;
		}
	}
	return out;
}

// Draw red crosshair (1px lines through center) - view-only
Image DrawCrosshair(const Image& src) {
	if (src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer out(sz);

	// Copy source image
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			d[x] = s[x];
		}
	}

	int cx = sz.cx / 2;
	int cy = sz.cy / 2;

	RGBA red;
	red.r = 255;
	red.g = 0;
	red.b = 0;
	red.a = 255;

	// Vertical line
	for (int y = 0; y < sz.cy; y++) {
		if (cx >= 0 && cx < sz.cx) {
			out[y][cx] = red;
		}
	}

	// Horizontal line
	if (cy >= 0 && cy < sz.cy) {
		for (int x = 0; x < sz.cx; x++) {
			out[cy][x] = red;
		}
	}

	return out;
}

// Draw horizontal epipolar lines to verify rectification
// In a properly rectified stereo pair, corresponding points lie on the same horizontal line
Image DrawEpipolarLines(const Image& src) {
	if (src.IsEmpty())
		return Image();
	Size sz = src.GetSize();
	ImageBuffer out(sz);

	// Copy source image
	for (int y = 0; y < sz.cy; y++) {
		const RGBA* s = src[y];
		RGBA* d = out[y];
		for (int x = 0; x < sz.cx; x++) {
			d[x] = s[x];
		}
	}

	// Draw horizontal lines at regular intervals
	// Use semi-transparent green so they don't obscure the image
	RGBA green;
	green.r = 0;
	green.g = 255;
	green.b = 0;
	green.a = 128;  // Semi-transparent

	// Draw lines every 10% of image height
	for (int i = 0; i <= 10; i++) {
		int y = (sz.cy * i) / 10;
		if (y >= 0 && y < sz.cy) {
			for (int x = 0; x < sz.cx; x++) {
				// Alpha blend with existing pixel
				RGBA& pixel = out[y][x];
				pixel.r = (byte)((pixel.r * (255 - green.a) + green.r * green.a) / 255);
				pixel.g = (byte)((pixel.g * (255 - green.a) + green.g * green.a) / 255);
				pixel.b = (byte)((pixel.b * (255 - green.a) + green.b * green.a) / 255);
			}
		}
	}

	return out;
}

vec3 TriangulatePoint(const vec3& pL, const vec3& dL, const vec3& pR, const vec3& dR) {
	vec3 w0 = pL - pR;
	double a = Dot(dL, dL);
	double b = Dot(dL, dR);
	double c = Dot(dR, dR);
	double d = Dot(dL, w0);
	double e = Dot(dR, w0);
	double denom = a * c - b * b;
	if (fabs(denom) > 1e-9) {
		double sc = (b * e - c * d) / denom;
		double tc = (a * e - b * d) / denom;
		return (pL + dL * (float)sc + pR + dR * (float)tc) * 0.5f;
	}
	return (pL + pR) * 0.5f + dL * 1000.0f;
}


static bool IsParamsValid(const StereoCalibrationParams& p) {
	return std::isfinite(p.a) && std::isfinite(p.cx) && std::isfinite(p.cy) &&
	       std::isfinite(p.c) && std::isfinite(p.d) &&
	       std::isfinite(p.yaw) && std::isfinite(p.pitch) && std::isfinite(p.roll) &&
	       std::isfinite(p.yaw_l) && std::isfinite(p.pitch_l) && std::isfinite(p.roll_l);
}

StageAWindow::StageAWindow() {
	Title("Stereo Calibration Tool - Stage A");
	Sizeable().Zoomable();
	AddFrame(menu);
	AddFrame(status);
	menu.Set(THISBACK(MainMenu));
	BuildLayout();
}

// Binds AppModel and builds all Stage A UI sections.
// Assumes AppModel is already loaded by the controller.
void StageAWindow::Init(AppModel& m) {
	model = &m;
	BuildStageAControls();
	BuildCaptureLists();
	BuildPlotters();
	RefreshFromModel();
}

// Pulls AppModel.project_state into UI controls and refreshes lists.
// Does not write any state back to disk.
void StageAWindow::RefreshFromModel() {
	if (!model)
		return;
	const ProjectState& ps = model->project_state;
	calib_eye_dist <<= ps.eye_dist;
	yaw_l <<= ps.yaw_l;
	pitch_l <<= ps.pitch_l;
	roll_l <<= ps.roll_l;
	yaw_r <<= ps.yaw_r;
	pitch_r <<= ps.pitch_r;
	roll_r <<= ps.roll_r;
	barrel_strength <<= ps.barrel_strength;
	fov_deg <<= ps.fov_deg;
	lens_cx <<= ps.lens_cx;
	lens_cy <<= ps.lens_cy;
	lens_k1 <<= ps.lens_k1;
	lens_k2 <<= ps.lens_k2;
	board_x <<= ps.board_x;
	board_y <<= ps.board_y;
	board_size <<= ps.square_size_mm;
	lock_intrinsics <<= ps.lock_intrinsics;
	lock_baseline <<= ps.lock_baseline;
	lock_yaw_symmetry <<= ps.lock_yaw_symmetry;
	preview_extrinsics <<= ps.preview_extrinsics;
	preview_intrinsics <<= ps.preview_intrinsics;
	overlay_eyes <<= ps.overlay_eyes;
	overlay_swap <<= ps.overlay_swap;
	show_difference <<= ps.show_difference;
	show_epipolar <<= ps.show_epipolar;
	tint_overlay <<= ps.tint_overlay;
	show_crosshair <<= ps.show_crosshair;
	rectified_overlay <<= ps.rectified_overlay;
	rectify_alpha_slider <<= (int)(ps.rectify_alpha * 100.0);  // alpha 0..1 to slider 0..100
	show_corners <<= ps.show_corners;
	show_reprojection <<= ps.show_reprojection;
	alpha_slider <<= ps.alpha;
			
			pipeline_state_lbl = "Pipeline: " + StereoCalibrationHelpers::GetCalibrationStateText(ps.calibration_state);
			
			captures_list.Clear();
			for (int i = 0; i < model->captured_frames.GetCount(); i++) {
				auto& f = model->captured_frames[i];
				
				captures_list.Add(
					Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), 
					f.source, 
					f.detected_l ? "Yes" : "No",
					f.detected_r ? "Yes" : "No",
					f.reproj_rms_l > 0 ? Format("%.3f", f.reproj_rms_l) : "-",
					f.reproj_rms_r > 0 ? Format("%.3f", f.reproj_rms_r) : "-",
					f.used ? "Yes" : "Skip",
					f.reject_reason
				);
			}	// If no selection or invalid selection, select the last image by default
	if (model->selected_capture < 0 || model->selected_capture >= captures_list.GetCount())
		model->selected_capture = captures_list.GetCount() - 1;

	if (model->selected_capture >= 0 && model->selected_capture < captures_list.GetCount())
		captures_list.SetCursor(model->selected_capture);

	// Rebuild rectification cache from saved calibration parameters (needed on restart)
	// Only rebuild if cache is invalid (don't overwrite freshly-computed rectification)
	if (!model->rectification_cache.valid) {
		RebuildRectificationFromState();
	}
	UpdateEpipolarDisplay();

	UpdatePreview();
}

// Composes the main layout (left controls + right preview/list split).
void StageAWindow::BuildLayout() {
	Add(controls.VSizePos(0, 0).LeftPos(0, 300));

	main_split.Vert(preview_split, tab_data);
	main_split.SetPos(6500);
	Add(main_split.VSizePos(0, 0).HSizePos(300, 0));

	preview_split.Horz(left_plot, right_plot);
	preview_split.SetPos(5000);
	
	BuildTabs();
	BuildCaptureLists();
}

void StageAWindow::BuildTabs() {
	tab_data.Add(tab_frames.SizePos(), "Frames");
	tab_data.Add(tab_board.SizePos(), "Board");
	tab_data.Add(tab_solve.SizePos(), "Solve");
	tab_data.Add(tab_report.SizePos(), "Report");
	
	tab_frames.Add(captures_list.SizePos());
	
	static Button export_btn;
	export_btn.SetLabel("Export OpenCV YAML...");
	export_btn <<= THISBACK(OnExportYaml);
	tab_solve.Add(export_btn.TopPos(10, 24).LeftPos(10, 150));
	
	coverage_lbl.SetLabel("Coverage Heatmap (8x6 grid)");
	tab_board.Add(coverage_lbl.TopPos(10, 20).LeftPos(10, 200));
	tab_board.Add(coverage_heat.TopPos(40, 200).LeftPos(10, 300));
	
	coverage_score_lbl.SetFont(Arial(12).Bold());
	tab_board.Add(coverage_score_lbl.TopPos(250, 24).LeftPos(10, 300));

	// Add epipolar metrics to Solve tab
	tab_solve.Add(epipolar_metric_lbl.TopPos(50, 20).LeftPos(10, 120));
	tab_solve.Add(epipolar_value_lbl.TopPos(50, 20).LeftPos(135, 400));
	epipolar_value_lbl.SetFont(Arial(10).Bold());

	tab_report.Add(report_log.SizePos());
	report_log.SetReadOnly();
}

// Builds Stage A controls (board params + view controls).
void StageAWindow::BuildStageAControls() {
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.SetInc(0.1);
	calib_eye_dist.WhenAction = THISBACK(SyncStageA);

	board_x_lbl.SetLabel("Squares X");
	board_x.MinMax(3, 25); board_x.WhenAction = THISBACK(SyncStageA);
	board_y_lbl.SetLabel("Squares Y");
	board_y.MinMax(3, 25); board_y.WhenAction = THISBACK(SyncStageA);
	board_sz_lbl.SetLabel("Size (mm)");
	board_size.MinMax(1.0, 1000.0); board_size.WhenAction = THISBACK(SyncStageA);
	
	detect_btn.SetLabel("Detect Corners");
	detect_btn <<= THISBACK(OnDetect);
	
	lock_intrinsics.SetLabel("Lock Intrinsics");
	lock_intrinsics.WhenAction = THISBACK(SyncStageA);
	
	lock_baseline.SetLabel("Lock Baseline");
	lock_baseline.WhenAction = THISBACK(SyncStageA);
	
	lock_yaw_symmetry.SetLabel("Lock Yaw Sym");
	lock_yaw_symmetry.WhenAction = THISBACK(SyncStageA);
	
	solve_int_btn.SetLabel("Solve Intrinsics");
	solve_int_btn <<= THISBACK(OnSolveIntrinsics);
	
	solve_stereo_btn.SetLabel("Solve Stereo");
	solve_stereo_btn <<= THISBACK(OnSolveStereo);

	preview_extrinsics.SetLabel("Preview extrinsics");
	preview_extrinsics <<= true;
	preview_extrinsics.WhenAction = THISBACK(SyncStageA);

	preview_intrinsics.SetLabel("Preview intrinsics");
	preview_intrinsics <<= false;
	preview_intrinsics.WhenAction = THISBACK(SyncStageA);

	barrel_lbl.SetLabel("Undistort strength");
	barrel_strength.SetInc(0.1); barrel_strength.MinMax(0, 5.0); barrel_strength <<= 0; barrel_strength.WhenAction = THISBACK(SyncStageA);
	fov_lbl.SetLabel("FOV (deg)");
	fov_deg.SetInc(1.0); fov_deg.MinMax(10, 170); fov_deg <<= 90; fov_deg.WhenAction = THISBACK(SyncStageA);

	cx_lbl.SetLabel("cx"); lens_cx.SetInc(1.0); lens_cx.WhenAction = THISBACK(SyncStageA);
	cy_lbl.SetLabel("cy"); lens_cy.SetInc(1.0); lens_cy.WhenAction = THISBACK(SyncStageA);
	k1_lbl.SetLabel("k1"); lens_k1.SetInc(0.01); lens_k1.MinMax(-2.0, 2.0); lens_k1.WhenAction = THISBACK(SyncStageA);
	k2_lbl.SetLabel("k2"); lens_k2.SetInc(0.01); lens_k2.MinMax(-2.0, 2.0); lens_k2.WhenAction = THISBACK(SyncStageA);

	basic_params_doc.SetReadOnly();

	show_corners.SetLabel("Show Corners");
	show_corners.WhenAction = THISBACK(OnReviewChanged);
	
	show_reprojection.SetLabel("Show Reproj");
	show_reprojection.WhenAction = THISBACK(OnReviewChanged);

	overlay_eyes.SetLabel("Overlay Eyes");
	overlay_eyes.WhenAction = THISBACK(OnReviewChanged);
	overlay_swap.SetLabel("Swap Order");
	overlay_swap.WhenAction = THISBACK(OnReviewChanged);
	show_difference.SetLabel("Show Diff");
	show_difference.WhenAction = THISBACK(OnReviewChanged);
	show_epipolar.SetLabel("Show epipolar lines (verify horizontal alignment)");
	show_epipolar.WhenAction = THISBACK(OnReviewChanged);

	tint_overlay.SetLabel("Tint overlay (L=blue, R=red)");
	tint_overlay <<= false;
	tint_overlay.WhenAction = THISBACK(OnReviewChanged);

	show_crosshair.SetLabel("Show center crosshair");
	show_crosshair <<= false;
	show_crosshair.WhenAction = THISBACK(OnReviewChanged);

	rectified_overlay.SetLabel("Rectified Overlay (aligns epipolar lines horizontally)");
	rectified_overlay <<= false;
	rectified_overlay.WhenAction = THISBACK(OnReviewChanged);

	rectify_alpha_lbl.SetLabel("Rectify α");
	rectify_alpha_slider.MinMax(0, 100);
	rectify_alpha_slider <<= 0;
	rectify_alpha_slider.WhenAction = THISBACK(SyncStageA);

	alpha_lbl.SetLabel("Alpha");
	alpha_slider.MinMax(0, 100);
	alpha_slider <<= 50;
	alpha_slider.WhenAction = THISBACK(OnReviewChanged);

	undo_btn.SetLabel("Undo");
	undo_btn <<= THISBACK(OnUndo);
	undo_btn.Disable();

	epipolar_metric_lbl.SetLabel("Epipolar Δy:");
	epipolar_value_lbl.SetLabel("Not computed");

	int y = 6;
	controls.Add(pipeline_state_lbl.TopPos(y, 20).HSizePos(8, 8));
	pipeline_state_lbl.SetFont(Arial(10).Bold());
	pipeline_state_lbl.SetAlign(ALIGN_CENTER);
	pipeline_state_lbl.SetInk(Blue());
	y += 24;
	controls.Add(calib_eye_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(calib_eye_dist.TopPos(y, 20).LeftPos(132, 80));
	y += 24;
	
	controls.Add(board_x_lbl.TopPos(y, 20).LeftPos(8, 80));
	controls.Add(board_x.TopPos(y, 20).LeftPos(92, 50));
	controls.Add(board_y_lbl.TopPos(y, 20).LeftPos(150, 80));
	controls.Add(board_y.TopPos(y, 20).LeftPos(234, 50));
	y += 24;
	controls.Add(board_sz_lbl.TopPos(y, 20).LeftPos(8, 80));
	controls.Add(board_size.TopPos(y, 20).LeftPos(92, 50));
	controls.Add(detect_btn.TopPos(y, 20).LeftPos(150, 134));
	y += 24;
	controls.Add(lock_intrinsics.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(lock_baseline.TopPos(y, 20).LeftPos(132, 100));
	controls.Add(lock_yaw_symmetry.TopPos(y, 20).LeftPos(236, 100));
	y += 30;
	
	controls.Add(solve_int_btn.TopPos(y, 24).LeftPos(8, 136));
	controls.Add(solve_stereo_btn.TopPos(y, 24).LeftPos(150, 134));
	y += 30;

	controls.Add(preview_extrinsics.TopPos(y, 20).LeftPos(8, 140));
	controls.Add(preview_intrinsics.TopPos(y, 20).LeftPos(152, 140));
	y += 24;
	controls.Add(barrel_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(barrel_strength.TopPos(y, 20).LeftPos(132, 50));
	controls.Add(k1_lbl.TopPos(y, 20).LeftPos(186, 20));
	controls.Add(lens_k1.TopPos(y, 20).LeftPos(210, 40));
	controls.Add(k2_lbl.TopPos(y, 20).LeftPos(254, 20));
	controls.Add(lens_k2.TopPos(y, 20).LeftPos(278, 40));
	y += 24;
	controls.Add(fov_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(fov_deg.TopPos(y, 20).LeftPos(132, 50));
	controls.Add(cx_lbl.TopPos(y, 20).LeftPos(186, 20));
	controls.Add(lens_cx.TopPos(y, 20).LeftPos(210, 40));
	controls.Add(cy_lbl.TopPos(y, 20).LeftPos(254, 20));
	controls.Add(lens_cy.TopPos(y, 20).LeftPos(278, 40));
	y += 24;
	controls.Add(basic_params_doc.TopPos(y, 100).HSizePos(8, 8));
	y += 104;
	
	controls.Add(undo_btn.TopPos(y, 20).LeftPos(8, 80));
	y += 30;

	controls.Add(overlay_eyes.TopPos(y, 20).LeftPos(8, 100));
	controls.Add(overlay_swap.TopPos(y, 20).LeftPos(112, 90));
	controls.Add(show_difference.TopPos(y, 20).LeftPos(206, 80));
	y += 24;
	controls.Add(show_corners.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(show_reprojection.TopPos(y, 20).LeftPos(132, 120));
	y += 24;
	controls.Add(alpha_lbl.TopPos(y, 20).LeftPos(8, 40));
	controls.Add(alpha_slider.TopPos(y, 20).LeftPos(52, 200));
	y += 24;
	controls.Add(tint_overlay.TopPos(y, 20).LeftPos(8, 200));
	y += 24;
	controls.Add(show_crosshair.TopPos(y, 20).LeftPos(8, 180));
	y += 24;
	controls.Add(show_epipolar.TopPos(y, 20).LeftPos(8, 180));
	y += 24;
	controls.Add(rectified_overlay.TopPos(y, 20).LeftPos(8, 280));
	y += 24;
	controls.Add(rectify_alpha_lbl.TopPos(y, 20).LeftPos(8, 60));
	controls.Add(rectify_alpha_slider.TopPos(y, 20).LeftPos(72, 180));
}




// Configures the capture list columns and selection callback.
void StageAWindow::BuildCaptureLists() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("L Det");
	captures_list.AddColumn("R Det");
	captures_list.AddColumn("RMS L");
	captures_list.AddColumn("RMS R");
	captures_list.AddColumn("Used");
	captures_list.AddColumn("Reject Reason");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);
	captures_list.WhenBar = THISBACK(OnCapturesBar);
}

void StageAWindow::OnCapturesBar(Bar& bar) {
	bar.Add("Delete selected", THISBACK(OnDeleteCapture))
	   .Enable(captures_list.IsCursor());
}

void StageAWindow::OnDeleteCapture() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount())
		return;
	
	if (!PromptOKCancel("Delete selected capture frame? This will rewrite indices on disk."))
		return;
	
	model->captured_frames.Remove(row);
	if (model->selected_capture >= model->captured_frames.GetCount())
		model->selected_capture = model->captured_frames.GetCount() - 1;
	
	SaveProjectState();
	RefreshFromModel();
}

// Initializes plotters (left/right images).
void StageAWindow::BuildPlotters() {
	left_plot.SetEye(0);
	left_plot.SetTitle("Left Eye");

	right_plot.SetEye(1);
	right_plot.SetTitle("Right Eye");
}

// Syncs Stage A UI values into AppModel.project_state and updates preview.
// Assumes controls contain valid numeric values.
void StageAWindow::SyncStageA() {
	if (!model)
		return;
	ProjectState& ps = model->project_state;
	ps.eye_dist = (double)calib_eye_dist;
	ps.yaw_l = (double)yaw_l;
	ps.pitch_l = (double)pitch_l;
	ps.roll_l = (double)roll_l;
	ps.yaw_r = (double)yaw_r;
	ps.pitch_r = (double)pitch_r;
	ps.roll_r = (double)roll_r;
	ps.barrel_strength = (double)barrel_strength;
	ps.fov_deg = (double)fov_deg;
	ps.lens_cx = (double)lens_cx;
	ps.lens_cy = (double)lens_cy;
	ps.lens_k1 = (double)lens_k1;
	ps.lens_k2 = (double)lens_k2;
	ps.board_x = (int)board_x;
	ps.board_y = (int)board_y;
	ps.square_size_mm = (double)board_size;
	ps.lock_intrinsics = (bool)lock_intrinsics;
	ps.lock_baseline = (bool)lock_baseline;
	ps.lock_yaw_symmetry = (bool)lock_yaw_symmetry;
	ps.preview_extrinsics = (bool)preview_extrinsics;
	ps.preview_intrinsics = (bool)preview_intrinsics;
	ps.rectify_alpha = (int)~rectify_alpha_slider / 100.0;  // UI slider 0..100 maps to alpha 0..1

	// If rectify_alpha changed, invalidate rectification cache
	if (fabs(model->rectification_cache.alpha - ps.rectify_alpha) > 1e-6) {
		model->rectification_cache.Invalidate();
	}

	String doc;
	doc << "Stage A Basic Params:\n";
	doc << "  Eye dist: " << ps.eye_dist << " mm\n";
	doc << Format("  FOV: %.1f, k1: %.3f, k2: %.3f\n", ps.fov_deg, ps.lens_k1, ps.lens_k2);
	doc << Format("  PP: %.1f, %.1f\n", ps.lens_cx, ps.lens_cy);
	doc << Format("  Left Yaw/Pitch/Roll: %.3f, %.3f, %.3f\n", ps.yaw_l, ps.pitch_l, ps.roll_l);
	doc << Format("  Right Yaw/Pitch/Roll: %.3f, %.3f, %.3f\n", ps.yaw_r, ps.pitch_r, ps.roll_r);
	basic_params_doc <<= doc;

	OnReviewChanged();
	SaveProjectState();
}

// Applies view-mode changes (overlay, alpha, epipolar) and refreshes preview.
void StageAWindow::OnReviewChanged() {
	if (!model)
		return;
	ProjectState& ps = model->project_state;
	ps.overlay_eyes = (bool)overlay_eyes;
	ps.overlay_swap = (bool)overlay_swap;
	ps.show_difference = (bool)show_difference;
	ps.show_epipolar = (bool)show_epipolar;
	ps.tint_overlay = (bool)tint_overlay;
	ps.show_crosshair = (bool)show_crosshair;
	ps.rectified_overlay = (bool)rectified_overlay;
	ps.show_corners = (bool)show_corners;
	ps.alpha = (int)~alpha_slider;

	for (auto& frame : model->captured_frames)
		frame.undist_valid = false;

	UpdatePreview();
	UpdateReviewEnablement();
	SaveProjectState();
}

void StageAWindow::OnUndo() {
	if (has_undo) {
		model->project_state = undo_state;
		has_undo = false;
		undo_btn.Disable();
		RefreshFromModel();
		SyncStageA();
	}
}

void StageAWindow::PushUndo() {
	undo_state = model->project_state;
	has_undo = true;
	undo_btn.Enable();
}

// Updates plotters based on capture selection.
void StageAWindow::UpdatePreview() {
	if (!model)
		return;
	UpdateReviewEnablement();
	int row = captures_list.GetCursor();
	model->selected_capture = row;
	
	UpdatePlotters();
}

// Updates left/right plotter images (raw or undistorted).
void StageAWindow::UpdatePlotters() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) {
		left_plot.SetImage(Image());
		right_plot.SetImage(Image());
		return;
	}
	CapturedFrame& frame = model->captured_frames[row];
	Size base_sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	if (base_sz.cx <= 0 || base_sz.cy <= 0) {
		left_plot.SetImage(Image());
		right_plot.SetImage(Image());
		return;
	}
	LensPoly lens;
	vec2 tilt;
	PreparePreviewLens(base_sz, lens, tilt);
	float max_radius = (float)sqrt(base_sz.cx * base_sz.cx * 0.25f + base_sz.cy * base_sz.cy * 0.25f);
	float max_angle = lens.PixelToAngle(max_radius);
	float linear_scale = (max_angle <= 1e-6f) ? 1.0f : (max_radius / max_angle);
	ApplyPreviewImages(frame, lens, linear_scale);
}

// Enables/disables review overlays based on calibration availability.
void StageAWindow::UpdateReviewEnablement() {
	bool has_poly = IsValidAnglePoly(model->last_calibration.angle_to_pixel);
	bool can_review = model->last_calibration.is_enabled && has_poly;

	// Epipolar lines don't require calibration - they can be shown on any captured frame
	// They're especially useful for verifying rectified overlay alignment
	show_epipolar.Enable(true);  // Always enable epipolar lines
}

// Builds/updates preview lens from AppModel settings and cache.
// Returns false if lens cannot be built from current data.
bool StageAWindow::PreparePreviewLens(const Size& sz, LensPoly& out_lens, vec2& out_tilt) {
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;

	StereoCalibrationParams p;
	bool use_basic = true;

	if (model->project_state.compare_basic_params)
		use_basic = true;

	if (use_basic) {
		// Use k1 radial model: r_d = f * theta * (1 + k1 * theta^2)
		// We use theta directly (equidistant-like) for the polynomial model
		// but focal length f is derived from FOV.
		double fov_deg = Clamp(model->project_state.fov_deg, 10.0, 170.0);
		double fov_rad = fov_deg * M_PI / 180.0;
		
		// Focal length for pinhole: f = (w/2) / tan(fov/2)
		double f = (sz.cx * 0.5) / tan(fov_rad * 0.5);
		
		p.a = f;
		
		// UI: barrel_strength increases => k1 becomes more negative => more barrel removed in Undistort
		// scale chosen so 1.0 is a typical "heavy" distortion
		double k1 = -model->project_state.barrel_strength * 0.1;
		
		p.b = 0;
		p.c = f * k1;
		p.d = 0;
		p.cx = sz.cx * 0.5;
		p.cy = sz.cy * 0.5;
	} else {
		p.a = model->last_calibration.angle_to_pixel[0];
		p.b = model->last_calibration.angle_to_pixel[1];
		p.c = model->last_calibration.angle_to_pixel[2];
		p.d = model->last_calibration.angle_to_pixel[3];
		p.cx = model->last_calibration.principal_point[0];
		p.cy = model->last_calibration.principal_point[1];
	}

	double yaw = 0, pitch = 0, roll = 0;
	if (model->project_state.preview_extrinsics) {
		yaw = model->project_state.yaw_r - model->project_state.yaw_l;
		pitch = model->project_state.pitch_r - model->project_state.pitch_l;
		roll = model->project_state.roll_r - model->project_state.roll_l;
	} else {
		yaw = model->last_calibration.outward_angle;
		pitch = model->last_calibration.right_pitch;
		roll = model->last_calibration.right_roll;
		if (model->project_state.stage_c_enabled) {
			yaw += model->dyaw_c;
			pitch += model->dpitch_c;
			roll += model->droll_c;
		}
	}
	p.yaw = yaw;
	p.pitch = pitch;
	p.roll = roll;

	vec4 poly((float)p.a, (float)p.b, (float)p.c, (float)p.d);
	vec2 pp((float)p.cx, (float)p.cy);
	vec2 tilt((float)p.pitch, (float)p.roll);

	bool needs = (preview_lens_size != sz) || !IsSamePoly(preview_lens_poly, poly) ||
		fabs(preview_lens_outward - (float)p.yaw) > 1e-6 ||
		fabs(preview_lens_pp[0] - pp[0]) > 1e-3 ||
		fabs(preview_lens_pp[1] - pp[1]) > 1e-3 ||
		fabs(preview_lens_tilt[0] - tilt[0]) > 1e-6 ||
		fabs(preview_lens_tilt[1] - tilt[1]) > 1e-6;

	if (needs) {
		preview_lens.SetAnglePixel(poly.data[0], poly.data[1], poly.data[2], poly.data[3]);
		preview_lens.SetEyeOutwardAngle((float)p.yaw);
		preview_lens.SetRightTilt((float)p.pitch, (float)p.roll);
		preview_lens.SetPrincipalPoint(pp[0], pp[1]);
		preview_lens.SetSize(sz);
		preview_lens_size = sz;
		preview_lens_poly = poly;
		preview_lens_outward = (float)p.yaw;
		preview_lens_pp = pp;
		preview_lens_tilt = tilt;
	}
    // LensPoly is non-copyable; configure out_lens explicitly.
    out_lens.SetAnglePixel(poly.data[0], poly.data[1], poly.data[2], poly.data[3]);
    out_lens.SetEyeOutwardAngle((float)p.yaw);
    out_lens.SetRightTilt((float)p.pitch, (float)p.roll);
    out_lens.SetPrincipalPoint(pp[0], pp[1]);
    out_lens.SetSize(sz);
	out_tilt = tilt;
	return true;
}

#if 0
// DEPRECATED: Builds undistort cache for a captured frame, if view mode requires it.
// NOTE: This is now deprecated since ApplyPreviewImages handles per-eye transforms directly.
// Kept for potential future use with view_mode settings.
bool StageAWindow::BuildUndistortCache(CapturedFrame& frame, const LensPoly& lens, float linear_scale) {
	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	if (model->project_state.view_mode == 0) {
		frame.undist_valid = false;
		return false;
	}
	if (frame.undist_valid && frame.undist_size == sz && IsSamePoly(frame.undist_poly, preview_lens_poly))
		return true;
	if (!frame.left_img.IsEmpty())
		frame.undist_left = StereoCalibrationHelpers::UndistortImage(frame.left_img, lens, linear_scale);
	else
		frame.undist_left = Image();
	if (!frame.right_img.IsEmpty())
		frame.undist_right = StereoCalibrationHelpers::UndistortImage(frame.right_img, lens, linear_scale);
	else
		frame.undist_right = Image();
	frame.undist_poly = preview_lens_poly;
	frame.undist_size = sz;
	frame.undist_valid = true;
	return true;
}
#endif

// Applies raw or transformed images to the plotters based on preview flags.
// Respects preview_extrinsics and preview_intrinsics toggles independently.
void StageAWindow::ApplyPreviewImages(CapturedFrame& frame, const LensPoly& lens, float linear_scale) {
	if (!model)
		return;

	const ProjectState& ps = model->project_state;
	bool apply_extr = ps.preview_extrinsics;
	bool apply_intr = ps.preview_intrinsics;

	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	
	// Helper to build params from arbitrary sources
	auto BuildParams = [&](const ProjectState& s) {
		StereoCalibrationHelpers::LensParams lp;
		lp.f = linear_scale; // Use common scale
		lp.cx = (float)(s.lens_cx > 0 ? s.lens_cx : sz.cx * 0.5);
		lp.cy = (float)(s.lens_cy > 0 ? s.lens_cy : sz.cy * 0.5);
		lp.k1 = (float)s.lens_k1;
		lp.k2 = (float)s.lens_k2;
		if (fabs(lp.k1) < 1e-6 && fabs(lp.k2) < 1e-6) 
			lp.k1 = (float)(-s.barrel_strength * 0.1);
		return lp;
	};

	auto lp_curr = BuildParams(ps);

	auto Render = [&](const Image& src, const StereoCalibrationHelpers::LensParams& lp, 
					  double y_deg, double p_deg, double r_deg, bool is_rad) {
		if (src.IsEmpty()) return Image();
		if (!apply_extr && !apply_intr) return src;
		
		float ry, rp, rr;
		if (is_rad) {
			ry = apply_extr ? (float)y_deg : 0;
			rp = apply_extr ? (float)p_deg : 0;
			rr = apply_extr ? (float)r_deg : 0;
		} else {
			ry = apply_extr ? (float)(y_deg * M_PI / 180.0) : 0;
			rp = apply_extr ? (float)(p_deg * M_PI / 180.0) : 0;
			rr = apply_extr ? (float)(r_deg * M_PI / 180.0) : 0;
		}
		
		// Apply Stage C deltas if enabled and compare is active
		if (ps.stage_c_compare) {
			// model->dyaw_c etc are in degrees
			ry += model->dyaw_c * M_PI / 180.0;
			rp += model->dpitch_c * M_PI / 180.0;
			rr += model->droll_c * M_PI / 180.0;
		}

		if (apply_intr) {
			return StereoCalibrationHelpers::RectifyAndRotateOnePass(src, lp, ry, rp, rr, sz);
		} else {
			vec2 pp(lp.cx, lp.cy);
			return StereoCalibrationHelpers::ApplyExtrinsicsOnly(src, ry, rp, rr, pp);
		}
	};

	Image L_curr = Render(frame.left_img, lp_curr, ps.yaw_l, ps.pitch_l, ps.roll_l, false);
	Image R_curr = Render(frame.right_img, lp_curr, ps.yaw_r, ps.pitch_r, ps.roll_r, false);

	if (ps.show_corners || ps.show_reprojection) {
		auto DrawCorners = [&](const Image& img, const Vector<Pointf>& corners, bool detected, Color c) {
			if (img.IsEmpty() || !detected) return img;
			ImageBuffer b(img.GetSize());
			Copy(b, Point(0,0), img, img.GetSize());
			for (const auto& p : corners) {
				int ix = (int)p.x;
				int iy = (int)p.y;
				for (int dy = -1; dy <= 1; dy++)
					for (int dx = -1; dx <= 1; dx++)
						if (ix + dx >= 0 && ix + dx < b.GetWidth() && iy + dy >= 0 && iy + dy < b.GetHeight())
							b[iy + dy][ix + dx] = c;
			}
			return (Image)b;
		};
		
		if (!apply_extr && !apply_intr) {
			if (ps.show_corners) {
				L_curr = DrawCorners(L_curr, frame.corners_l, frame.detected_l, Green());
				R_curr = DrawCorners(R_curr, frame.corners_r, frame.detected_r, Green());
			}
			if (ps.show_reprojection) {
				// For now, re-projection points are same as corners in RAW view
				// In production, we'd calculate cv::projectPoints here
			}
		}
	}

	last_left_preview = L_curr;
	last_right_preview = R_curr;

	// Sync points to plotters (in image pixels)
	Vector<Pointf> pts_l, pts_r;
	for (const MatchPair& m : frame.matches) {
		pts_l.Add(Pointf(m.left.x * sz.cx, m.left.y * sz.cy));
		pts_r.Add(Pointf(m.right.x * sz.cx, m.right.y * sz.cy));
	}
	left_plot.SetMatchingPoints(pts_l);
	right_plot.SetMatchingPoints(pts_r);

	// Compose final display images (handles overlay, tint, crosshair)
	ComposeFinalDisplayImages();
}

// Composes final display images from cached per-eye previews.
// Handles overlay, tint, crosshair - all view-only operations.
void StageAWindow::ComposeFinalDisplayImages() {
	if (!model)
		return;

	const ProjectState& ps = model->project_state;
	bool do_overlay = ps.overlay_eyes;
	bool do_tint = ps.tint_overlay;
	bool do_crosshair = ps.show_crosshair;
	bool do_diff = ps.show_difference;
	bool do_epipolar = ps.show_epipolar;
	bool swap_order = ps.overlay_swap;
	bool do_rectify = ps.rectified_overlay;
	float alpha = ps.alpha / 100.0f; // Convert 0..100 to 0..1

	Image left_display = last_left_preview;
	Image right_display = last_right_preview;

	// Apply rectification if enabled and cache is valid
	if (do_rectify && model->rectification_cache.valid) {
		// Convert U++ Images to cv::Mat
		cv::Mat left_mat = ImageToBGRMat(last_left_preview);
		cv::Mat right_mat = ImageToBGRMat(last_right_preview);

		// Apply rectification using cached remap maps
		cv::Mat left_rect, right_rect;
		cv::remap(left_mat, left_rect,
		          model->rectification_cache.map1x,
		          model->rectification_cache.map1y,
		          cv::INTER_LINEAR);
		cv::remap(right_mat, right_rect,
		          model->rectification_cache.map2x,
		          model->rectification_cache.map2y,
		          cv::INTER_LINEAR);

		// Convert back to U++ Images
		left_display = MatToImage(left_rect);
		right_display = MatToImage(right_rect);
	}

	// Show difference mode (takes priority over overlay)
	if (do_diff) {
		Image diff_img = ComputeDiff(last_left_preview, last_right_preview);
		if (do_crosshair)
			diff_img = DrawCrosshair(diff_img);
		left_plot.SetImage(diff_img);
		right_plot.SetImage(Image()); // Hide right plotter
		return;
	}

	if (do_overlay) {
		// Use rectified images if rectification was applied, otherwise use raw previews
		Image base_img = swap_order ? right_display : left_display;
		Image top_img = swap_order ? left_display : right_display;

		if (do_tint) {
			// Left = blue, Right = red
			Image left_tinted = TintBlue(left_display);
			Image right_tinted = TintRed(right_display);
			base_img = swap_order ? right_tinted : left_tinted;
			top_img = swap_order ? left_tinted : right_tinted;
		}

		// Alpha blend
		Image composited = AlphaBlend(base_img, top_img, alpha);

		// Apply epipolar lines to verify rectification (before crosshair so crosshair is on top)
		if (do_epipolar)
			composited = DrawEpipolarLines(composited);

		// Apply crosshair to composited image
		if (do_crosshair)
			composited = DrawCrosshair(composited);

		// Show composited in left plotter, hide right (or show same in both)
		left_plot.SetImage(composited);
		right_plot.SetImage(Image()); // Hide right plotter in overlay mode
	} else {
		// Side-by-side mode: apply epipolar lines and crosshair to each eye independently
		if (do_epipolar) {
			left_display = DrawEpipolarLines(left_display);
			right_display = DrawEpipolarLines(right_display);
		}

		if (do_crosshair) {
			left_display = DrawCrosshair(left_display);
			right_display = DrawCrosshair(right_display);
		}

		left_plot.SetImage(left_display);
		right_plot.SetImage(right_display);
	}
}

// Capture list selection callback: refresh preview/match list.
void StageAWindow::OnCaptureSelection() {
	UpdatePreview();
}

// Persists AppModel (project.json + images) to disk.
void StageAWindow::SaveProjectState() {
	if (!model || model->project_dir.IsEmpty())
		return;
	StereoCalibrationHelpers::SaveState(*model);
}

void StageAWindow::OnDetect() {
	SyncStageA();
	
	// Input UI values are SQUARES count.
	// OpenCV findChessboardCorners needs INNER CORNERS count.
	int nx = (int)board_x - 1;
	int ny = (int)board_y - 1;
	if (nx < 2 || ny < 2) {
		PromptOK("Invalid board dimensions (need at least 3x3 squares).");
		return;
	}
	
	Progress pi;
	pi.SetText("Detecting corners...");
	pi.SetTotal(model->captured_frames.GetCount());
	
	int total_l = 0, total_r = 0;
	
	for(int i=0; i<model->captured_frames.GetCount(); i++) {
		if (pi.Canceled()) break;
		pi.SetPos(i);
		
		CapturedFrame& f = model->captured_frames[i];
		
		// Convert to cv::Mat
		cv::Mat mL = ImageToGrayMat(f.left_img);
		cv::Mat mR = ImageToGrayMat(f.right_img);
		
		if (mL.empty() || mR.empty()) continue;
		
		std::vector<cv::Point2f> cornersL, cornersR;
		
		int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;
		
		f.detected_l = cv::findChessboardCorners(mL, cv::Size(nx, ny), cornersL, flags);
		f.detected_r = cv::findChessboardCorners(mR, cv::Size(nx, ny), cornersR, flags);
		
		f.reject_reason = "";
			
		if(f.detected_l) {
			cv::cornerSubPix(mL, cornersL, cv::Size(11, 11), cv::Size(-1, -1), 
				cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
			f.corners_l.Clear();
			for(const auto& p : cornersL) f.corners_l.Add(Pointf(p.x, p.y));
			
			// Quality check: board size
			cv::Rect bbox = cv::boundingRect(cornersL);
			double area_ratio = (double)(bbox.width * bbox.height) / (mL.cols * mL.rows);
			if (area_ratio < 0.05) {
				f.detected_l = false;
				f.reject_reason = "L: too small";
			} else {
				total_l++;
			}
		} else {
			f.corners_l.Clear();
		}
		
		if(f.detected_r) {
			cv::cornerSubPix(mR, cornersR, cv::Size(11, 11), cv::Size(-1, -1), 
				cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
			f.corners_r.Clear();
			for(const auto& p : cornersR) f.corners_r.Add(Pointf(p.x, p.y));
			
			// Quality check: board size
			cv::Rect bbox = cv::boundingRect(cornersR);
			double area_ratio = (double)(bbox.width * bbox.height) / (mR.cols * mR.rows);
			if (area_ratio < 0.05) {
				f.detected_r = false;
				f.reject_reason += (f.reject_reason.IsEmpty() ? "" : ", ") + String("R: too small");
			} else {
				total_r++;
			}
		} else {
			f.corners_r.Clear();
		}
		
		// A frame is potentially useful if at least one eye saw the board
		f.used = f.detected_l || f.detected_r;
	}
	
	RefreshFromModel();
	UpdateCoverageHeatmap();
	SaveProjectState();
	PromptOK(Format("Detection complete.\nLeft: %d\nRight: %d", total_l, total_r));
}

bool StageAWindow::CheckPoseDiversity() {
	int n_l = 0, n_r = 0;
	double min_sq_px = 1e9, max_sq_px = 0;
	
	for (const auto& f : model->captured_frames) {
		if (!f.used) continue;
		if (f.detected_l) {
			n_l++;
			cv::Rect bbox = cv::boundingRect(std::vector<cv::Point2f>{
				cv::Point2f((float)f.corners_l[0].x, (float)f.corners_l[0].y),
				cv::Point2f((float)f.corners_l.Top().x, (float)f.corners_l.Top().y)
			});
			double char_size = sqrt((double)bbox.width * bbox.height);
			min_sq_px = min(min_sq_px, char_size);
			max_sq_px = max(max_sq_px, char_size);
		}
		if (f.detected_r) n_r++;
	}
	
	if (n_l < 10 || n_r < 10) {
		PromptOK("Need more frames (at least 10 per eye) for stable calibration.");
		return false;
	}
	
	// Check distance diversity via square size proxy
	if (max_sq_px / min_sq_px < 1.5) {
		PromptOK("Need more distance diversity (move board closer and further).");
		return false;
	}
	
	return true;
}

void StageAWindow::UpdateCoverageHeatmap() {
	int rows = 6;
	int cols = 8;
	Vector<int> hits;
	hits.SetCount(rows * cols, 0);
	
	int total_pts = 0;
	Size img_sz(0,0);
	
	for(const auto& f : model->captured_frames) {
		if(!f.used || !f.detected_l) continue;
		
		Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : Size(0,0);
		if(sz.cx <= 0) continue;
		img_sz = sz;
		
		for(const auto& p : f.corners_l) {
			int cx = (int)(p.x / sz.cx * cols);
			int cy = (int)(p.y / sz.cy * rows);
			if(cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
				hits[cy * cols + cx]++;
				total_pts++;
			}
		}
	}
	
	if(img_sz.cx == 0) return;
	
	ImageBuffer ib(300, 200);
	Fill(ib, White(), 300*200);
	
	double cell_w = 300.0 / cols;
	double cell_h = 200.0 / rows;
	
	int max_hits = 1;
	for(int h : hits) max_hits = max(max_hits, h);
	
	for(int y=0; y<rows; y++) {
		for(int x=0; x<cols; x++) {
			int h = hits[y * cols + x];
			double intensity = (double)h / max_hits;
			
			RGBA color;
			if(h == 0) {
				color = Gray(); // No data
			} else {
				// Green scale based on intensity
				color.r = 0; 
				color.g = (int)(100 + 155 * intensity); 
				color.b = 0; 
				color.a = 255;
			}
			
			int x0 = (int)(x * cell_w);
			int y0 = (int)(y * cell_h);
			int x1 = (int)((x+1) * cell_w) - 2; // -2 for grid lines
			int y1 = (int)((y+1) * cell_h) - 2;
			
			for(int py=y0; py<=y1; py++)
				for(int px=x0; px<=x1; px++)
					if(px < 300 && py < 200) ib[py][px] = color;
		}
	}
	
	coverage_heat.SetImage(ib);
	
	int covered = 0;
	for(int h : hits) if(h > 0) covered++;
	double area_ratio = (double)covered / (rows * cols);
	
	String score_text = "Coverage: ";
	if (area_ratio > 0.7) score_text << "GOOD";
	else if (area_ratio > 0.4) score_text << "OK";
	else score_text << "POOR";
	
	score_text << Format(" (%.1f%%)", area_ratio * 100.0);
	coverage_score_lbl.SetLabel(score_text);
	coverage_score_lbl.SetInk(area_ratio > 0.7 ? Green() : (area_ratio > 0.4 ? Yellow() : Red()));
	
	coverage_lbl.SetLabel(Format("Heatmap: %d/%d cells", covered, rows*cols));
}

void StageAWindow::OnSolveIntrinsics() {
	SyncStageA();
	if (!CheckPoseDiversity()) return;
	
	int nx = (int)board_x - 1;
	int ny = (int)board_y - 1;
	double sz_mm = (double)board_size;
	
	if (nx < 2 || ny < 2) return;
	
	std::vector<std::vector<cv::Point3f>> objL, objR;
	std::vector<std::vector<cv::Point2f>> imgL, imgR;
	Vector<int> indicesL, indicesR;
	
	std::vector<cv::Point3f> obj;
	for(int y=0; y<ny; y++)
		for(int x=0; x<nx; x++)
			obj.push_back(cv::Point3f((float)(x * sz_mm), (float)(y * sz_mm), 0.0f));
	
	Size img_sz(0,0);
	
	for(int i=0; i<model->captured_frames.GetCount(); i++) {
		const auto& f = model->captured_frames[i];
		if(!f.used) continue;
		
		Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : Size(0,0);
		if (sz.cx <= 0) continue;
		img_sz = sz;
		
		if (f.detected_l) {
			objL.push_back(obj);
			std::vector<cv::Point2f> pts;
			for(const auto& p : f.corners_l) pts.push_back(cv::Point2f(p.x, p.y));
			imgL.push_back(pts);
			indicesL.Add(i);
		}
		if (f.detected_r) {
			objR.push_back(obj);
			std::vector<cv::Point2f> pts;
			for(const auto& p : f.corners_r) pts.push_back(cv::Point2f(p.x, p.y));
			imgR.push_back(pts);
			indicesR.Add(i);
		}
	}
	
	cv::Mat K_L = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat K_R = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat D_L, D_R;
	std::vector<cv::Mat> rvecsL, tvecsL, rvecsR, tvecsR;
	std::vector<double> perViewErrorsL, perFrameErrorsR;
	
	double rmsL = cv::calibrateCamera(objL, imgL, cv::Size(img_sz.cx, img_sz.cy), K_L, D_L, rvecsL, tvecsL, 
		cv::noArray(), cv::noArray(), perViewErrorsL, cv::CALIB_FIX_ASPECT_RATIO);
	double rmsR = cv::calibrateCamera(objR, imgR, cv::Size(img_sz.cx, img_sz.cy), K_R, D_R, rvecsR, tvecsR, 
		cv::noArray(), cv::noArray(), perFrameErrorsR, cv::CALIB_FIX_ASPECT_RATIO);
	
	for (int i = 0; i < indicesL.GetCount(); i++)
		model->captured_frames[indicesL[i]].reproj_rms_l = perViewErrorsL[i];
	for (int i = 0; i < indicesR.GetCount(); i++)
		model->captured_frames[indicesR[i]].reproj_rms_r = perFrameErrorsR[i];
	
	ProjectState& ps = model->project_state;
	ps.lens_f = (K_L.at<double>(0, 0) + K_R.at<double>(0, 0)) * 0.5;
	ps.lens_cx = (K_L.at<double>(0, 2) + K_R.at<double>(0, 2)) * 0.5;
	ps.lens_cy = (K_L.at<double>(1, 2) + K_R.at<double>(1, 2)) * 0.5;
	ps.lens_k1 = (D_L.at<double>(0) + D_R.at<double>(0)) * 0.5;
	ps.lens_k2 = (D_L.at<double>(1) + D_R.at<double>(1)) * 0.5;
	
	double fov_rad = 2.0 * atan((img_sz.cx * 0.5) / ps.lens_f);
	ps.fov_deg = fov_rad * 180.0 / M_PI;
	
	String report = Format("Intrinsics Solved (L:%d, R:%d frames)\n", (int)objL.size(), (int)objR.size());
	report << Format("Left RMS: %.4f px\n", rmsL);
	report << Format("Right RMS: %.4f px\n", rmsR);
	report << Format("Focal: %.2f\n", ps.lens_f);
	report << Format("Principal: %.2f, %.2f\n", ps.lens_cx, ps.lens_cy);
	report << Format("Distortion: k1=%.4f, k2=%.4f\n", ps.lens_k1, ps.lens_k2);
	
	report_log <<= report;
	RefreshFromModel();
	SaveProjectState();
}

void StageAWindow::OnSolveStereo() {
	SyncStageA();
	
	int nx = (int)board_x - 1;
	int ny = (int)board_y - 1;
	double sz_mm = (double)board_size;
	
	if (nx < 2 || ny < 2) return;
	
	std::vector<std::vector<cv::Point3f>> objectPoints;
	std::vector<std::vector<cv::Point2f>> imagePointsL, imagePointsR;
	
	std::vector<cv::Point3f> obj;
	for(int y=0; y<ny; y++)
		for(int x=0; x<nx; x++)
			obj.push_back(cv::Point3f((float)(x * sz_mm), (float)(y * sz_mm), 0.0f));
	
	Size img_sz(0,0);
	int n_frames = 0;
	
	for(const auto& f : model->captured_frames) {
		if(!f.used || !f.detected_l || !f.detected_r) continue;
		
		Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : Size(0,0);
		if (sz.cx <= 0) continue;
		img_sz = sz;
		
		objectPoints.push_back(obj);
		
		std::vector<cv::Point2f> ptsL, ptsR;
		for(const auto& p : f.corners_l) ptsL.push_back(cv::Point2f(p.x, p.y));
		for(const auto& p : f.corners_r) ptsR.push_back(cv::Point2f(p.x, p.y));
		
		imagePointsL.push_back(ptsL);
		imagePointsR.push_back(ptsR);
		n_frames++;
	}
	
	if (n_frames < 3) {
		PromptOK("Not enough valid stereo frames (need >= 3 with BOTH eyes detected).");
		return;
	}
	
	ProjectState& ps = model->project_state;
	
	cv::Mat K = cv::Mat::eye(3, 3, CV_64F);
	K.at<double>(0,0) = ps.lens_f;
	K.at<double>(1,1) = ps.lens_f;
	K.at<double>(0,2) = ps.lens_cx;
	K.at<double>(1,2) = ps.lens_cy;
	
	cv::Mat D = cv::Mat::zeros(5, 1, CV_64F);
	D.at<double>(0) = ps.lens_k1;
	D.at<double>(1) = ps.lens_k2;
	
	cv::Mat R, T, E, F;
	
	int flags = cv::CALIB_FIX_ASPECT_RATIO;
	if (ps.lock_intrinsics) {
		flags |= (cv::CALIB_FIX_INTRINSIC | cv::CALIB_SAME_FOCAL_LENGTH | cv::CALIB_FIX_PRINCIPAL_POINT);
	}
	
	if (ps.lock_baseline) {
		// OpenCV doesn't have a direct "FIX_BASELINE" but we can fix T if we knew it.
		// However, we usually want to solve for it. 
		// If user wants to lock baseline, we can potentially fix T after a first run.
	}

	// Use separate D matrices for stereoCalibrate
	cv::Mat D_L = cv::Mat::zeros(5, 1, CV_64F);
	D_L.at<double>(0) = ps.lens_k1; D_L.at<double>(1) = ps.lens_k2;
	cv::Mat D_R = D_L.clone();

	double rms = cv::stereoCalibrate(objectPoints, imagePointsL, imagePointsR,
		K, D_L, K, D_R,
		cv::Size(img_sz.cx, img_sz.cy),
		R, T, E, F,
		flags,
		cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 100, 1e-5));
		
	if (ps.lock_yaw_symmetry) {
		// Manually enforce symmetry on R
		cv::Mat mtxR, mtxQ, Qx, Qy, Qz;
		cv::Vec3d euler = cv::RQDecomp3x3(R, mtxR, mtxQ, Qx, Qy, Qz);
		// Force pitch and roll to 0 relative to each other if locking symmetry?
		// Actually relative R should be identity if perfect.
		// For high yaw toe-out, R should be mainly Y-rotation.
	}
		
	// Translation vector T is in board units (mm)
	double dist_mm = cv::norm(T);
	ps.eye_dist = dist_mm;
	
	// Sanity Check: Triangulate first point of first frame
	cv::Mat P1 = cv::Mat::eye(3, 4, CV_64F);
	cv::Mat P2(3, 4, CV_64F);
	R.copyTo(P2(cv::Rect(0, 0, 3, 3)));
	T.copyTo(P2(cv::Rect(3, 0, 1, 3)));
	
	// Normalized points for triangulation
	cv::Mat ptL(1, 1, CV_64FC2), ptR(1, 1, CV_64FC2);
	ptL.at<cv::Vec2d>(0,0) = cv::Vec2d((imagePointsL[0][0].x - ps.lens_cx)/ps.lens_f, (imagePointsL[0][0].y - ps.lens_cy)/ps.lens_f);
	ptR.at<cv::Vec2d>(0,0) = cv::Vec2d((imagePointsR[0][0].x - ps.lens_cx)/ps.lens_f, (imagePointsR[0][0].y - ps.lens_cy)/ps.lens_f);
	
	cv::Mat pts4d;
	cv::triangulatePoints(P1, P2, ptL, ptR, pts4d);
	double z = pts4d.at<double>(2,0) / pts4d.at<double>(3,0);
	bool z_ok = (z > 0);
	
	// Decompose R into Euler angles
	cv::Mat mtxR, mtxQ, Qx, Qy, Qz;
	cv::Vec3d eulerAngles = cv::RQDecomp3x3(R, mtxR, mtxQ, Qx, Qy, Qz);
	
	ps.yaw_l = 0; ps.pitch_l = 0; ps.roll_l = 0;
	ps.yaw_r = eulerAngles[1];   
	ps.pitch_r = eulerAngles[0]; 
	ps.roll_r = eulerAngles[2];  
	
	String report;
	report << report_log.Get() << "\n";
	report << Format("Stereo Solved (%d frames)\n", n_frames);
	report << Format("  Stereo RMS: %.4f px\n", rms);
	report << Format("  Baseline: %.2f mm\n", dist_mm);
	report << Format("  Z-Check (dist to board): %.1f mm (%s)\n", z, z_ok ? "OK" : "BACKWARDS!");
	report << Format("  Right Relative: Y=%.2f, P=%.2f, R=%.2f deg\n", ps.yaw_r, ps.pitch_r, ps.roll_r);
	
	report_log <<= report;

	// Compute stereo rectification (for rectified overlay preview)
	ComputeStereoRectification(K, D_L, K, D_R, R, T, img_sz);

	// Compute epipolar alignment metrics
	ComputeEpipolarMetrics(K, D_L, K, D_R, R, T);
	UpdateEpipolarDisplay();

	// Append epipolar metrics to report
	if (model->epipolar_num_points > 0) {
		String epipolar_report = Format("  Epipolar Δy: median=%.2f px, p95=%.2f px\n",
		                                 model->epipolar_median_dy,
		                                 model->epipolar_p95_dy);
		report << epipolar_report;
		report_log <<= epipolar_report;
	}

	// Save report.txt
	String report_path = AppendFileName(model->project_dir, "report.txt");
	FileOut out(report_path);
	out << report;
	out.Close();

	RefreshFromModel();
	SaveProjectState();
}

void StageAWindow::OnExportYaml() {
	FileSel fs;
	fs.Type("OpenCV YAML", "*.yaml");
	if (!fs.ExecuteSaveAs("Export OpenCV Calibration")) return;
	
	const ProjectState& ps = model->project_state;
	
	cv::FileStorage file(std::string(~fs), cv::FileStorage::WRITE);
	
	cv::Mat K = (cv::Mat_<double>(3,3) << ps.lens_f, 0, ps.lens_cx, 0, ps.lens_f, ps.lens_cy, 0, 0, 1);
	cv::Mat D = (cv::Mat_<double>(5,1) << ps.lens_k1, ps.lens_k2, 0, 0, 0);
	
	file << "K" << K;
	file << "D" << D;
	
	// Relative rotation (approximate decomposition from solved Euler)
	// For production, we should store the cv::Mat R directly in AppModel.
	// But here we re-construct from Euler for the bridge.
	mat4 rot = AxesMat(ps.yaw_r * M_PI / 180.0, ps.pitch_r * M_PI / 180.0, ps.roll_r * M_PI / 180.0);
	cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
	for(int r=0; r<3; r++) for(int c=0; r<3; r++) R.at<double>(r,c) = rot[r][c];
	
	cv::Mat T = (cv::Mat_<double>(3,1) << ps.eye_dist, 0, 0); // Assuming primary X-translation
	
	file << "R" << R;
	file << "T" << T;
	file.release();
	
	PromptOK("Exported calibration to:\n" + fs.Get());
}

// ------------------------------------------------------------
// Stereo Rectification Pipeline
// ------------------------------------------------------------

/*
ComputeStereoRectification:
- Computes rectification transforms using cv::stereoRectify
- Stores results in model->rectification_cache
- Called after stereo calibration completes

Why this is needed:
- Rectification aligns epipolar lines horizontally, making stereo matching easier
- Allows visual verification that stereo calibration is correct
- Enables epipolar alignment metrics (median Δy should be ~0 after rectification)
*/
void StageAWindow::ComputeStereoRectification(const cv::Mat& K1, const cv::Mat& D1,
                                               const cv::Mat& K2, const cv::Mat& D2,
                                               const cv::Mat& R, const cv::Mat& T,
                                               const Size& img_sz) {
	StereoRectificationCache& cache = model->rectification_cache;

	// Get rectify alpha parameter from project state
	double alpha = model->project_state.rectify_alpha;

	// Check if cache is still valid
	if (cache.IsValid(K1, D1, K2, D2, R, T, cv::Size(img_sz.cx, img_sz.cy), alpha)) {
		return; // Cache is valid, no need to recompute
	}

	// Compute rectification using OpenCV stereoRectify
	// This function computes the rotation matrices (R1, R2) and projection matrices (P1, P2)
	// that transform the cameras into a common rectified coordinate system.
	cv::stereoRectify(
		K1, D1,           // Left camera intrinsics
		K2, D2,           // Right camera intrinsics
		cv::Size(img_sz.cx, img_sz.cy),
		R, T,             // Stereo extrinsics
		cache.R1,         // Output: left rectification rotation
		cache.R2,         // Output: right rectification rotation
		cache.P1,         // Output: left projection matrix in rectified coords
		cache.P2,         // Output: right projection matrix in rectified coords
		cache.Q,          // Output: disparity-to-depth mapping matrix
		cv::CALIB_ZERO_DISPARITY, // Flags: principal points at same position after rectification
		alpha,            // 0=crop all invalid, 1=retain all pixels
		cv::Size(img_sz.cx, img_sz.cy), // newImageSize: keep same resolution
		&cache.roi1,      // Output: left valid region
		&cache.roi2       // Output: right valid region
	);

	// Store input parameters for cache validation
	cache.K1 = K1.clone();
	cache.D1 = D1.clone();
	cache.K2 = K2.clone();
	cache.D2 = D2.clone();
	cache.R = R.clone();
	cache.T = T.clone();
	cache.image_size = cv::Size(img_sz.cx, img_sz.cy);
	cache.alpha = alpha;
	cache.valid = true;

	// Now we need to build the remap maps
	BuildRectificationMaps();
}

/*
BuildRectificationMaps:
- Builds undistort+rectify remap maps using cv::initUndistortRectifyMap
- Uses cached rectification parameters from ComputeStereoRectification
- These maps are applied with cv::remap to generate rectified preview images

Why separate from ComputeStereoRectification:
- Remap maps depend only on rectification parameters, not on frame content
- Can be reused for all frames with same calibration
*/
void StageAWindow::BuildRectificationMaps() {
	StereoRectificationCache& cache = model->rectification_cache;

	if (!cache.valid) return;

	// Build remap maps for left camera
	// Maps pixels from rectified image coordinates back to original distorted coordinates
	cv::initUndistortRectifyMap(
		cache.K1,         // Camera intrinsics
		cache.D1,         // Distortion coefficients
		cache.R1,         // Rectification rotation
		cache.P1,         // Projection matrix in rectified coords
		cache.image_size, // Output image size
		CV_32FC1,         // Map type (32-bit float, single channel per map)
		cache.map1x,      // Output: x-coordinates map
		cache.map1y       // Output: y-coordinates map
	);

	// Build remap maps for right camera
	cv::initUndistortRectifyMap(
		cache.K2, cache.D2, cache.R2, cache.P2,
		cache.image_size, CV_32FC1,
		cache.map2x, cache.map2y
	);
}

/*
RebuildRectificationFromState:
- Rebuilds stereo rectification from saved ProjectState parameters after program restart
- Reconstructs K, D, R, T matrices from saved calibration data
- Called from RefreshFromModel() to restore rectification cache

Why this is needed:
- StereoRectificationCache is in-memory only (cv::Mat cannot be serialized to JSON)
- When program restarts, rectification must be recomputed from saved parameters
- Without this, rectified overlay only works during session, not after restart

Implementation notes:
- Assumes both eyes share same intrinsics (K matrix) - current calibration pipeline
- Reconstructs R (rotation) from right eye Euler angles (yaw_r, pitch_r, roll_r)
- T (translation) is just [eye_dist, 0, 0] baseline separation
- Only rebuilds if calibration data exists (lens_f > 0) and frames are available
*/
void StageAWindow::RebuildRectificationFromState() {
	if (!model) return;

	const ProjectState& ps = model->project_state;

	// Check if we have calibration data saved
	if (ps.lens_f <= 0 || ps.eye_dist <= 0) {
		model->rectification_cache.Invalidate();
		return;
	}

	// Get image size from first captured frame (needed for rectification)
	Size img_sz(0, 0);
	if (model->captured_frames.GetCount() > 0) {
		img_sz = model->captured_frames[0].left_img.GetSize();
	}

	if (img_sz.cx <= 0 || img_sz.cy <= 0) {
		model->rectification_cache.Invalidate();
		return;
	}

	// Reconstruct K matrix (same for both eyes in current implementation)
	cv::Mat K = cv::Mat::eye(3, 3, CV_64F);
	K.at<double>(0, 0) = ps.lens_f;  // fx
	K.at<double>(1, 1) = ps.lens_f;  // fy
	K.at<double>(0, 2) = ps.lens_cx; // cx (principal point x)
	K.at<double>(1, 2) = ps.lens_cy; // cy (principal point y)

	// Reconstruct D matrix (distortion coefficients)
	cv::Mat D = cv::Mat::zeros(5, 1, CV_64F);
	D.at<double>(0) = ps.lens_k1;    // k1 (radial distortion)
	D.at<double>(1) = ps.lens_k2;    // k2 (radial distortion)
	// k3, p1, p2 = 0 (not used in current calibration)

	// Reconstruct R from right eye Euler angles
	// Rotation matrix represents relative orientation of right camera w.r.t. left
	double yaw_rad = ps.yaw_r * M_PI / 180.0;
	double pitch_rad = ps.pitch_r * M_PI / 180.0;
	double roll_rad = ps.roll_r * M_PI / 180.0;

	mat4 rot = AxesMat(yaw_rad, pitch_rad, roll_rad);

	cv::Mat R = cv::Mat::eye(3, 3, CV_64F);
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			R.at<double>(r, c) = rot[r][c];
		}
	}

	// Reconstruct T (translation vector: baseline separation)
	// [eye_dist, 0, 0] means right camera is eye_dist mm to the right of left camera
	cv::Mat T = (cv::Mat_<double>(3, 1) << ps.eye_dist, 0, 0);

	// Rebuild rectification cache from reconstructed parameters
	ComputeStereoRectification(K, D, K, D, R, T, img_sz);
}

/*
ComputeEpipolarMetrics:
- Computes epipolar alignment quality metrics after stereo calibration
- Uses detected checkerboard corners from all frames with both eyes detected
- Rectifies corner points and measures |yL - yR| (vertical disparity)
- Good calibration should have median |Δy| < 2 pixels

Why this matters:
- Epipolar constraint says corresponding points lie on same horizontal line after rectification
- Large vertical disparity indicates calibration error or misalignment
- Provides quantitative validation of rectification quality
*/
void StageAWindow::ComputeEpipolarMetrics(const cv::Mat& K1, const cv::Mat& D1,
                                           const cv::Mat& K2, const cv::Mat& D2,
                                           const cv::Mat& R, const cv::Mat& T) {
	StereoRectificationCache& cache = model->rectification_cache;

	if (!cache.valid) {
		model->epipolar_median_dy = -1.0;
		model->epipolar_p95_dy = -1.0;
		model->epipolar_num_points = 0;
		return;
	}

	// Collect all corner pairs from frames with both eyes detected
	std::vector<cv::Point2f> left_pts, right_pts;

	for (const auto& f : model->captured_frames) {
		if (!f.used || !f.detected_l || !f.detected_r) continue;

		for (int i = 0; i < min(f.corners_l.GetCount(), f.corners_r.GetCount()); i++) {
			left_pts.push_back(cv::Point2f(f.corners_l[i].x, f.corners_l[i].y));
			right_pts.push_back(cv::Point2f(f.corners_r[i].x, f.corners_r[i].y));
		}
	}

	if (left_pts.empty()) {
		model->epipolar_median_dy = -1.0;
		model->epipolar_p95_dy = -1.0;
		model->epipolar_num_points = 0;
		return;
	}

	// Undistort and rectify points
	std::vector<cv::Point2f> left_rect, right_rect;
	cv::undistortPoints(left_pts, left_rect, K1, D1, cache.R1, cache.P1);
	cv::undistortPoints(right_pts, right_rect, K2, D2, cache.R2, cache.P2);

	// Compute |yL - yR| for each point pair
	Vector<double> dy_values;
	dy_values.Reserve(left_rect.size());

	for (int i = 0; i < (int)left_rect.size(); i++) {
		double dy = fabs(left_rect[i].y - right_rect[i].y);
		dy_values.Add(dy);
	}

	// Compute median and 95th percentile
	Sort(dy_values);
	int n = dy_values.GetCount();
	model->epipolar_num_points = n;

	if (n > 0) {
		model->epipolar_median_dy = dy_values[n / 2];
		int p95_idx = min(n - 1, (int)(n * 0.95));
		model->epipolar_p95_dy = dy_values[p95_idx];
	} else {
		model->epipolar_median_dy = -1.0;
		model->epipolar_p95_dy = -1.0;
	}
}

/*
UpdateEpipolarDisplay:
- Updates UI labels showing epipolar alignment metrics
- Color-codes results: green if good, yellow if ok, red if poor
*/
void StageAWindow::UpdateEpipolarDisplay() {
	if (model->epipolar_num_points == 0 || model->epipolar_median_dy < 0) {
		epipolar_metric_lbl.SetLabel("Epipolar Δy:");
		epipolar_value_lbl.SetLabel("Not computed");
		epipolar_value_lbl.SetInk(Black());
		return;
	}

	epipolar_metric_lbl.SetLabel("Epipolar Δy:");

	String value_text = Format("median=%.2f px, p95=%.2f px (%d pts)",
	                           model->epipolar_median_dy,
	                           model->epipolar_p95_dy,
	                           model->epipolar_num_points);

	epipolar_value_lbl.SetLabel(value_text);

	// Color coding: < 1.0 px = excellent (green), < 2.0 px = good (yellow), >= 2.0 px = poor (red)
	if (model->epipolar_median_dy < 1.0) {
		epipolar_value_lbl.SetInk(Green());
	} else if (model->epipolar_median_dy < 2.0) {
		epipolar_value_lbl.SetInk(Color(200, 150, 0)); // Yellow-orange
	} else {
		epipolar_value_lbl.SetInk(Red());
	}
}

void StageAWindow::MainMenu(Bar& bar) {
	bar.Add("Edit", THISBACK(SubMenuEdit));
	bar.Add("Help", THISBACK(SubMenuHelp));
}

void StageAWindow::SubMenuHelp(Bar& bar) {
	bar.Add("Instructions", [] { StereoCalibrationHelpers::ShowInstructions(); });
}

void StageAWindow::SubMenuEdit(Bar& bar) {
	bar.Add("Delete selected frame", THISBACK(OnDeleteCapture))
	   .Enable(captures_list.IsCursor());
}

END_UPP_NAMESPACE
