#include "StereoCalibrationTool.h"

NAMESPACE_UPP

/*
LiveResult.cpp
==============
Purpose:
- Live camera view with calibration applied.
 - Does NOT add to the capture list or write project.json.

Data flow:
- Reads live frames from a StereoSource.
- Uses AppModel.last_calibration + project_state to render the view.
*/

LiveResultWindow::LiveResultWindow() {
	Title("Stereo Calibration Tool - Live Result");
	Sizeable().Zoomable();
	AddFrame(status);
	BuildLayout();
}

// Binds AppModel and initializes source list + UI callbacks.
void LiveResultWindow::Init(AppModel& m) {
	model = &m;

	source_list.Add(0, "HMD Stereo Camera");
	source_list.Add(1, "USB Stereo (Side-by-side)");
	source_list.Add(2, "Stereo Video File");
	source_list.SetIndex(0);
	source_list.WhenAction = THISBACK(OnSourceChanged);

	start_source.SetLabel("Start");
	stop_source.SetLabel("Stop");
	live_view.SetLabel("Live view");

	start_source <<= THISBACK(StartSource);
	stop_source <<= THISBACK(StopSource);
	live_view <<= THISBACK(ToggleLiveView);

	source_status.SetLabel("Status: idle");

	sources.Clear();
	sources.Add(MakeOne<HmdStereoSource>());
	sources.Add(MakeOne<UsbStereoSource>());
	sources.Add(MakeOne<VideoStereoSource>());
}

// Updates verbose flag for all live sources.
void LiveResultWindow::SetVerbose(bool v) {
	verbose = v;
	for (auto& src : sources)
		src->SetVerbose(v);
}

// Builds the live result layout (source controls + preview).
void LiveResultWindow::BuildLayout() {
	int y = 6;
	Add(source_list.TopPos(y, 24).HSizePos(6, 6));
	y += 28;
	Add(start_source.TopPos(y, 24).LeftPos(6, 70));
	Add(stop_source.TopPos(y, 24).LeftPos(82, 70));
	Add(live_view.TopPos(y, 24).LeftPos(158, 80));
	y += 28;
	Add(source_status.TopPos(y, 20).HSizePos(6, 6));
	y += 24;
	preview_split.Horz(left_view, right_view);
	preview_split.SetPos(5000);
	Add(preview_split.VSizePos(y, 0).HSizePos(6, 6));
}

// Updates status label on source selection changes.
void LiveResultWindow::OnSourceChanged() {
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		source_status.SetLabel("Selected: " + sources[idx]->GetName());
}

// Starts a source by index (used by controller tests).
bool LiveResultWindow::StartSourceByIndex(int idx) {
	if (idx < 0 || idx >= sources.GetCount())
		return false;
	Mutex::Lock __(source_mutex);
	sources[idx]->SetVerbose(verbose);
	if (!sources[idx]->Start())
		return false;
	source_list.SetIndex(idx);
	source_status.SetLabel("Running: " + sources[idx]->GetName());
	return true;
}

// Starts the currently selected source.
void LiveResultWindow::StartSource() {
	int idx = source_list.GetIndex();
	if (!StartSourceByIndex(idx))
		status.Set("Failed to start source.");
}

// Stops the active source and live timer.
void LiveResultWindow::StopSource() {
	Mutex::Lock __(source_mutex);
	int idx = source_list.GetIndex();
	if (idx >= 0 && idx < sources.GetCount())
		sources[idx]->Stop();
	live_active = false;
	live_cb.Kill();
	source_status.SetLabel("Status: stopped");
}

// Toggles the live preview loop.
void LiveResultWindow::ToggleLiveView() {
	live_active = !live_active;
	if (live_active)
		live_cb.Set(-30, THISBACK(UpdateLivePreview));
	else
		live_cb.Kill();
}

// Non-destructive peek used by automated tests.
bool LiveResultWindow::PeekFrame(Image& left, Image& right, bool prefer_bright) {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return false;
	VisualFrame lf, rf;
	bool ok = false;
	{
		Mutex::Lock __(source_mutex);
		ok = sources[idx]->PeakFrame(lf, rf, prefer_bright);
	}
	if (!ok)
		return false;
	left = StereoCalibrationHelpers::CopyFrameImage(lf);
	right = StereoCalibrationHelpers::CopyFrameImage(rf);
	return !left.IsEmpty() && !right.IsEmpty();
}

// Reads a live frame, applies calibration, and updates the preview control.
void LiveResultWindow::UpdateLivePreview() {
	int idx = source_list.GetIndex();
	if (idx < 0 || idx >= sources.GetCount())
		return;
	VisualFrame lf, rf;
	bool ok = false;
	{
		Mutex::Lock __(source_mutex);
		ok = sources[idx]->ReadFrame(lf, rf, false);
	}
	if (!ok)
		return;
	Image left = StereoCalibrationHelpers::CopyFrameImage(lf);
	Image right = StereoCalibrationHelpers::CopyFrameImage(rf);
	if (left.IsEmpty() || right.IsEmpty())
		return;

	if (BuildLiveUndistortCache(left, right, lf.serial))
	{
		left_view.SetImage(model->live_undist_left);
		right_view.SetImage(model->live_undist_right);
	}
	else
	{
		left_view.SetImage(left);
		right_view.SetImage(right);
	}
}

// Builds undistort cache for live frames, if view mode requires it.
bool LiveResultWindow::BuildLiveUndistortCache(const Image& left, const Image& right, int64 serial) {
	if (!model)
		return false;
	Size sz = !left.IsEmpty() ? left.GetSize() : right.GetSize();
	if (sz.cx <= 0 || sz.cy <= 0)
		return false;
	if (model->project_state.view_mode == 0) {
		model->live_undist_valid = false;
		return false;
	}
	if (model->project_state.view_mode == VIEW_SOLVED && !model->last_calibration.is_enabled)
		return false;

	StereoCalibrationParams p;
	bool use_basic = (model->project_state.view_mode == 1);
	if (use_basic) {
		double fov_rad = model->project_state.fov_deg * M_PI / 180.0;
		p.a = (sz.cx * 0.5) / (fov_rad * 0.5);
		double s = model->project_state.barrel_strength * 0.01;
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

	LensPoly lens;
	lens.SetAnglePixel((float)p.a, (float)p.b, (float)p.c, (float)p.d);
	lens.SetPrincipalPoint((float)p.cx, (float)p.cy);
	float yaw = (float)model->last_calibration.outward_angle;
	float pitch = (float)model->last_calibration.right_pitch;
	float roll = (float)model->last_calibration.right_roll;
	if (model->project_state.stage_c_enabled) {
		yaw += model->dyaw_c;
		pitch += model->dpitch_c;
		roll += model->droll_c;
	}
	lens.SetEyeOutwardAngle(yaw);
	lens.SetRightTilt(pitch, roll);
	lens.SetSize(sz);

	float max_radius = (float)sqrt(sz.cx * sz.cx * 0.25f + sz.cy * sz.cy * 0.25f);
	float max_angle = lens.PixelToAngle(max_radius);
	if (max_angle <= 1e-6f)
		return false;
	float linear_scale = max_radius / max_angle;

	model->live_undist_left = StereoCalibrationHelpers::UndistortImage(left, lens, linear_scale);
	model->live_undist_right = StereoCalibrationHelpers::UndistortImage(right, lens, linear_scale);
	model->live_undist_size = sz;
	model->live_undist_valid = true;
	model->live_undist_serial = serial;
	return true;
}

END_UPP_NAMESPACE
