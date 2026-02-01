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
	BuildGAPanel();
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
	preview_extrinsics <<= ps.preview_extrinsics;
	preview_intrinsics <<= ps.preview_intrinsics;
	compare_ga_toggle <<= ps.compare_ga_result;
	view_mode_list.SetIndex(ps.view_mode);
	overlay_eyes <<= ps.overlay_eyes;
	overlay_swap <<= ps.overlay_swap;
	show_difference <<= ps.show_difference;
	show_epipolar <<= ps.show_epipolar;
	tint_overlay <<= ps.tint_overlay;
	show_crosshair <<= ps.show_crosshair;
	alpha_slider <<= ps.alpha;
	
	// Safety check for legacy tool modes
	if (ps.tool_mode >= tool_list.GetCount())
		const_cast<ProjectState&>(ps).tool_mode = 0; // Reset to None if invalid
	
	tool_list.SetIndex(ps.tool_mode);
	
	ga_pop_edit <<= model->ga_population;
	ga_gen_edit <<= model->ga_generations;

	captures_list.Clear();
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		auto& f = model->captured_frames[i];
		captures_list.Add(Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), f.source, f.matches.GetCount());
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
	
	details_split.Vert(matches_list, lines_list);
	details_split.SetPos(5000);
	
	list_split.Horz(captures_list, details_split);
	list_split.SetPos(3500);
	
	tab_data.Add(list_split, "Data");
	tab_data.Add(ga_tab_ctrl, "Genetic Optimizer");
}

