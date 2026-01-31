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

namespace {
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
}

StageAWindow::StageAWindow() {
	Title("Stereo Calibration Tool - Stage A");
	Sizeable().Zoomable();
	AddFrame(status);
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
	preview_extrinsics <<= ps.preview_extrinsics;
	view_mode_list.SetIndex(ps.view_mode);
	overlay_eyes <<= ps.overlay_eyes;
	overlay_swap <<= ps.overlay_swap;
	show_difference <<= ps.show_difference;
	show_epipolar <<= ps.show_epipolar;
	alpha_slider <<= ps.alpha;

	captures_list.Clear();
	for (int i = 0; i < model->captured_frames.GetCount(); i++) {
		auto& f = model->captured_frames[i];
		captures_list.Add(Format("%02d:%02d:%02d", f.time.hour, f.time.minute, f.time.second), f.source, f.matches.GetCount());
	}
	if (model->selected_capture >= 0 && model->selected_capture < captures_list.GetCount())
		captures_list.SetCursor(model->selected_capture);
	UpdatePreview();
}

// Composes the main layout (left controls + right preview/list split).
void StageAWindow::BuildLayout() {
	Add(controls.VSizePos(0, 0).LeftPos(0, 300));

	main_split.Horz(preview_split, right_split);
	main_split.SetPos(6500);
	Add(main_split.VSizePos(0, 0).HSizePos(300, 0));

	preview_split.Horz(left_plot, right_plot);
	preview_split.SetPos(5000);
	list_split.Horz(captures_list, matches_list);
	list_split.SetPos(3500);
	right_split.Vert(preview_split, list_split);
	right_split.SetPos(5000);
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

	yaw_l.SetInc(0.01); yaw_l.MinMax(-1.2, 1.2); yaw_l.WhenAction = THISBACK(SyncStageA);
	pitch_l.SetInc(0.01); pitch_l.MinMax(-0.6, 0.6); pitch_l.WhenAction = THISBACK(SyncStageA);
	roll_l.SetInc(0.01); roll_l.MinMax(-1.2, 1.2); roll_l.WhenAction = THISBACK(SyncStageA);
	yaw_r.SetInc(0.01); yaw_r.MinMax(-1.2, 1.2); yaw_r.WhenAction = THISBACK(SyncStageA);
	pitch_r.SetInc(0.01); pitch_r.MinMax(-0.6, 0.6); pitch_r.WhenAction = THISBACK(SyncStageA);
	roll_r.SetInc(0.01); roll_r.MinMax(-1.2, 1.2); roll_r.WhenAction = THISBACK(SyncStageA);

	preview_extrinsics.SetLabel("Preview extrinsics");
	preview_extrinsics <<= true;
	preview_extrinsics.WhenAction = THISBACK(SyncStageA);

	barrel_lbl.SetLabel("Barrel distortion");
	barrel_strength.SetInc(0.1); barrel_strength <<= 0; barrel_strength.WhenAction = THISBACK(SyncStageA);
	fov_lbl.SetLabel("FOV (deg)");
	fov_deg.SetInc(1.0); fov_deg <<= 90; fov_deg.WhenAction = THISBACK(SyncStageA);

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

	alpha_lbl.SetLabel("Alpha");
	alpha_slider.MinMax(0, 100);
	alpha_slider <<= 50;
	alpha_slider.WhenAction = THISBACK(OnReviewChanged);

	yaw_center_btn.SetLabel("Yaw center");
	yaw_center_btn <<= THISBACK(OnYawCenter);
	pitch_center_btn.SetLabel("Pitch center");
	pitch_center_btn <<= THISBACK(OnPitchCenter);

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

	controls.Add(preview_extrinsics.TopPos(y, 20).LeftPos(8, 180));
	y += 24;
	controls.Add(barrel_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(barrel_strength.TopPos(y, 20).LeftPos(132, 80));
	y += 24;
	controls.Add(fov_lbl.TopPos(y, 20).LeftPos(8, 120));
	controls.Add(fov_deg.TopPos(y, 20).LeftPos(132, 80));
	y += 24;
	controls.Add(basic_params_doc.TopPos(y, 100).HSizePos(8, 8));
	y += 104;
	controls.Add(yaw_center_btn.TopPos(y, 24).LeftPos(8, 90));
	controls.Add(pitch_center_btn.TopPos(y, 24).LeftPos(104, 90));
	y += 30;

	controls.Add(view_mode_lbl.TopPos(y, 20).LeftPos(8, 80));
	controls.Add(view_mode_list.TopPos(y, 20).LeftPos(92, 160));
	y += 24;
	controls.Add(overlay_eyes.TopPos(y, 20).LeftPos(8, 100));
	controls.Add(overlay_swap.TopPos(y, 20).LeftPos(112, 90));
	controls.Add(show_difference.TopPos(y, 20).LeftPos(206, 80));
	y += 24;
	controls.Add(alpha_lbl.TopPos(y, 20).LeftPos(8, 40));
	controls.Add(alpha_slider.TopPos(y, 20).LeftPos(52, 200));
	y += 24;
	controls.Add(show_epipolar.TopPos(y, 20).LeftPos(8, 180));
}

// Configures capture and match lists (columns + callbacks).
void StageAWindow::BuildCaptureLists() {
	captures_list.AddColumn("Time");
	captures_list.AddColumn("Source");
	captures_list.AddColumn("Samples");
	captures_list.WhenCursor = THISBACK(OnCaptureSelection);

	matches_list.AddColumn("Left");
	matches_list.AddColumn("Right");
	matches_list.AddColumn("Dist L (mm)").Edit(dist_l_editor);
	matches_list.AddColumn("Dist R (mm)").Edit(dist_r_editor);
	matches_list.WhenAcceptRow = [=] { OnMatchEdited(); return true; };
}

// Initializes plotters (left/right images).
void StageAWindow::BuildPlotters() {
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
	ps.preview_extrinsics = (bool)preview_extrinsics;

	String doc;
	doc << "Stage A Basic Params:\n";
	doc << "  Eye dist: " << ps.eye_dist << " mm\n";
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
	ps.alpha = (int)~alpha_slider;

	for (auto& frame : model->captured_frames)
		frame.undist_valid = false;

	UpdatePreview();
	UpdateReviewEnablement();
	SaveProjectState();
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
	for (const MatchPair& pair : frame.matches)
		matches_list.Add(pair.left_text, pair.right_text, pair.dist_l, pair.dist_r);

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
		double fov_rad = (double)model->project_state.fov_deg * M_PI / 180.0;
		p.a = (sz.cx * 0.5) / (fov_rad * 0.5);
		double s = (double)model->project_state.barrel_strength * 0.01;
		p.b = p.a * s;
		p.c = p.a * s;
		p.d = p.a * s;
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

// Builds undistort cache for a captured frame, if view mode requires it.
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

// Applies raw or undistorted images to the plotters.
void StageAWindow::ApplyPreviewImages(CapturedFrame& frame, const LensPoly& lens, float linear_scale) {
	if (BuildUndistortCache(frame, lens, linear_scale))
	{
		left_plot.SetImage(frame.undist_left);
		right_plot.SetImage(frame.undist_right);
	}
	else
	{
		left_plot.SetImage(frame.left_img);
		right_plot.SetImage(frame.right_img);
	}
}

// Auto-centers yaw using the latest match pair in the selected capture.
void StageAWindow::OnYawCenter() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) {
		status.Set("Select a captured frame first.");
		return;
	}
	CapturedFrame& frame = model->captured_frames[row];
	if (frame.matches.IsEmpty()) {
		status.Set("Add a match pair to center.");
		return;
	}
	const MatchPair& m = frame.matches.Top();
	if (IsNull(m.left) || IsNull(m.right))
		return;
	Size sz = frame.left_img.GetSize();
	if (sz.cx <= 0)
		return;

	double fov_rad = (double)model->project_state.fov_deg * M_PI / 180.0;
	double a = (sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)model->project_state.barrel_strength * 0.01;
	LensPoly lens;
	lens.SetAnglePixel((float)a, (float)(a*s), (float)(a*s), (float)(a*s));
	lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);

	auto GetHAngle = [&](Pointf p) -> double {
		float cx = sz.cx * 0.5f;
		float cy = sz.cy * 0.5f;
		float dx = p.x * sz.cx - cx;
		float dy = p.y * sz.cy - cy;
		float r = sqrt(dx*dx + dy*dy);
		float theta = lens.PixelToAngle(r);
		float roll = atan2(dy, dx);
		double x = sin(theta) * cos(roll);
		double z = cos(theta);
		return atan2(x, z);
	};

	double angle_l = GetHAngle(m.left);
	double angle_r = GetHAngle(m.right);
	double new_yaw_r = model->project_state.yaw_l + angle_l - angle_r;
	double delta = new_yaw_r - model->project_state.yaw_r;
	model->project_state.yaw_r = new_yaw_r;
	yaw_r <<= new_yaw_r;
	SyncStageA();
	status.Set(Format("Yaw aligned. Delta: %.3f deg", delta * 180.0 / M_PI));
}

