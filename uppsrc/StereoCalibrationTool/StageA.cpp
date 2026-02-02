#include "StereoCalibrationTool.h"

NAMESPACE_UPP

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
	preview_extrinsics <<= ps.preview_extrinsics;
	preview_intrinsics <<= ps.preview_intrinsics;
	view_mode_list.SetIndex(ps.view_mode);
	overlay_eyes <<= ps.overlay_eyes;
	overlay_swap <<= ps.overlay_swap;
	show_difference <<= ps.show_difference;
	show_epipolar <<= ps.show_epipolar;
	tint_overlay <<= ps.tint_overlay;
	show_crosshair <<= ps.show_crosshair;
	alpha_slider <<= ps.alpha;
	
	pipeline_state_lbl = "Pipeline: " + StereoCalibrationHelpers::GetCalibrationStateText(ps.calibration_state);
	
	// Safety check for legacy tool modes
	if (ps.tool_mode >= tool_list.GetCount())
		const_cast<ProjectState&>(ps).tool_mode = 0; // Reset to None if invalid
	
	tool_list.SetIndex(ps.tool_mode);

	captures_list.Clear();
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		auto& f = model->captured_frames[i];
		captures_list.Add(
			Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), 
			f.source, 
			f.detected_l ? "Yes" : "No",
			f.detected_r ? "Yes" : "No",
			f.used ? "Yes" : "Skip"
		);
	}
	
	// If no selection or invalid selection, select the last image by default
	if (model->selected_capture < 0 || model->selected_capture >= captures_list.GetCount())
		model->selected_capture = captures_list.GetCount() - 1;

	if (model->selected_capture >= 0 && model->selected_capture < captures_list.GetCount())
		captures_list.SetCursor(model->selected_capture);
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
	
	coverage_lbl.SetLabel("Coverage Heatmap (8x6 grid)");
	tab_board.Add(coverage_lbl.TopPos(10, 20).LeftPos(10, 200));
	tab_board.Add(coverage_heat.TopPos(40, 200).LeftPos(10, 300));
	
	tab_report.Add(report_log.SizePos());
	report_log.SetReadOnly();
}

// Builds Stage A controls (board params + view controls).
void StageAWindow::BuildStageAControls() {
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.SetInc(0.1);
	calib_eye_dist.WhenAction = THISBACK(SyncStageA);

	board_x_lbl.SetLabel("Corners X");
	board_x.MinMax(3, 20); board_x.WhenAction = THISBACK(SyncStageA);
	board_y_lbl.SetLabel("Corners Y");
	board_y.MinMax(3, 20); board_y.WhenAction = THISBACK(SyncStageA);
	board_sz_lbl.SetLabel("Size (mm)");
	board_size.MinMax(1.0, 1000.0); board_size.WhenAction = THISBACK(SyncStageA);
	
	detect_btn.SetLabel("Detect Corners");
	detect_btn <<= THISBACK(OnDetect);
	
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

	view_mode_lbl.SetLabel("View Mode");
	view_mode_list.Add(0, "Raw");
	view_mode_list.Add(1, "Basic Undistort");
	view_mode_list.Add(2, "Solved Undistort");
	view_mode_list.SetIndex(0);
	view_mode_list.WhenAction = THISBACK(OnReviewChanged);

	overlay_eyes.SetLabel("Overlay Eyes");
	overlay_eyes.WhenAction = THISBACK(OnReviewChanged);
	overlay_swap.SetLabel("Swap Order");
	overlay_swap.WhenAction = THISBACK(OnReviewChanged);
	show_difference.SetLabel("Show Diff");
	show_difference.WhenAction = THISBACK(OnReviewChanged);
	show_epipolar.SetLabel("Show epipolar line");
	show_epipolar.WhenAction = THISBACK(OnReviewChanged);

	tint_overlay.SetLabel("Tint overlay (L=blue, R=red)");
	tint_overlay <<= false;
	tint_overlay.WhenAction = THISBACK(OnReviewChanged);

	show_crosshair.SetLabel("Show center crosshair");
	show_crosshair <<= false;
	show_crosshair.WhenAction = THISBACK(OnReviewChanged);

	alpha_lbl.SetLabel("Alpha");
	alpha_slider.MinMax(0, 100);
	alpha_slider <<= 50;
	alpha_slider.WhenAction = THISBACK(OnReviewChanged);

	undo_btn.SetLabel("Undo");
	undo_btn <<= THISBACK(OnUndo);
	undo_btn.Disable();

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
	controls.Add(alpha_lbl.TopPos(y, 20).LeftPos(8, 40));
	controls.Add(alpha_slider.TopPos(y, 20).LeftPos(52, 200));
	y += 24;
	controls.Add(tint_overlay.TopPos(y, 20).LeftPos(8, 200));
	y += 24;
	controls.Add(show_crosshair.TopPos(y, 20).LeftPos(8, 180));
	y += 24;
	controls.Add(show_epipolar.TopPos(y, 20).LeftPos(8, 180));
}




