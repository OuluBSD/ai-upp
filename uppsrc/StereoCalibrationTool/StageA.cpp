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
	preview_intrinsics <<= ps.preview_intrinsics;
	view_mode_list.SetIndex(ps.view_mode);
	overlay_eyes <<= ps.overlay_eyes;
	overlay_swap <<= ps.overlay_swap;
	show_difference <<= ps.show_difference;
	show_epipolar <<= ps.show_epipolar;
	tint_overlay <<= ps.tint_overlay;
	show_crosshair <<= ps.show_crosshair;
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

	preview_intrinsics.SetLabel("Preview intrinsics");
	preview_intrinsics <<= false;
	preview_intrinsics.WhenAction = THISBACK(SyncStageA);

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

	controls.Add(preview_extrinsics.TopPos(y, 20).LeftPos(8, 140));
	controls.Add(preview_intrinsics.TopPos(y, 20).LeftPos(152, 140));
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
	controls.Add(tint_overlay.TopPos(y, 20).LeftPos(8, 200));
	y += 24;
	controls.Add(show_crosshair.TopPos(y, 20).LeftPos(8, 180));
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
	ps.preview_intrinsics = (bool)preview_intrinsics;

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
	ps.tint_overlay = (bool)tint_overlay;
	ps.show_crosshair = (bool)show_crosshair;
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
	bool apply_extrinsics = ps.preview_extrinsics;
	bool apply_intrinsics = ps.preview_intrinsics;

	// Determine per-eye parameters
	double yaw_l = ps.yaw_l;
	double pitch_l = ps.pitch_l;
	double roll_l = ps.roll_l;
	double yaw_r = ps.yaw_r;
	double pitch_r = ps.pitch_r;
	double roll_r = ps.roll_r;

	Size sz = !frame.left_img.IsEmpty() ? frame.left_img.GetSize() : frame.right_img.GetSize();
	vec2 pp(sz.cx * 0.5f, sz.cy * 0.5f);

	// Process left eye
	Image left_out = frame.left_img;
	if (!left_out.IsEmpty()) {
		// Fast-path: identity (no preview effects)
		bool left_is_identity = !apply_extrinsics && !apply_intrinsics;
		if (!left_is_identity && apply_extrinsics && fabs(yaw_l) < 1e-6 && fabs(pitch_l) < 1e-6 && fabs(roll_l) < 1e-6 && !apply_intrinsics)
			left_is_identity = true;

		if (!left_is_identity) {
			if (apply_extrinsics && apply_intrinsics) {
				// Both: undistort first (barrel→linear), then rotate
				// Sequential application loses some quality but is simple and correct
				left_out = StereoCalibrationHelpers::ApplyIntrinsicsOnly(left_out, lens, linear_scale, true);
				left_out = StereoCalibrationHelpers::ApplyExtrinsicsOnly(left_out, (float)yaw_l, (float)pitch_l, (float)roll_l, pp);
			} else if (apply_extrinsics) {
				// Extrinsics only
				left_out = StereoCalibrationHelpers::ApplyExtrinsicsOnly(left_out, (float)yaw_l, (float)pitch_l, (float)roll_l, pp);
			} else if (apply_intrinsics) {
				// Intrinsics only: undistort (remove barrel distortion from raw camera image)
				left_out = StereoCalibrationHelpers::ApplyIntrinsicsOnly(left_out, lens, linear_scale, true);
			}
		}
	}

	// Process right eye
	Image right_out = frame.right_img;
	if (!right_out.IsEmpty()) {
		// Fast-path: identity (no preview effects)
		bool right_is_identity = !apply_extrinsics && !apply_intrinsics;
		if (!right_is_identity && apply_extrinsics && fabs(yaw_r) < 1e-6 && fabs(pitch_r) < 1e-6 && fabs(roll_r) < 1e-6 && !apply_intrinsics)
			right_is_identity = true;

		if (!right_is_identity) {
			if (apply_extrinsics && apply_intrinsics) {
				// Both: undistort first (barrel→linear), then rotate
				// Sequential application loses some quality but is simple and correct
				right_out = StereoCalibrationHelpers::ApplyIntrinsicsOnly(right_out, lens, linear_scale, true);
				right_out = StereoCalibrationHelpers::ApplyExtrinsicsOnly(right_out, (float)yaw_r, (float)pitch_r, (float)roll_r, pp);
			} else if (apply_extrinsics) {
				// Extrinsics only
				right_out = StereoCalibrationHelpers::ApplyExtrinsicsOnly(right_out, (float)yaw_r, (float)pitch_r, (float)roll_r, pp);
			} else if (apply_intrinsics) {
				// Intrinsics only: undistort (remove barrel distortion from raw camera image)
				right_out = StereoCalibrationHelpers::ApplyIntrinsicsOnly(right_out, lens, linear_scale, true);
			}
		}
	}

	// Store previews for overlay/tint/crosshair composition (avoid big refactor)
	last_left_preview = left_out;
	last_right_preview = right_out;

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
	bool swap_order = ps.overlay_swap;
	float alpha = ps.alpha / 100.0f; // Convert 0..100 to 0..1

	Image left_display = last_left_preview;
	Image right_display = last_right_preview;

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