// Builds Stage A controls (basic params + view controls).
void StageAWindow::BuildStageAControls() {
	calib_eye_lbl.SetLabel("Eye dist (mm)");
	calib_eye_dist.SetInc(0.1);
	calib_eye_dist.WhenAction = THISBACK(SyncStageA);

	yaw_l_lbl.SetLabel("Yaw");
	pitch_l_lbl.SetLabel("Pitch");
	roll_l_lbl.SetLabel("Roll");
	yaw_r_lbl.SetLabel("Yaw");
	pitch_r_lbl.SetLabel("Pitch");
	roll_r_lbl.SetLabel("Roll");

	yaw_l.SetInc(0.01); yaw_l.MinMax(-180, 180); yaw_l.WhenAction = THISBACK(SyncStageA);
	pitch_l.SetInc(0.01); pitch_l.MinMax(-90, 90); pitch_l.WhenAction = THISBACK(SyncStageA);
	roll_l.SetInc(0.01); roll_l.MinMax(-180, 180); roll_l.WhenAction = THISBACK(SyncStageA);
	yaw_r.SetInc(0.01); yaw_r.MinMax(-180, 180); yaw_r.WhenAction = THISBACK(SyncStageA);
	pitch_r.SetInc(0.01); pitch_r.MinMax(-90, 90); pitch_r.WhenAction = THISBACK(SyncStageA);
	roll_r.SetInc(0.01); roll_r.MinMax(-180, 180); roll_r.WhenAction = THISBACK(SyncStageA);

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

	tool_lbl.SetLabel("Tool");
	tool_list.Add(0, "None");
	tool_list.Add(1, "Pick Match");
	tool_list.Add(2, "Line Annotate");
	tool_list.SetIndex(0);
	tool_list.WhenAction = THISBACK(OnToolAction);

	undo_btn.SetLabel("Undo");
	undo_btn <<= THISBACK(OnUndo);
	undo_btn.Disable();

	int y = 6;
	controls.Add(calib_eye_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(calib_eye_dist.TopPos(y, 20).LeftPos(132, 80));
	y += 24;

	eye_l_group.SetLabel("Left Eye");
	controls.Add(eye_l_group.TopPos(y, 100).HSizePos(8, 8));
	int gy = y + 20;
	controls.Add(yaw_l_lbl.TopPos(gy, 20).LeftPos(12, 40));
	controls.Add(yaw_l.TopPos(gy, 20).LeftPos(52, 80));
	controls.Add(pitch_l_lbl.TopPos(gy + 24, 20).LeftPos(12, 40));
	controls.Add(pitch_l.TopPos(gy + 24, 20).LeftPos(52, 80));
	controls.Add(roll_l_lbl.TopPos(gy + 48, 20).LeftPos(12, 40));
	controls.Add(roll_l.TopPos(gy + 48, 20).LeftPos(52, 80));
	y += 100;

	eye_r_group.SetLabel("Right Eye");
	controls.Add(eye_r_group.TopPos(y, 100).HSizePos(8, 8));
	gy = y + 20;
	controls.Add(yaw_r_lbl.TopPos(gy, 20).LeftPos(12, 40));
	controls.Add(yaw_r.TopPos(gy, 20).LeftPos(52, 80));
	controls.Add(pitch_r_lbl.TopPos(gy + 24, 20).LeftPos(12, 40));
	controls.Add(pitch_r.TopPos(gy + 24, 20).LeftPos(52, 80));
	controls.Add(roll_r_lbl.TopPos(gy + 48, 20).LeftPos(12, 40));
	controls.Add(roll_r.TopPos(gy + 48, 20).LeftPos(52, 80));
	y += 100;

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
	
	controls.Add(tool_lbl.TopPos(y, 20).LeftPos(8, 40));
	controls.Add(tool_list.TopPos(y, 20).LeftPos(52, 160));
	controls.Add(undo_btn.TopPos(y, 20).LeftPos(216, 76));
	y += 30;

	// ViewMode DropList hidden in Stage A (simplified UI; toggles control all preview modes)
	// controls.Add(view_mode_lbl.TopPos(y, 20).LeftPos(8, 80));
	// controls.Add(view_mode_list.TopPos(y, 20).LeftPos(92, 160));
	// y += 24;
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

void StageAWindow::BuildGAPanel() {
	int y = 6;
	ga_group.SetLabel("Genetic Optimizer");
	ga_tab_ctrl.Add(ga_group.TopPos(y, 240).HSizePos(4, 4));
	
	int gy = y + 20;
	ga_start.SetLabel("Start GA");
	ga_start <<= THISBACK(OnGAStart);
	ga_tab_ctrl.Add(ga_start.TopPos(gy, 24).LeftPos(12, 80));
	
	ga_stop.SetLabel("Stop");
	ga_stop <<= THISBACK(OnGAStop);
	ga_stop.Disable();
	ga_tab_ctrl.Add(ga_stop.TopPos(gy, 24).LeftPos(100, 60));
	
	ga_apply.SetLabel("Apply Best");
	ga_apply <<= THISBACK(OnGAApply);
	ga_apply.Disable();
	ga_tab_ctrl.Add(ga_apply.TopPos(gy, 24).LeftPos(170, 80));
	
	gy += 30;
	ga_pop_lbl.SetLabel("Pop:");
	ga_pop_edit.MinMax(10, 1000);
	ga_tab_ctrl.Add(ga_pop_lbl.TopPos(gy, 20).LeftPos(12, 30));
	ga_tab_ctrl.Add(ga_pop_edit.TopPos(gy, 20).LeftPos(46, 50));
	
	ga_gen_lbl.SetLabel("Gen:");
	ga_gen_edit.MinMax(1, 1000);
	ga_tab_ctrl.Add(ga_gen_lbl.TopPos(gy, 20).LeftPos(104, 30));
	ga_tab_ctrl.Add(ga_gen_edit.TopPos(gy, 20).LeftPos(138, 50));
	
	gy += 30;
	ga_use_all_frames.SetLabel("Use all frames");
	ga_use_all_frames <<= true;
	ga_tab_ctrl.Add(ga_use_all_frames.TopPos(gy, 20).LeftPos(12, 100));
	
	gy += 30;
	ga_status_lbl.SetLabel("Status: Idle");
	ga_tab_ctrl.Add(ga_status_lbl.TopPos(gy, 20).LeftPos(12, 200));
	
	gy += 24;
	compare_ga_toggle.SetLabel("Compare with GA Best");
	compare_ga_toggle.WhenAction = THISBACK(SyncStageA);
	compare_ga_toggle.Disable(); // Enabled when GA finishes
	ga_tab_ctrl.Add(compare_ga_toggle.TopPos(gy, 20).LeftPos(12, 200));
	
	gy += 24;
	ga_tab_ctrl.Add(ga_plot.TopPos(gy, 80).LeftPos(12, 260));
	
	gy += 84;
	ga_diag_lbl.SetFont(Arial(10));
	ga_diag_lbl.SetAlign(ALIGN_TOP);
	ga_tab_ctrl.Add(ga_diag_lbl.TopPos(gy, 100).LeftPos(12, 260));
}

void StageAWindow::OnGAStart() {
	if (ga_running) return;
	
	int pop = (int)ga_pop_edit;
	int gen = (int)ga_gen_edit;
	bool use_all = (bool)ga_use_all_frames;
	
	if (pop <= 0 || gen <= 0) return;
	
	ga_running = true;
	ga_cancel = 0;
	ga_start.Disable();
	ga_stop.Enable();
	ga_apply.Disable();
	ga_status_lbl.SetLabel("Status: Initializing...");
	ga_plot.Clear();
	
	// Prepare data for solver (copy to staging member)
	ga_input_matches.Clear();
	int row = captures_list.GetCursor();
	
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		if (!use_all && i != row) continue; // Skip if single-frame mode
		const CapturedFrame& f = model->captured_frames[i];
		Size sz = !f.left_img.IsEmpty() ? f.left_img.GetSize() : f.right_img.GetSize();
		if (sz.cx <= 0) continue;
		
		for (const MatchPair& m : f.matches) {
			StereoCalibrationMatch& sm = ga_input_matches.Add();
			sm.left_px = vec2(m.left.x * sz.cx, m.left.y * sz.cy);
			sm.right_px = vec2(m.right.x * sz.cx, m.right.y * sz.cy);
			sm.image_size = sz;
			sm.dist_l = m.dist_l / 1000.0; // mm -> m
			sm.dist_r = m.dist_r / 1000.0;
		}
	}
	
	if (ga_input_matches.GetCount() < 5) {
		ga_status_lbl.SetLabel("Status: Too few matches (<5)");
		ga_running = false;
		ga_start.Enable();
		ga_stop.Disable();
		return;
	}
	
	// Copy current state
	ProjectState start_state = model->project_state;
	double eye_dist = start_state.eye_dist / 1000.0; // mm -> m
	
	// Run worker thread
	ga_thread.Run([=] {
		StereoCalibrationSolver solver;
		solver.matches <<= ga_input_matches; // Deep copy from member
		solver.eye_dist = eye_dist;
		solver.ga_population = pop;
		solver.ga_generations = gen;
		solver.dist_weight = start_state.distance_weight;
		solver.huber_px = start_state.huber_px;
		solver.huber_m = start_state.huber_m;
		
		// Initial params
		StereoCalibrationParams params;
		
		solver.ga_step_cb = [&](int g, double cost, const StereoCalibrationParams& best_p) -> bool {
			if (ga_cancel) return false;
			PostCallback([=] { OnGAStep(g, cost, best_p); });
			return true;
		};
		
		solver.GABootstrapIntrinsics(params);
		
		if (ga_cancel) return; // Aborted
		
		// Finished
		PostCallback([=] {
			ga_best_params = params;
			OnGAFinished();
		});
	});
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
	ga_plot.AddValue(best_cost);
	
	// Optional: update best_p to UI? Maybe too frequent.
	// But cost plot needs this.
}

void StageAWindow::OnGAFinished() {
	ga_running = false;
	ga_start.Enable();
	ga_stop.Disable();
	ga_apply.Enable();
	ga_status_lbl.SetLabel("Status: Finished");
	
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
	
	// Copy ga_best_params to model->project_state
	// GA optimizes: a, cx, cy, c, d (k1, k2 derived)
	// We need to map back to ProjectState fields.
	// NOTE: Solver uses polynomial model (a,b,c,d). StageA uses (f, k1, k2).
	// We need to convert back.
	// solver: r_src = a*theta + b*theta^3 ...
	// StageA: r_src = f*theta * (1 + k1*theta^2 + k2*theta^4) = f*theta + f*k1*theta^3 + f*k2*theta^5
	// So: a = f. 
	// b = f*k1 => k1 = b/a. (But GA solver uses c for 3rd order, d for 5th order?)
	// Let's check GABootstrapIntrinsics mapping in Solver.cpp:
	// p.c = p.a * k1; p.d = p.a * k2; (p.b=0)
	// So k1 = c/a, k2 = d/a.
	
	double f = ga_best_params.a;
	if (fabs(f) > 1e-6) {
		model->project_state.lens_f = f;
		model->project_state.lens_cx = ga_best_params.cx;
		model->project_state.lens_cy = ga_best_params.cy;
		model->project_state.lens_k1 = ga_best_params.c / f;
		model->project_state.lens_k2 = ga_best_params.d / f;
		
		// Update FOV for UI consistency
		// f = (w/2) / tan(fov/2) => tan(fov/2) = w/(2f) => fov = 2*atan(w/(2f))
		// We need width. Assume first image width?
		int row = captures_list.GetCursor();
		if (row >= 0 && row < model->captured_frames.GetCount()) {
			const CapturedFrame& cf = model->captured_frames[row];
			Size sz = !cf.left_img.IsEmpty() ? cf.left_img.GetSize() : cf.right_img.GetSize();
			if (sz.cx > 0) {
				double fov_rad = 2.0 * atan((sz.cx * 0.5) / f);
				model->project_state.fov_deg = fov_rad * 180.0 / M_PI;
			}
		}
		
		RefreshFromModel(); // Updates UI and Preview
		SaveProjectState();
		PromptOK("Genetic optimization results applied.");
	}
}

// Configures columns and selection callbacks for capture/match lists.
void StageAWindow::BuildCaptureLists() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);
	captures_list.WhenBar = THISBACK(OnCapturesBar);

	matches_list.AddColumn("Left");
	matches_list.AddColumn("Right");
	matches_list.AddColumn("Dist L (mm)").Edit(dist_l_editor);
	matches_list.AddColumn("Dist R (mm)").Edit(dist_r_editor);
	matches_list.WhenAction = THISBACK(OnMatchEdited);
	matches_list.WhenBar = THISBACK(OnMatchesBar);
	
	lines_list.AddColumn("Eye");
	lines_list.AddColumn("Points");
	lines_list.WhenBar = THISBACK(OnLinesBar);
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

	right_plot.SetEye(1);
	right_plot.SetTitle("Right Eye");
	right_plot.WhenClickPoint = THISBACK(OnPickMatchTool);
	right_plot.WhenHoverPoint = [=](Pointf p) { OnHoverPoint(p, 1); };
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
	ps.preview_extrinsics = (bool)preview_extrinsics;
	ps.preview_intrinsics = (bool)preview_intrinsics;
	ps.compare_ga_result = (bool)compare_ga_toggle;
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
			Size isz = frame.left_img.GetSize();
			if (isz.cx <= 0) return;

			MatchPair& m = frame.matches.Add();
			m.left = Pointf(pending_left.x / isz.cx, pending_left.y / isz.cy);
			m.right = Pointf(p.x / isz.cx, p.y / isz.cy);
			m.left_text = Format("L%d", frame.matches.GetCount());
			m.right_text = Format("R%d", frame.matches.GetCount());
			
			pending_left = Null;
			status.Set("Match pair added.");
			UpdatePreview();
			SaveProjectState();
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
	if (sz.cx > 0) {
		const ProjectState& ps = model->project_state;
		
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
				if (!ps.preview_extrinsics && !ps.preview_intrinsics) {
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

		Vector<Vector<Pointf>> lines_l, lines_r;
		for(const auto& chain : frame.annotation_lines_left) lines_l.Add(ProjectChain(chain, 0));
		for(const auto& chain : frame.annotation_lines_right) lines_r.Add(ProjectChain(chain, 1));
		
		left_plot.SetAnnotationLines(lines_l);
		right_plot.SetAnnotationLines(lines_r);
	}
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

	// GA params (if valid)
	StereoCalibrationHelpers::LensParams lp_ga;
	double ga_yaw_l=0, ga_pitch_l=0, ga_roll_l=0;
	double ga_yaw_r=0, ga_pitch_r=0, ga_roll_r=0;
	bool ga_valid = compare_ga && (fabs(ga_best_params.a) > 1e-6);

	if (ga_valid) {
		lp_ga.f = (float)ga_best_params.a;
		lp_ga.cx = (float)ga_best_params.cx;
		lp_ga.cy = (float)ga_best_params.cy;
		lp_ga.k1 = (float)(ga_best_params.c / ga_best_params.a);
		lp_ga.k2 = (float)(ga_best_params.d / ga_best_params.a);
		ga_yaw_l = ga_best_params.yaw_l;
		ga_pitch_l = ga_best_params.pitch_l;
		ga_roll_l = ga_best_params.roll_l;
		ga_yaw_r = ga_best_params.yaw;
		ga_pitch_r = ga_best_params.pitch;
		ga_roll_r = ga_best_params.roll;
	}

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

		if (apply_intr) {
			return StereoCalibrationHelpers::RectifyAndRotateOnePass(src, lp, ry, rp, rr, sz);
		} else {
			vec2 pp(lp.cx, lp.cy);
			return StereoCalibrationHelpers::ApplyExtrinsicsOnly(src, ry, rp, rr, pp);
		}
	};

	Image L_curr = Render(frame.left_img, lp_curr, ps.yaw_l, ps.pitch_l, ps.roll_l, false);
	Image R_curr = Render(frame.right_img, lp_curr, ps.yaw_r, ps.pitch_r, ps.roll_r, false);

	if (ga_valid) {
		Image L_ga = Render(frame.left_img, lp_ga, ga_yaw_l, ga_pitch_l, ga_roll_l, true);
		Image R_ga = Render(frame.right_img, lp_ga, ga_yaw_r, ga_pitch_r, ga_roll_r, true);
		
		auto MakeGray = [](const Image& src) {
			if (src.IsEmpty()) return Image();
			Size s = src.GetSize();
			ImageBuffer b(s);
			const RGBA* s_ptr = ~src;
			RGBA* d_ptr = b;
			for(int i=0; i<s.cx*s.cy; i++) {
				int gray = (s_ptr[i].r + s_ptr[i].g + s_ptr[i].b)/3;
				d_ptr[i].r = d_ptr[i].g = d_ptr[i].b = (byte)gray;
				d_ptr[i].a = 255;
			}
			return (Image)b;
		};
		
		L_curr = MakeGray(L_curr);
		R_curr = MakeGray(R_curr);
		
		last_left_preview = AlphaBlend(L_curr, L_ga, 0.5f);
		last_right_preview = AlphaBlend(R_curr, R_ga, 0.5f);
	} else {
		last_left_preview = L_curr;
		last_right_preview = R_curr;
	}

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


END_UPP_NAMESPACE