void StageAWindow::OnGAStop() {
	if (ga_running) {
		ga_cancel = 1;
		ga_status_lbl.SetLabel("Status: Stopping...");
	}
}

void StageAWindow::OnGAStep(int gen, double best_cost, StereoCalibrationParams best_p) {
	if (!ga_running) return;
	ga_status_lbl.SetLabel(Format("Gen: %d, Cost: %.4f", gen, best_cost));
	ga_plot.AddPoint(gen, best_cost);
	
	if (IsParamsValid(best_p)) {
		GAEntry& e = ga_history.Add();
		e.params = best_p;
		e.cost = best_cost;
		ga_history_slider.MinMax(0, max(1, ga_history.GetCount()));
		if (ga_history.GetCount() > 0) ga_history_slider.Enable();
	}
}

void StageAWindow::OnGAFinished() {
	ga_running = false;
	ga_start.Enable();
	ga_stop.Disable();
	
	if (ga_best_params.a > 1e-6) {
		ga_apply.Enable();
		compare_ga_toggle.Enable();
		ga_status_lbl.SetLabel("Status: Finished");
		
		// Populate best results list
		ga_best_results_list.Clear();
		int n = 0;
		for(int i = ga_history.GetCount() - 1; i >= 0 && n < 20; i--) {
			const auto& e = ga_history[i];
			const auto& p = e.params;
			
			String desc = Format("fov:%.1f, y/p/r:%.1f/%.1f/%.1f", 
				2.0 * atan((ga_input_matches[0].image_size.cx * 0.5) / p.a) * 180.0 / M_PI,
				p.yaw * 180.0 / M_PI, p.pitch * 180.0 / M_PI, p.roll * 180.0 / M_PI);
			
			ga_best_results_list.Add(n + 1, Format("%.4f", e.cost), desc);
			n++;
		}
	} else {
		ga_apply.Disable();
		compare_ga_toggle.Disable();
		ga_status_lbl.SetLabel("Status: Aborted/Failed");
		return;
	}
	
	// Compute diagnostics
	if (ga_input_matches.IsEmpty()) return;
	
	StereoCalibrationSolver solver;
	solver.matches <<= ga_input_matches;
	solver.eye_dist = model->project_state.eye_dist / 1000.0;
	solver.dist_weight = model->project_state.distance_weight;
	solver.huber_px = model->project_state.huber_px;
	solver.huber_m = model->project_state.huber_m;
	
	// Reconstruct initial params from current project state (baseline)
	const ProjectState& ps = model->project_state;
	StereoCalibrationParams initial_p;
	
	// Assume width from first match
	double w = solver.matches[0].image_size.cx;
	double fov_rad = ps.fov_deg * M_PI / 180.0;
	double f = (w * 0.5) / tan(fov_rad * 0.5);
	
	initial_p.a = f;
	initial_p.b = 0;
	initial_p.c = f * ps.lens_k1;
	initial_p.d = f * ps.lens_k2;
	initial_p.cx = ps.lens_cx > 0 ? ps.lens_cx : w*0.5;
	initial_p.cy = ps.lens_cy > 0 ? ps.lens_cy : solver.matches[0].image_size.cy*0.5;
	initial_p.yaw_l = ps.yaw_l * M_PI / 180.0;
	initial_p.pitch_l = ps.pitch_l * M_PI / 180.0;
	initial_p.roll_l = ps.roll_l * M_PI / 180.0;
	initial_p.yaw = ps.yaw_r * M_PI / 180.0;
	initial_p.pitch = ps.pitch_r * M_PI / 180.0;
	initial_p.roll = ps.roll_r * M_PI / 180.0;
	
	StereoCalibrationGADiagnostics diag;
	StereoCalibrationDiagnostics d_init, d_final;
	
	solver.ComputeDiagnostics(initial_p, d_init);
	solver.ComputeDiagnostics(ga_best_params, d_final);
	
	auto CalcCost = [&](const StereoCalibrationDiagnostics& d) {
		double c = 0;
		for(const auto& r : d.residuals) {
			c += r.err_l_px*r.err_l_px + r.err_r_px*r.err_r_px;
		}
		return c;
	};
	
	double cost0 = CalcCost(d_init);
	double cost1 = CalcCost(d_final);
	
	solver.ComputeGADiagnostics(ga_best_params, cost0, cost1, diag);
	
	// Store in model
	model->project_state.last_ga_diagnostics.best_cost = diag.best_cost;
	model->project_state.last_ga_diagnostics.initial_cost = diag.initial_cost;
	model->project_state.last_ga_diagnostics.cost_improvement_ratio = diag.cost_improvement_ratio;
	model->project_state.last_ga_diagnostics.num_matches_used = diag.num_matches_used;
	model->project_state.last_ga_diagnostics.mean_reproj_error_px = diag.mean_reproj_error_px;
	model->project_state.last_ga_diagnostics.max_reproj_error_px = diag.max_reproj_error_px;
	model->project_state.last_ga_diagnostics.median_reproj_error_px = diag.median_reproj_error_px;
	
	// Update UI
	String text;
	text << "Diagnostics:\n";
	text << Format("  Improvement: %.2fx (%.2f -> %.2f)\n", diag.cost_improvement_ratio, diag.initial_cost, diag.best_cost);
	text << Format("  Mean Reproj Err: %.2f px\n", diag.mean_reproj_error_px);
	text << Format("  Max Reproj Err: %.2f px\n", diag.max_reproj_error_px);
	text << Format("  Matches: %d\n", diag.num_matches_used);
	
	if (diag.cost_improvement_ratio < 1.2)
		text << "\nWARNING: Low improvement (< 1.2x)";
	if (diag.mean_reproj_error_px > 5.0)
		text << "\nWARNING: High reprojection error (> 5px)";
		
	ga_diag_lbl.SetLabel(text);
}