// Auto-centers pitch using the latest match pair in the selected capture.
void StageAWindow::OnPitchCenter() {
	int row = captures_list.GetCursor();
	if (row < 0 || row >= model->captured_frames.GetCount()) {
		status.Set("Select a captured frame first.");
		return;
	}
	CapturedFrame& frame = model->captured_frames[row];
	if (frame.matches.IsEmpty()) {
		status.Set("Add a match pair to center.");
		return;
	}
	const MatchPair& m = frame.matches.Top();
	if (IsNull(m.left) || IsNull(m.right))
		return;
	Size sz = frame.left_img.GetSize();
	if (sz.cx <= 0)
		return;

	double fov_rad = (double)model->project_state.fov_deg * M_PI / 180.0;
	double a = (sz.cx * 0.5) / (fov_rad * 0.5);
	double s = (double)model->project_state.barrel_strength * 0.01;
	LensPoly lens;
	lens.SetAnglePixel((float)a, (float)(a*s), (float)(a*s), (float)(a*s));
	lens.SetPrincipalPoint(sz.cx * 0.5f, sz.cy * 0.5f);

	auto GetVAngle = [&](Pointf p) -> double {
		float cx = sz.cx * 0.5f;
		float cy = sz.cy * 0.5f;
		float dx = p.x * sz.cx - cx;
		float dy = p.y * sz.cy - cy;
		float r = sqrt(dx*dx + dy*dy);
		float theta = lens.PixelToAngle(r);
		float roll = atan2(dy, dx);
		double y_val = -sin(theta) * sin(roll);
		double z_val = cos(theta);
		return atan2(y_val, z_val);
	};

	double angle_l = GetVAngle(m.left);
	double angle_r = GetVAngle(m.right);
	double new_pitch_r = model->project_state.pitch_l + angle_l - angle_r;
	double delta = new_pitch_r - model->project_state.pitch_r;
	model->project_state.pitch_r = new_pitch_r;
	pitch_r <<= new_pitch_r;
	SyncStageA();
	status.Set(Format("Pitch aligned. Delta: %.3f deg", delta * 180.0 / M_PI));
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

END_UPP_NAMESPACE