void StageAWindow::OnGAApply() {
	if (ga_running) return;
	
	double f = ga_best_params.a;
	if (fabs(f) > 1e-6) {
		ProjectState& ps = model->project_state;
		int phase = ps.ga_phase;
		
		if (phase == GA_PHASE_INTRINSICS || phase == GA_PHASE_BOTH) {
			ps.lens_f = f;
			ps.lens_cx = ga_best_params.cx;
			ps.lens_cy = ga_best_params.cy;
			ps.lens_k1 = ga_best_params.c / f;
			ps.lens_k2 = ga_best_params.d / f;
			
			// Update FOV for UI consistency
			int row = captures_list.GetCursor();
			if (row >= 0 && row < model->captured_frames.GetCount()) {
				const CapturedFrame& cf = model->captured_frames[row];
				Size sz = !cf.left_img.IsEmpty() ? cf.left_img.GetSize() : cf.right_img.GetSize();
				if (sz.cx > 0) {
					double fov_rad = 2.0 * atan((sz.cx * 0.5) / f);
					ps.fov_deg = fov_rad * 180.0 / M_PI;
				}
			}
			ps.calibration_state = CALIB_GA_INTRINSICS;
		}
		
		if (phase == GA_PHASE_EXTRINSICS || phase == GA_PHASE_BOTH) {
			ps.yaw_l = ga_best_params.yaw_l * 180.0 / M_PI;
			ps.pitch_l = ga_best_params.pitch_l * 180.0 / M_PI;
			ps.roll_l = ga_best_params.roll_l * 180.0 / M_PI;
			ps.yaw_r = ga_best_params.yaw * 180.0 / M_PI;
			ps.pitch_r = ga_best_params.pitch * 180.0 / M_PI;
			ps.roll_r = ga_best_params.roll * 180.0 / M_PI;
			if (phase == GA_PHASE_EXTRINSICS)
				ps.calibration_state = CALIB_GA_EXTRINSICS;
		}
		
		RefreshFromModel(); // Updates UI and Preview
		SaveProjectState();
		PromptOK("Genetic optimization results applied.");
	}
}

// Configures the capture list columns and selection callback.
void StageAWindow::BuildCaptureLists() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("L Det");
	captures_list.AddColumn("R Det");
	captures_list.AddColumn("Used");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);
	captures_list.WhenBar = THISBACK(OnCapturesBar);
}

void StageAWindow::OnLinesBar(Bar& bar) {
	bar.Add("Delete selected", THISBACK(OnDeleteLine))
	   .Enable(lines_list.IsCursor());
}

void StageAWindow::OnDeleteLine() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) return;
	
	int lrow = lines_list.GetCursor();
	if (lrow < 0) return;
	
	CapturedFrame& f = model->captured_frames[row];
	// We need to map list row to left/right arrays.
	// We populate list: Left lines then Right lines.
	int n_left = f.annotation_lines_left.GetCount();
	if (lrow < n_left) {
		f.annotation_lines_left.Remove(lrow);
	} else {
		f.annotation_lines_right.Remove(lrow - n_left);
	}
	SaveProjectState();
	UpdatePreview();
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
	left_plot.WhenClickPoint = THISBACK(OnPickMatchTool);
	left_plot.WhenHoverPoint = [=](Pointf p) { OnHoverPoint(p, 0); };
	left_plot.WhenFinalizeLine = THISBACK(OnFinalizeLine);

	right_plot.SetEye(1);
	right_plot.SetTitle("Right Eye");
	right_plot.WhenClickPoint = THISBACK(OnPickMatchTool);
	right_plot.WhenHoverPoint = [=](Pointf p) { OnHoverPoint(p, 1); };
	right_plot.WhenFinalizeLine = THISBACK(OnFinalizeLine);
}

// Handler for hovering over plotters to drive epipolar lines
void StageAWindow::OnHoverPoint(Pointf p_rect, int eye) {
	hover_point = p_rect;
	hover_eye = eye;
	
	if (show_epipolar_lines) {
		left_plot.SetEpipolarY(p_rect.y);
		right_plot.SetEpipolarY(p_rect.y);
	} else {
		left_plot.SetEpipolarY(-1);
		right_plot.SetEpipolarY(-1);
	}
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
	ps.preview_extrinsics = (bool)preview_extrinsics;
	ps.preview_intrinsics = (bool)preview_intrinsics;
	
	ps.tool_mode = tool_list.GetIndex();

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
	ps.view_mode = view_mode_list.GetIndex();
	ps.overlay_eyes = (bool)overlay_eyes;
	ps.overlay_swap = (bool)overlay_swap;
	ps.show_difference = (bool)show_difference;
	ps.show_epipolar = (bool)show_epipolar;
	ps.tint_overlay = (bool)tint_overlay;
	ps.show_crosshair = (bool)show_crosshair;
	ps.alpha = (int)~alpha_slider;

	// Local view-only diagnostics
	left_plot.SetShowCurvature((bool)show_curvature_error);
	right_plot.SetShowCurvature((bool)show_curvature_error);
	
	// Epipolar lines update
	if (!(bool)show_epipolar_lines) {
		left_plot.SetEpipolarY(-1);
		right_plot.SetEpipolarY(-1);
	} else if (!IsNull(hover_point)) {
		OnHoverPoint(hover_point, hover_eye);
	}

	for (auto& frame : model->captured_frames)
		frame.undist_valid = false;

	UpdatePreview();
	UpdateReviewEnablement();
	SaveProjectState();
}

void StageAWindow::OnToolAction() {
	SyncStageA();
	pending_left = Null;
	
	ToolMode m = ToolMode::None;
	int idx = tool_list.GetIndex();
	if (idx == 1) m = ToolMode::PickMatch;
	if (idx == 2) m = ToolMode::LineAnnotate;
	
	left_plot.SetToolMode(m);
	right_plot.SetToolMode(m);
	
	Refresh();
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


Pointf StageAWindow::UnprojectPoint(int eye, Pointf p_view, Size sz) {
	if (sz.cx <= 0) return p_view;
	const ProjectState& ps = model->project_state;
	int vmode = view_mode_list.GetIndex();
	if (vmode == 0 || (!ps.preview_extrinsics && !ps.preview_intrinsics))
		return p_view;

	double fov_rad = Clamp(ps.fov_deg, 10.0, 170.0) * M_PI / 180.0;
	float f = (float)((sz.cx * 0.5) / tan(fov_rad * 0.5));
	StereoCalibrationHelpers::LensParams lp;
	lp.f = f;
	lp.cx = (float)(ps.lens_cx > 0 ? ps.lens_cx : sz.cx * 0.5);
	lp.cy = (float)(ps.lens_cy > 0 ? ps.lens_cy : sz.cy * 0.5);
	lp.k1 = (float)ps.lens_k1;
	lp.k2 = (float)ps.lens_k2;
	if (fabs(lp.k1) < 1e-6 && fabs(lp.k2) < 1e-6) lp.k1 = (float)(-ps.barrel_strength * 0.1);

	float ry = ps.preview_extrinsics ? (float)(ps.yaw_l * M_PI / 180.0) : 0;
	float rp = ps.preview_extrinsics ? (float)(ps.pitch_l * M_PI / 180.0) : 0;
	float rr = ps.preview_extrinsics ? (float)(ps.roll_l * M_PI / 180.0) : 0;
	if (eye == 1) {
		ry = ps.preview_extrinsics ? (float)(ps.yaw_r * M_PI / 180.0) : 0;
		rp = ps.preview_extrinsics ? (float)(ps.pitch_r * M_PI / 180.0) : 0;
		rr = ps.preview_extrinsics ? (float)(ps.roll_r * M_PI / 180.0) : 0;
	}
	if (!ps.preview_intrinsics) { lp.k1 = 0; lp.k2 = 0; }
	return StereoCalibrationHelpers::UnprojectPointOnePass(p_view, sz, lp, ry, rp, rr);
}

void StageAWindow::OnFinalizeLine(int eye, const Vector<Pointf>& chain) {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) return;
	
	CapturedFrame& frame = model->captured_frames[row];
	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	if (sz.cx <= 0) return;

	Vector<Pointf> norm_chain;
	for(Pointf p_rect : chain) {
		Pointf p_raw = UnprojectPoint(eye, p_rect, sz);
		if (p_raw.x >= 0)
			norm_chain.Add(Pointf(p_raw.x / sz.cx, p_raw.y / sz.cy));
	}
	
	if (norm_chain.GetCount() >= 2) {
		if (eye == 0) frame.annotation_lines_left.Add(pick(norm_chain));
		else frame.annotation_lines_right.Add(pick(norm_chain));
		SaveProjectState();
	}
	UpdatePreview();
}

void StageAWindow::OnPickMatchTool(int eye, Pointf p) {
	int mode = tool_list.GetIndex();
	
	// Pick match point (1)
	if (mode == 1) {
		if (eye == 0) {
			pending_left = p;
			status.Set("Point picked in Left eye. Now click corresponding point in Right eye.");
		} else {
			if (IsNull(pending_left)) {
				status.Set("Pick point in Left eye first.");
				return;
			}
			
			int row = captures_list.GetCursor();
			if (row < 0) {
				status.Set("Select a capture frame first.");
				return;
			}
			
			CapturedFrame& frame = model->captured_frames[row];
			Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
			if (sz.cx <= 0) return;

			Pointf p_l_raw = UnprojectPoint(0, pending_left, sz);
			Pointf p_r_raw = UnprojectPoint(1, p, sz);

			if (p_l_raw.x >= 0 && p_r_raw.x >= 0) {
				MatchPair& m = frame.matches.Add();
				m.left = Pointf(p_l_raw.x / sz.cx, p_l_raw.y / sz.cy);
				m.right = Pointf(p_r_raw.x / sz.cx, p_r_raw.y / sz.cy);
				m.left_text = Format("L%d", frame.matches.GetCount());
				m.right_text = Format("R%d", frame.matches.GetCount());
				status.Set("Match pair added.");
				SaveProjectState();
			} else {
				status.Set("Failed to unproject points.");
			}
			
			pending_left = Null;
			UpdatePreview();
		}
	}
}

// Updates plotters + match list based on capture selection.
void StageAWindow::UpdatePreview() {
	if (!model)
		return;
	UpdateReviewEnablement();
	int row = captures_list.GetCursor();
	model->selected_capture = row;
	if (row < 0 || row >= model->captured_frames.GetCount()) {
		matches_list.Clear();
		UpdatePlotters();
		return;
	}

    CapturedFrame& frame = model->captured_frames[row];
    matches_list.Clear();
	for (int i = 0; i < frame.matches.GetCount(); i++) {
		const MatchPair& pair = frame.matches[i];
		matches_list.Add(
			Format("%d", i),
			Format("%.3f, %.3f", pair.left.x, pair.left.y),
			Format("%.3f, %.3f", pair.right.x, pair.right.y),
			pair.dist_l,
			pair.dist_r
		);
	}
	
	lines_list.Clear();
	for(int i=0; i<frame.annotation_lines_left.GetCount(); i++)
		lines_list.Add("Left", frame.annotation_lines_left[i].GetCount());
	for(int i=0; i<frame.annotation_lines_right.GetCount(); i++)
		lines_list.Add("Right", frame.annotation_lines_right[i].GetCount());

	UpdatePlotters();
	
	// Sync annotation lines
	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	Vector<Vector<Pointf>> lines_l, lines_r;
	if (sz.cx > 0) {
		const ProjectState& ps = model->project_state;
		int vmode = view_mode_list.GetIndex();
		
		// Lens params for projection
		double fov_rad = Clamp(ps.fov_deg, 10.0, 170.0) * M_PI / 180.0;
		float f = (float)((sz.cx * 0.5) / tan(fov_rad * 0.5));
		
		StereoCalibrationHelpers::LensParams lp;
		lp.f = f;
		lp.cx = (float)(ps.lens_cx > 0 ? ps.lens_cx : sz.cx * 0.5);
		lp.cy = (float)(ps.lens_cy > 0 ? ps.lens_cy : sz.cy * 0.5);
		lp.k1 = (float)ps.lens_k1;
		lp.k2 = (float)ps.lens_k2;
		if (fabs(lp.k1) < 1e-6 && fabs(lp.k2) < 1e-6) lp.k1 = (float)(-ps.barrel_strength * 0.1);

		auto ProjectChain = [&](const Vector<Pointf>& raw_chain, int eye) {
			Vector<Pointf> rect_chain;
			for(Pointf p_norm : raw_chain) {
				// Raw point in pixels
				Pointf p_raw(p_norm.x * sz.cx, p_norm.y * sz.cy);
				
				Pointf p_rect;
				if (vmode == 0 || (!ps.preview_extrinsics && !ps.preview_intrinsics)) {
					p_rect = p_raw;
				} else {
					float ry = ps.preview_extrinsics ? (float)(ps.yaw_l * M_PI / 180.0) : 0;
					float rp = ps.preview_extrinsics ? (float)(ps.pitch_l * M_PI / 180.0) : 0;
					float rr = ps.preview_extrinsics ? (float)(ps.roll_l * M_PI / 180.0) : 0;
					if (eye == 1) {
						ry = ps.preview_extrinsics ? (float)(ps.yaw_r * M_PI / 180.0) : 0;
						rp = ps.preview_extrinsics ? (float)(ps.pitch_r * M_PI / 180.0) : 0;
						rr = ps.preview_extrinsics ? (float)(ps.roll_r * M_PI / 180.0) : 0;
					}
					
					if (ps.preview_intrinsics) {
						p_rect = StereoCalibrationHelpers::ProjectPointOnePass(p_raw, sz, lp, ry, rp, rr);
					} else {
						// Rotation only. We use ProjectPointOnePass with zero distortion.
						StereoCalibrationHelpers::LensParams lp_nodist = lp;
						lp_nodist.k1 = 0; lp_nodist.k2 = 0;
						p_rect = StereoCalibrationHelpers::ProjectPointOnePass(p_raw, sz, lp_nodist, ry, rp, rr);
					}
				}
				if (p_rect.x >= 0) rect_chain.Add(p_rect);
			}
			return rect_chain;
		};

		for(const auto& chain : frame.annotation_lines_left) lines_l.Add(ProjectChain(chain, 0));
		for(const auto& chain : frame.annotation_lines_right) lines_r.Add(ProjectChain(chain, 1));
	}
	left_plot.SetAnnotationLines(lines_l);
	right_plot.SetAnnotationLines(lines_r);
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
	show_epipolar.Enable(can_review);
	if (!can_review)
		show_epipolar = false;
}

// Builds/updates preview lens from AppModel settings and cache.
// Returns false if lens cannot be built from current data.
bool StageAWindow::PreparePreviewLens(const Size& sz, LensPoly& out_lens, vec2& out_tilt) {
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;

	StereoCalibrationParams p;
	bool use_basic = false;
	int vmode = view_mode_list.GetIndex();
	if (vmode == 1)
		use_basic = true;
	else if (vmode == 2)
		use_basic = false;
	else
		use_basic = true;

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
	bool compare_ga = ps.compare_ga_result;

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
	bool swap_order = ps.overlay_swap;
	float alpha = ps.alpha / 100.0f; // Convert 0..100 to 0..1

	Image left_display = last_left_preview;
	Image right_display = last_right_preview;

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
		// Apply tint before blending if enabled
		Image base_img = swap_order ? last_right_preview : last_left_preview;
		Image top_img = swap_order ? last_left_preview : last_right_preview;

		if (do_tint) {
			// Left = blue, Right = red
			Image left_tinted = TintBlue(last_left_preview);
			Image right_tinted = TintRed(last_right_preview);
			base_img = swap_order ? right_tinted : left_tinted;
			top_img = swap_order ? left_tinted : right_tinted;
		}

		// Alpha blend
		Image composited = AlphaBlend(base_img, top_img, alpha);

		// Apply crosshair to composited image
		if (do_crosshair)
			composited = DrawCrosshair(composited);

		// Show composited in left plotter, hide right (or show same in both)
		left_plot.SetImage(composited);
		right_plot.SetImage(Image()); // Hide right plotter in overlay mode
	} else {
		// Side-by-side mode: apply crosshair to each eye independently
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

// Writes edited match distances back into AppModel.
void StageAWindow::OnMatchEdited() {
	int row = captures_list.GetCursor();
	int mrow = matches_list.GetCursor();
	if (row >= 0 && row < model->captured_frames.GetCount() && mrow >= 0) {
		CapturedFrame& f = model->captured_frames[row];
		if (mrow < f.matches.GetCount()) {
			f.matches[mrow].dist_l = matches_list.Get(mrow, 2);
			f.matches[mrow].dist_r = matches_list.Get(mrow, 3);
			SaveProjectState();
		}
	}
}

// Persists AppModel (project.json + images) to disk.
void StageAWindow::SaveProjectState() {
	if (!model || model->project_dir.IsEmpty())
		return;
	StereoCalibrationHelpers::SaveState(*model);
}

void StageAWindow::OnMatchesBar(Bar& bar) {
	bar.Add("Delete selected match", THISBACK(OnDeleteMatch))
	   .Enable(matches_list.IsCursor());
}

void StageAWindow::OnDeleteMatch() {
	int row = captures_list.GetCursor();
	int mrow = matches_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) return;
	
	CapturedFrame& f = model->captured_frames[row];
	if (mrow < 0 || mrow >= f.matches.GetCount()) return;
	
	f.matches.Remove(mrow);
	SaveProjectState();
	UpdatePreview();
}

void StageAWindow::MainMenu(Bar& bar) {
	bar.Add("Edit", THISBACK(SubMenuEdit));
}

void StageAWindow::SubMenuEdit(Bar& bar) {
	bar.Add("Delete selected match point", THISBACK(OnDeleteMatch))
	   .Enable(matches_list.IsCursor());
	bar.Add("Delete all match points in frame", THISBACK(OnDeleteAllMatches))
	   .Enable(model && model->selected_capture >= 0);
	bar.Separator();
	bar.Add("Delete selected annotation line", THISBACK(OnDeleteLine))
	   .Enable(lines_list.IsCursor());
	bar.Add("Delete all annotation lines in frame", THISBACK(OnDeleteAllLines))
	   .Enable(model && model->selected_capture >= 0);
}

void StageAWindow::OnDeleteAllMatches() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) return;
	if (!PromptOKCancel("Delete all match points in this frame?")) return;
	
	model->captured_frames[row].matches.Clear();
	SaveProjectState();
	UpdatePreview();
}

void StageAWindow::OnDeleteAllLines() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) return;
	if (!PromptOKCancel("Delete all annotation lines in this frame?")) return;
	
	model->captured_frames[row].annotation_lines_left.Clear();
	model->captured_frames[row].annotation_lines_right.Clear();
	SaveProjectState();
	UpdatePreview();
}

void StageAWindow::OnDetect() {
	SyncStageA();
	PromptOK("Detection not implemented yet.");
}

void StageAWindow::OnSolveIntrinsics() {
	SyncStageA();
	PromptOK("Solve Intrinsics not implemented yet.");
}

void StageAWindow::OnSolveStereo() {
	SyncStageA();
	PromptOK("Solve Stereo not implemented yet.");
}

END_UPP_NAMESPACE
